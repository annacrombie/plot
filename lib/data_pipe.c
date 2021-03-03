#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "internal/data_proc.h"
#include "internal/log.h"
#include "plot/plot.h"

bool
plot_pipeline_append(struct plot_data *pd, enum plot_data_proc_type proc, void *ctx, uint32_t ctx_size)
{
	assert(ctx_size < PLOT_PIPELINE_CTX_SIZE);
	assert(proc < data_proc_type_count);

	if (pd->pipeline_len >= pd->pipeline_max) {
		return false;
	}

	struct plot_pipeline_elem *pe = &pd->pipe[pd->pipeline_len];

	if (dproc_registry[proc].init &&
	    !dproc_registry[proc].init(pe->ctx, ctx, ctx_size)) {
		return false;
	}

	pe->proc = dproc_registry[proc].proc;

	++pd->pipeline_len;
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
flush_dbuf(struct plot_dbuf *dbuf)
{
	flush_buf(dbuf->dat, &dbuf->i, &dbuf->len);
}

static bool
pipeline_exec(double *out, uint32_t *out_len, uint32_t out_cap, uint32_t max_new,
	struct plot_data *pd)
{
	uint32_t i, new, flush;
	struct plot_dbuf *in;

	in = pd->pipeline_len ? &pd->pipe[pd->pipeline_len - 1].buf : &pd->in.out;
	if (max_new == 0 || in->len - in->i < max_new) {
		in = &pd->in.out;

		new = pd->in.read(pd->in.ctx, &in->dat[in->len], PLOT_DBUF_SIZE - in->len);

		if (!new) {
			/* no input */
			return false;
		}
		in->len += new;

		for (i = 0; i < pd->pipeline_len; ++i) {
			pd->pipe[i].proc(&pd->pipe[i].buf, in, pd->pipe[i].ctx);
			in = &pd->pipe[i].buf;
		}
	}

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
		pd->total_len += new;
		in->i += new;
	}

	flush_dbuf(&pd->in.out);
	for (i = 0; i < pd->pipeline_len; ++i) {
		flush_dbuf(&pd->pipe[i].buf);
	}

	return true;
}

static void
pipeline_sync(struct plot *p)
{
	uint32_t i, max = 0, tmp;

	for (i = 0; i < p->datasets; ++i) {
		tmp = p->data[i].total_len - p->data[i].len;
		if (max < tmp) {
			max = tmp;
		}
	}

	for (i = 0; i < p->datasets; ++i) {
		tmp = max - (p->data[i].total_len - p->data[i].len);

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

	for (i = 0; i < p->datasets; ++i) {
		read |= pipeline_exec(&p->data_buf[i * p->width], &p->data[i].len,
			p->width, max_new, &p->data[i]);
	}

	pipeline_sync(p);

	return read;
}

bool
plot_fetch_until_full(struct plot *p)
{
	uint32_t i;
	bool read = true, full = false;

	while (read && !full) {
		read = false;

		for (i = 0; i < p->datasets; ++i) {
			if (p->data[i].len >= p->width) {
				full = true;
				break;
			}

			read |= pipeline_exec(&p->data_buf[i * p->width], &p->data[i].len,
				p->width, p->width - p->data[i].len, &p->data[i]);
		}

		pipeline_sync(p);
	}

	return full;
}
