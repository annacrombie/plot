#include <ruby.h>
#include <ruby/encoding.h>
#include <plot.h>
#include "extconf.h"

#define MODNAME "AsciiPlot"

VALUE rb_plot(size_t argc, VALUE* argv, VALUE self)
{
  struct plot_format *pf = init_plot_format();
  VALUE arr, opts, oval;
  double *darr;
  size_t ary_len;

  rb_scan_args(argc, argv, "*:", &arr, &opts);

  if (opts != Qnil) {
    oval = rb_hash_lookup(opts, ID2SYM(rb_intern("height")));
    if (oval != Qnil) pf->height = NUM2LONG(oval);

    oval = rb_hash_lookup(opts, ID2SYM(rb_intern("fmt")));
    if (oval != Qnil) pf->label_format = StringValueCStr(oval);
  }

  ary_len = rb_array_len(arr);

  if (ary_len == 0) return Qnil;

  darr = calloc(ary_len, sizeof(double));

  for (argc=0; argc<ary_len; argc++) {
    darr[argc] = NUM2DBL(rb_ary_shift(arr));
  }

  plotf((int)ary_len, darr, pf);
  free(darr);

  return Qnil;
}

void Init_asciiplot()
{
  VALUE mod = rb_define_module(MODNAME);
  rb_define_module_function(mod, "plot", rb_plot, -1);
}
