# Plot

Generate a simple ascii plot

```
$ { seq 1 5; seq 1 4 | tac } | plot -d5
       5.00 ┤   ╭╮
       4.00 ┤  ╭╯╰╮
       3.00 ┤ ╭╯  ╰╮
       2.00 ┤╭╯    ╰╮
       1.00 ┼╯      ╰─
$
```

# Building

Dependencies:

+ meson
+ ninja
+ c compiler

```
$ meson build
$ ninja -C build
```

# Installing

```
$ ninja -C build install
```

# Usage

```
plot 0.3.0
usage: plot [opts]
opts
  -i <filename>|- - specify a data source
  -a <n> - average n inputs per data point
  -d [width]:[height] - set plot dimensions
  -x [every]:[offset]:[mod]:[side]:[color] - set x label format
  -y [width]:[prec]:[side] - set y label format
  -c <color> - set color of next data source
  -f - "follow" input
  -S <milliseconds> - follow rate
  -m - visually merge overlapping lines, e.g. ╯ and ╰ form ┴
  -s %<charset>|ascii|unicode - set output charset
  -h - duh...

colors: black, red, green, yellow, blue, magenta, cyan, white
use capital character for bright variant

ex.
seq 1 99 | shuf | plot -c g
```

# Credits

Inspired by [asciichart](https://github.com/kroitor/asciichart)
