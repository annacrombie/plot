#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "data_pipe.h"
#include "data_proc.h"
#include "input.h"
#include "log.h"
#include "plot.h"

#define PIPELINE_CTX 32
#define PIPELINE_LEN 4

struct pipeline_elem {
	uint8_t ctx[PIPELINE_CTX];
	struct dbuf buf;
	dproc proc;
};

struct pipeline {
	struct input in;
	struct pipeline_elem pipe[PIPELINE_LEN];
	uint8_t len;
	uint32_t total_len;
};

struct pipeline default_pipeline = { 0 };
static struct pipeline pipelines[MAX_DATA] = { 0 };
static uint32_t pipelines_len = 0;

bool
pipeline_create(const char *path)
{
	int fd;

	if (pipelines_len >= MAX_DATA) {
		return false;
	}

	memcpy(&pipelines[pipelines_len], &default_pipeline, sizeof(struct pipeline));

	struct input *in = &pipelines[pipelines_len].in;

	if (strcmp(path, "-") == 0) {
		in->src = stdin;
	} else if (!(in->src = fopen(path, "r"))) {
		fprintf(stderr, "error opening file '%s': %s\n", path,
			strerror(errno));
		return false;
	}

	if ((fd = fileno(in->src) == -1)) {
		fprintf(stderr, "couldn't get file descriptor for '%s': %s\n",
			path, strerror(errno));
		return false;
	}

	int flgs = fcntl(fd, F_GETFL);
	flgs = fcntl(fd, F_SETFL, flgs | O_NONBLOCK);

	++pipelines_len;
	return true;
}

bool
pipeline_append(enum data_proc_type proc, void *ctx, uint32_t size)
{
	assert(size < PIPELINE_CTX);
	assert(proc < data_proc_type_count);

	struct pipeline *pl;

	if (pipelines_len) {
		pl = &pipelines[pipelines_len - 1];
	} else {
		pl = &default_pipeline;
	}

	if (pl->len >= PIPELINE_LEN) {
		return false;
	} else if (dproc_registry[proc].ctx_validate &&
		   !dproc_registry[proc].ctx_validate(ctx, size)) {
		return false;
	}

	struct pipeline_elem *pe = &pl->pipe[pl->len];

	memcpy(pe->ctx, ctx, size);
	pe->proc = dproc_registry[proc].dproc;

	++pl->len;
	return true;
}

static void
flush_buf(double *dat, uint32_t *i, uint32_t *len)
{
	/* L("flushing %d/%d nums", *i, *len); */
	memmove(dat, &dat[*i], (*len - *i) * sizeof(double));
	*len -= *i;
	*i = 0;
}

static void
flush_dbuf(struct dbuf *dbuf)
{
	flush_buf(dbuf->dat, &dbuf->i, &dbuf->len);
}

static bool
pipeline_exec(double *out, uint32_t *out_len, uint32_t out_cap, uint32_t max_new,
	struct pipeline *pl)
{
	uint32_t i, new, flush;
	struct dbuf *in;

	in = pl->len ? &pl->pipe[pl->len - 1].buf : &pl->in.out;
	if (max_new == 0 || in->len - in->i < max_new) {
		in = &pl->in.out;

		if (!input_read(&pl->in)) {
			/* L("no input"); */
			return false;
		}

		for (i = 0; i < pl->len; ++i) {
			/* L("data[%d] <<< %d:%d", i, in->i, in->len); */
			/* for (uint32_t j = in->i; j < in->len; ++j) { */
			/* 	fprintf(stderr, "%f, ", in->dat[j]); */
			/* } */
			/* fprintf(stderr, "\n"); */

			pl->pipe[i].proc(&pl->pipe[i].buf, in, pl->pipe[i].ctx);
			in = &pl->pipe[i].buf;
		}
	}

	/* L("out <<< %d:%d", in->i, in->len); */
	/* for (uint32_t j = in->i; j < in->len; ++j) { */
	/* 	fprintf(stderr, "%f, ", in->dat[j]); */
	/* } */
	/* fprintf(stderr, "\n"); */

	new = in->len - in->i;
	if (max_new && new > max_new) {
		new = max_new;
	}

	if (*out_len + new >= out_cap) {
		if (new >= out_cap) {
			flush = *out_len;
		} else {
			flush = (*out_len + new) - out_cap;
		}

		flush_buf(out, &flush, out_len);
	}

	if (new) {
		memcpy(&out[*out_len], in->dat, new * sizeof(double));
		*out_len += new;
		pl->total_len += new;
		in->i += new;
	}

	flush_dbuf(&pl->in.out);
	for (i = 0; i < pl->len; ++i) {
		flush_dbuf(&pl->pipe[i].buf);
	}

	return true;
}

static void
pipeline_sync(struct plot *p)
{
	uint32_t i, max = 0, tmp;

	for (i = 0; i < pipelines_len; ++i) {
		tmp = pipelines[i].total_len - p->data[i].len;
		if (max < tmp) {
			max = tmp;
		}
	}

	for (i = 0; i < pipelines_len; ++i) {
		tmp = max - (pipelines[i].total_len - p->data[i].len);

		if (tmp && tmp <= p->data[i].len) {
			flush_buf(p->data[i].data, &tmp, &p->data[i].len);
		}
	}
}

bool
pipeline_exec_all(struct plot *p, uint32_t max_new)
{
	uint32_t i;
	bool read = false;

	for (i = 0; i < pipelines_len; ++i) {
		read |= pipeline_exec(p->data[i].data, &p->data[i].len,
			p->width, max_new, &pipelines[i]);
	}

	pipeline_sync(p);

	return read;
}

void
pipeline_fast_fwd(struct plot *p)
{
	while (pipeline_exec_all(p, 0)) {
	}
}

void
pipeline_reset_eofs(void)
{
	uint32_t i;
	for (i = 0; i < pipelines_len; ++i) {
		if (feof(pipelines[i].in.src)) {
			clearerr(pipelines[i].in.src);
		}
	}
}
