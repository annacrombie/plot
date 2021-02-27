#ifndef PLOT_H
#define PLOT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define PLOT_DBUF_SIZE 128
#define PLOT_PIPELINE_CTX_SIZE 32

enum plot_charset {
	plot_charset_unicode,
	plot_charset_ascii,
	plot_charset_custom
};

enum plot_color {
	plot_color_black,
	plot_color_red,
	plot_color_green,
	plot_color_yellow,
	plot_color_blue,
	plot_color_magenta,
	plot_color_cyan,
	plot_color_white,
	plot_color_brblack,
	plot_color_brred,
	plot_color_brgreen,
	plot_color_bryellow,
	plot_color_brblue,
	plot_color_brmagenta,
	plot_color_brcyan,
	plot_color_brwhite,
};

enum plot_label_side {
	plot_label_side_neither = 0,
	plot_label_side_bottom  = 1 << 0,
	plot_label_side_top     = 1 << 1,
	plot_label_side_left    = 1 << 0,
	plot_label_side_right   = 1 << 1,
	plot_label_side_both    = 3,
};

enum plot_flags {
	plot_flag_animate           = 1 << 0,
	plot_flag_follow            = 1 << 1,
	plot_flag_color             = 1 << 2,
	plot_flag_merge_plot_pieces = 1 << 3,
	plot_flag_fixed_bounds      = 1 << 4,
};

enum plot_data_proc_type {
	data_proc_avg,
	data_proc_sma,
	data_proc_cma,
	data_proc_roc,
	data_proc_type_count,
};

struct plot_dbuf;

typedef uint32_t ((*plot_input_func)(void *ctx, double *out, uint32_t out_max));
typedef void ((*plot_data_proc_proc))(struct plot_dbuf *out, struct plot_dbuf *in, void *ctx);
typedef bool ((*plot_data_proc_init))(void *ctx, uint32_t size);

struct plot_dbuf {
	double dat[PLOT_DBUF_SIZE];
	uint32_t i, len;
};

struct plot_pipeline_elem {
	uint8_t ctx[PLOT_PIPELINE_CTX_SIZE];
	struct plot_dbuf buf;
	plot_data_proc_proc proc;
};

struct plot_data {
	struct {
		plot_input_func read;
		void *ctx;
		struct plot_dbuf out;
	} in;
	struct plot_pipeline_elem *pipe;
	uint32_t pipeline_len, pipeline_max;
	uint32_t total_len;
	uint32_t len;
	enum plot_color color;
};

struct plot {
	/* low 4 bits are plot piece
	 * high 4 bits are color */
	uint8_t *canvas;
	double *data_buf;
	struct plot_data *data;
	char charset[16][4];
	struct {
		double max, min;
	} bounds;
	struct {
		int64_t start;
		uint32_t mod, every;
		enum plot_color color;
		enum plot_label_side side;
	} x_label;
	struct {
		uint32_t width, prec;
		enum plot_label_side side;
	} y_label;
	uint32_t height, width, depth;
	uint32_t datasets;
	uint32_t follow_rate;
	uint32_t flags;
};

struct plot_version {
	const char *version, *vcs_tag;
};

extern const struct plot_version plot_version;

void plot_init(struct plot *plot, uint8_t *canvas, double *data_buf,
	struct plot_data *pd, uint32_t height, uint32_t width, uint32_t datasets);
void plot_set_charset(struct plot *plot, enum plot_charset charset);
void plot_set_custom_charset(struct plot *plot, char *str, size_t len);
void plot_dataset_init(struct plot_data *pd, enum plot_color color,
	struct plot_pipeline_elem *ple, uint32_t ple_max,
	plot_input_func input_func, void *input_ctx);
bool plot_add_dataset(struct plot *plot, enum plot_color color,
	struct plot_pipeline_elem *ple, uint32_t ple_max,
	plot_input_func input_func, void *input_ctx);
bool plot_pipeline_append(struct plot_data *pd, enum plot_data_proc_type proc, void *ctx, uint32_t ctx_size);
bool plot_fetch(struct plot *plot, uint32_t max_new);
bool plot_plot(struct plot *plot);
#endif
