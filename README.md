# Plot

Generate a simple ascii plot

```
$ plot <<<"1 2 3 4 5 4 3 2 1"
       5.00 ┤   ╭╮
       4.73 ┤   ││
       4.47 ┤   ││
       4.20 ┤   ││
       3.93 ┤  ╭╯╰╮
       3.67 ┤  │  │
       3.40 ┤  │  │
       3.13 ┤ ╭╯  ╰╮
       2.87 ┤ │    │
       2.60 ┤ │    │
       2.33 ┤ │    │
       2.07 ┤╭╯    ╰╮
       1.80 ┤│      │
       1.53 ┤│      │
       1.27 ┤│      │
       1.00 ┼╯      ╰─
```

# Building

```
$ meson build
$ ninja -C build
```

# Installing

You might want to put the executable somewhere in your `$PATH`, e.g.

```
$ cp target/release/plot /usr/local/bin/plot
```

# Usage

```
plot 0.2.0
usage: plot [opts]
opts
  -i [filename|-] - specify a data source
  -d [width]:[height] - set plot dimensions
  -x [every]:[offset]:[mod]:[side]:[color] - set x label format
  -y [width]:[prec]:[side] - set y label format
  -c <color> - set color of next data source
  -f - "follow" input, only works with stdin
  -m - visually merge overlapping lines, e.g. ╯ and ╰ form ┴
  -h - duh...

colors: black, red, green, yellow, blue, magenta, cyan, white
use capital character for bright variant

ex.
seq 1 99 | shuf | plot -c g
```

# Credits

Inspired by [asciichart](https://github.com/kroitor/asciichart)
