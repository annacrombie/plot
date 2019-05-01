#include <ruby.h>
#include <ruby/encoding.h>
#include <plot.h>
#include "extconf.h"

#define MODNAME "AsciiPlot"

VALUE rb_plot(int argc, VALUE* argv, VALUE self)
{
  VALUE arr;
  VALUE opts;
  double *darr;
  long ary_len;

  rb_scan_args(argc, argv, "*:", &arr, &opts);

  ary_len = rb_array_len(arr);

  if (ary_len == 0) return Qnil;

  darr = calloc(ary_len, sizeof(double));

  for (argc=0; argc<ary_len; argc++) {
    darr[argc] = NUM2DBL(rb_ary_shift(arr));
  }

  plot(16, "%11.2f %s", (int)ary_len, darr);
  free(darr);

  return Qnil;
}

void Init_asciiplot()
{
  VALUE mod = rb_define_module(MODNAME);
  rb_define_module_function(mod, "plot", rb_plot, -1);
}
