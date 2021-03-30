# Plot

plot on the command line

```
$ seq 1 5 | plot -d5
       5.00 ┤   ╭─
       4.00 ┤  ╭╯
       3.00 ┤ ╭╯
       2.00 ┤╭╯
       1.00 ┼╯
```

Monitor linux network usage:

```
card=wlan0
plot -d10 -f -p "avg:5|roc:5" \
    -c l -i "/sys/class/net/$card/statistics/rx_packets:r" \
    -c r -i "/sys/class/net/$card/statistics/tx_packets:r"
```

![output of the above command](https://mochiro.moe/d/plot_netmon.gif)

## Building

compile:

```
$ meson build
$ meson compile -C build
```

install:

```
$ meson install -C build
```

## Testing

```
$ meson test -C build
```

or, if you want to watch the tests,

```
$ meson test -C build -v --num-processes 1
```

## Usage

For the cli, see `plot -h` or `man plot`.

See `include/plot/plot.h` for library documentation.  Real documentation is a TODO.

## Credits

Inspired by [asciichart](https://github.com/kroitor/asciichart)
