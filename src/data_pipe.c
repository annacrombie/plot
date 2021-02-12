#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

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
};

static struct pipeline pipelines[MAX_DATA] = { 0 };
static uint32_t pipelines_len = 0;

bool
pipeline_create(char *path)
{
	if (pipelines_len >= MAX_DATA) {
		return false;
	}

	struct input *in = &pipelines[pipelines_len].in;

	if (strcmp(path, "-") == 0) {
		in->src = stdin;
	} else if (!(in->src = fopen(path, "r"))) {
		fprintf(stderr, "error opening file '%s': %s\n", path,
			strerror(errno));
		return false;
	}

	++pipelines_len;
	return true;
}

bool
pipeline_append(enum data_proc_type proc, void *ctx, uint32_t size)
{
	assert(pipelines_len);
	assert(size < PIPELINE_CTX);
	assert(proc < data_proc_type_count);

	if (pipelines[pipelines_len - 1].len >= PIPELINE_LEN) {
		return false;
	} else if (!dproc_registry[proc].ctx_validate(ctx, size)) {
		return false;
	}

	struct pipeline_elem *pe = &pipelines[pipelines_len - 1]
				   .pipe[pipelines[pipelines_len - 1].len];

	memcpy(pe->ctx, ctx, size);
	pe->proc = dproc_registry[proc].dproc;

	++pipelines[pipelines_len - 1].len;
	return true;
}

static void
flush_buf(double *dat, uint32_t *i, uint32_t *len)
{
	/* L("flushing %d/%d nums", dbuf->i, dbuf->len); */
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
pipeline_exec(double *out, uint32_t *out_len, uint32_t out_cap, struct pipeline *pl)
{
	uint32_t i, r;

	struct dbuf *in = &pl->in.out;

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

	/* L("out <<< %d:%d", in->i, in->len); */
	/* for (uint32_t j = in->i; j < in->len; ++j) { */
	/* 	fprintf(stderr, "%f, ", in->dat[j]); */
	/* } */
	/* fprintf(stderr, "\n"); */

	r = in->len - in->i;
	if (*out_len + r >= out_cap) {
		uint32_t flush = (*out_len + r) - out_cap;
		flush_buf(out, &flush, out_len);
		/* r = out_cap - *out_len; */
	}

	if (r) {
		memcpy(&out[*out_len], in->dat, r * sizeof(double));
		*out_len += r;
		in->i += r;
	}

	flush_dbuf(&pl->in.out);
	for (i = 0; i < pl->len; ++i) {
		flush_dbuf(&pl->pipe[i].buf);
	}

	return true;
}

void
pipeline_exec_all(struct plot *p)
{
	uint32_t i;
	bool read;
	do {
		read = false;
		for (i = 0; i < pipelines_len; ++i) {
			read |= pipeline_exec(p->data[i].data, &p->data[i].len,
				p->width, &pipelines[i]);
		}
	} while (read);
}
