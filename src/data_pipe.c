#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "data_pipe.h"
#include "data_proc.h"
#include "log.h"
#include "plot.h"

#define PIPELINE_CTX 32
#define PIPELINE_LEN 4

struct pipeline_elem {
	uint8_t ctx[PIPELINE_CTX];
	struct dbuf buf;
	data_proc_proc proc;
};

struct pipeline {
	struct {
		plot_input_func read;
		void *ctx;
		struct dbuf out;
	} in;
	struct pipeline_elem pipe[PIPELINE_LEN];
	uint8_t len;
	uint32_t total_len;
};

struct pipeline default_pipeline = { 0 };
static struct pipeline pipelines[PLOT_MAX_DATASETS] = { 0 };
static uint32_t pipelines_len = 0;

bool
plot_pipeline_create(plot_input_func input_func, void *input_ctx)
{
	if (pipelines_len >= PLOT_MAX_DATASETS) {
		return false;
	}

	memcpy(&pipelines[pipelines_len], &default_pipeline, sizeof(struct pipeline));

	pipelines[pipelines_len].in.read = input_func;
	pipelines[pipelines_len].in.ctx = input_ctx;

	++pipelines_len;
	return true;
}

bool
plot_pipeline_append(enum plot_data_proc_type proc, void *ctx, uint32_t size)
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
	} else if (dproc_registry[proc].init &&
		   !dproc_registry[proc].init(ctx, size)) {
		return false;
	}

	struct pipeline_elem *pe = &pl->pipe[pl->len];

	memcpy(pe->ctx, ctx, size);
	pe->proc = dproc_registry[proc].proc;

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

		if (!(pl->in.out.len = pl->in.read(pl->in.ctx, pl->in.out.dat, DATA_LEN))) {
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
			flush_buf(&p->data_buf[i * p->width], &tmp, &p->data[i].len);
		}
	}
}

bool
plot_fetch(struct plot *p, uint32_t max_new)
{
	uint32_t i;
	bool read = false;

	for (i = 0; i < pipelines_len; ++i) {
		read |= pipeline_exec(&p->data_buf[i * p->width], &p->data[i].len,
			p->width, max_new, &pipelines[i]);
	}

	pipeline_sync(p);

	return read;
}
