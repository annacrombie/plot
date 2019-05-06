#include <ruby.h>
#include <ruby/encoding.h>
#include <plot.h>
#include "extconf.h"

#define MODNAME "AsciiPlot"

VALUE rb_plot(size_t argc, VALUE* argv, VALUE self)
{
  VALUE arr, opts, oval;
  double *darr;
  size_t ary_len;
  long height = 16;
  const char *fmt = "%11.2f %s";

  rb_scan_args(argc, argv, "*:", &arr, &opts);

  if (opts != Qnil) {
    oval = rb_hash_lookup(opts, ID2SYM(rb_intern("height")));
    if (oval != Qnil) height = NUM2LONG(oval);

    oval = rb_hash_lookup(opts, ID2SYM(rb_intern("fmt")));
    if (oval != Qnil) fmt = StringValueCStr(oval);
  }

  ary_len = rb_array_len(arr);

  if (ary_len == 0) return Qnil;

  darr = calloc(ary_len, sizeof(double));

  for (argc=0; argc<ary_len; argc++) {
    darr[argc] = NUM2DBL(rb_ary_shift(arr));
  }

  plot(height, fmt, (int)ary_len, darr);
  free(darr);

  return Qnil;
}

void Init_asciiplot()
{
  VALUE mod = rb_define_module(MODNAME);
  rb_define_module_function(mod, "plot", rb_plot, -1);
}
