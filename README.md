# Plot

Generate a simple ascii plot

```
$ plot 1 2 3 4 5 4 3 2 1
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
$ git clone https://github.com/annacrombie/plot && cd plot
$ make
```

You might want to put the executable somewhere in your `$PATH`, e.g.

```
$ cp target/release/plot /usr/local/bin/plot
```

You can also make a ruby extension with `make ruby`.

# Usage

```
plot [OPTS] [point[ point [ ...]]]
```

Points are parsed as doubles using strtod(3)

## OPTS

+ `-h` - show help
+ `-H HEIGHT` - scale points so that the total height of the plot is HEIGHT
  lines
+ `-f FMT` - the format string for the y-axis labels.  The default is
  `"%11.2f %s"`.  One double and one string are passed to the call to printf(1),
  the double is the y value and the string is the box drawing character.
  WARNING, if you pass an invalid format string, the program will segfault.

# Credits

Inspired by https://github.com/kroitor/asciichart
