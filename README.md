# Plot

Generate a simple ascii plot

```
$ { seq 1 5; seq 1 4 | tac; } | plot -d5
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

# Testing

```
$ meson test -C build
```

or, if you want to watch the tests,

```
$ meson test -C build -v --num-processes 1
```

# Usage

See `plot -h`

# Credits

Inspired by [asciichart](https://github.com/kroitor/asciichart)
