#ifndef CLI_MAIN_H
#define CLI_MAIN_H

#include <plot/file_input.h>
#include <plot/plot.h>

#define MAX_HEIGHT 512
#define MAX_WIDTH 512
#define MAX_DATASETS 32
#define MAX_PIPELINE_ELEMENTS 32
#define FILE_INPUT_BUF 16384
#define OUT_BUF (MAX_WIDTH * MAX_HEIGHT)

struct plot_static_memory {
	struct plot plot;
	uint8_t canvas[MAX_WIDTH * MAX_HEIGHT];
	double data_buf[MAX_WIDTH * MAX_HEIGHT];
	struct plot_data pd[MAX_DATASETS];
	char out_buf[MAX_WIDTH * MAX_HEIGHT];
	struct plot_file_input file_input_ctxs[MAX_DATASETS];
	char file_input_bufs[MAX_DATASETS][FILE_INPUT_BUF];
	struct plot_pipeline_elem pipeline_elems[MAX_DATASETS * MAX_PIPELINE_ELEMENTS];
	struct plot_pipeline_elem default_pipeline[MAX_PIPELINE_ELEMENTS];
	struct plot_data default_dataset;
};
#endif
