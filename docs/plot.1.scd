plot(1)

# NAME

plot - line graphs on the command line

# SYNOPSIS

_plot_ [OPTIONS]

# DESCRIPTION

plot reads sequences of numbers and outputs a line graph.

# OPTIONS

## INPUT

*-i* <filename>[:flags]
	add <filename> as a data source.  This also creates a data pipeline,
	which is either empty, or copied from the _default pipeline_.  The
	filename `-` indicates stdin.

	The data format accepted by plot is very lenient.  The parser scans the
	input until a digit is found, and attempts to parse the number.  If
	successful, the number is added to the input, otherwise the digit is
	skipped.

	Flags modify how the file is accessed.  Each flag is a single
	character, order and duplicates make no difference and are not checked.

	Available flags are:
	- `r` - *rewind*(3) the file if an EOF is encountered and start reading
	  new data from the beginning. this flag only has an effect in follow
	  mode
	- `n` - open the file in nonblocking mode.  be careful

	If _plot_ is invoked without any `-i` options, an implicit `-i -` is
	added.  i.e. stdin is added as an input and a data pipeline is set up
	for it.

*-p* <pipeline>
	The syntax for a pipeline element is *<data processor>*[:argument].
	Wether or not an argument is required depends on the data processor.
	Multiple processors can be used, separated by the pipe character `|`.
	In this case, the output of each successive processor is passed as
	input to the next.

	*DEFAULT PIPELINE*

	If a pipeline is specified before any input (before
	any `-i`s in the command line), it is recorded as the _default
	pipeline_.  Any pipeline created afterwards (with the `-p` flag or
	implicitly with the `-i` flag) will have this _default pipeline_
	prepended to it.

	See *DATA PROCESSORS* for a list available processors

## DISPLAY / FORMATTING

*-b* [min]:[max]
	set fixed plot bounds (instead of dynamically evaluating bounds).

*-d* [height]:[width]
	set plot dimensions (not including axis labels)

*-x* [every]:[offset]:[mod]:[side]:[color]
	set x label format.  You may omit fields, in which case the default is
	used.  e.g. 4::3 will only set every=4 and mod=3, leaving the rest of
	the parameters unchanged.

	*every* - display "every" nth number.
		e.g., if *every* = 2, display every 2 numbers:
			0 2 4 6 8...

	*offset* - start counting at this number.
		e.g., if *every* = 2 and *offset* = -2:
			\-2 0 2 4 6 8...

	*mod* - perform modulo *mod* on the number before printing.
		e.g., if *every* = 2 and *offset* = -2 and *mod* = 4:
			2 0 2 0 2 0...

	*side* - specify label position, default: bottom
		*0*: hide, *1*: bottom, *2*: top, *3*: top and bottom

	*color* - color of zeroes.  see *COLORS*

	ex: Label hourly data, indicating every four hours
	```
	plot -x4::24::
	```

*-y* [width]:[prec]:[side]
	set y label format

	*width* - the width of the label

	*prec* - the precision of the label

	*side* - specify label position
		*0*: hide, *1*: left, *2*: right, *3*: left and right

	*width* and *prec* are used to generate a format string that is
	subsequently used to format and print the label using *printf*(3).  The
	format string template looks something like this: "%<width>.<prec>f"

*-c* <color>
	color of next data source.  A color is specified using a character.
	The available characters are:

	*b*lack, *r*ed, *g*reen, *y*ellow, b*l*ue, *m*agenta, *c*yan, *w*hite

	Additionally, the character may be capitalized to get the "bright"
	version of the color. E.g. R == bright red.

*-m*
	visually merge overlapping lines
		e.g. ╯ and ╰ form ┴

*-s* %<charset> | ascii | unicode
	set the output charset.  The default is *unicode*.  *ascii* uses only
	ascii characters.  Finally, you can specify a custom charset by passing
	an argument in the form of %<charset> where <charset> is a string
	containing 12 utf8 codepoints.

	For example, to get the default unicode charset:
	```
	-s "% ╭│╰├╮─┬╯┤┴┼"
	```
	(note the space right after the %).

	Here's another example to try:
	```
	-s "% ╔║╚╠╗═╦╝╣╩╬".
	```

	If you pass an insufficient number of codepoints, the missing ones will
	show up as '?'.

## FOLLOW / ANIMATE

*-f*
	"follow" input, e.g. continuously monitor inputs for new data and
	redraw the plot every time a new number is read.  Additionally, when
	the length of the plot reaches the maximum width, begin scrolling to
	the left.

*-A*
	"animate" input by following and exit. The behaviour is identical
	to the "follow" -f feature, but upon input EOF, the plot stops.

*-S* <ms>
	set the framerate for animate/follow modes


## HELP

*-h*
	Show a brief help message

# DATA PROCESSORS

[- name
:- argument
:< description
|  avg
:  integer
:  averages every n inputs together into a single data point
|  sma
:  integer
:  a simple moving average (boxcar filter) for n inputs
|  cma
:  none
:  a cumulative moving average
|  roc
:  float
:  determines rate of change with the basic formula (val - old_val) / t

# EXAMPLE

Monitor network usage on linux:

```
card=wlan0
plot -f -p "avg:5|roc:5" \
    -c l -i "/sys/class/net/$card/statistics/rx_packets:r" \
    -c r -i "/sys/class/net/$card/statistics/tx_packets:r"
```

First, use the `-f` flag to tell plot to continue to read the file.  Next,
setup a default pipeline which first averages every 5 inputs together (to
smooth out noise) and then computes the rate of change.  Next we add two
inputs, with their color set to blue and red respectively.  We also use the
rewind (`:r`) flag for both files, to convert a file containing a single number
into a stream of numbers by re-reading it over and over from the beginning.

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
