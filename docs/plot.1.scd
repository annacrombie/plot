plot(1)

# NAME

plot - line graphs on the command line

# SYNOPSIS

_plot_ [OPTIONS]

# DESCRIPTION

plot reads sequences of numbers and outputs a line graph.

# OPTIONS

*-i* <filename> | -
	add filename (or stdin) as a data source.  see *INPUT*

*-d* [width]:[height]
	set plot dimensions (not including axis labels)

*-x* [every]:[offset]:[mod]:[side]:[color]
	set x label format.

	*every* - display "every" nth number.
		e.g., if *every* = 2, display every 2 numbers:
			0 2 4 6 8...

	*offset* - start counting at this number.
		e.g., if *every* = 2 and *offset* = -2:
			\-2 0 2 4 6 8...

	*mod* - perform modulo *mod* on the number before printing.
		e.g., if *every* = 2 and *offset* = -2 and *mod* = 4:
			2 0 2 0 2 0...
	*side* - specify label position
		- *1* put x label on the bottom
		- *2* put x label on the top
		- *3* put x label on bottom and top

	*color* - color of zeroes.  see *COLORS*

	ex: To label hourly data, indicating every four hours you would use:
		plot \-x4::24::

*-y* [width]:[prec]:[side]
	set y label format

	*width* - the width of the label
	*prec* - the precision of the label
	*side* - specify label position
		- *1* put y label on the left
		- *2* put y label on the right
		- *3* put y label on left and right

	*width* and *prec* are used to generate a format string that is
	subsequently used to format and print the label using *printf*(3).  The
	format string template looks something like this: "%<width>.<prec>f"

*-c* <color>
	color of next data source.  see *COLORS*

*-f*
	"follow" input. Basically what this does is print out the entire plot
	every time a number is read.  Additionally, when the length of the
	array of read numbers reaches the plot's width, it shifts everything
	over to the left before adding the new number so that the plot will
	scroll as more data is added.

	Currently only works with stdin as a data source.
*-h*
	Show usage

# INPUT

Multiple inputs can be specified with *-i*.  If no inputs are specified, stdin
is read. Invalid (not 0-9, or -) characters in the input are skipped.  Numbers
are parsed using *strtod*(3).  The sequence "1, 2 3aaaaa4" would be interpreted
the same as the sequence "1 2.0 3 4". This allows plot to read a list of numbers delimited in
almost any way.

# COLORS

A color is specified using a character.  The available characters are:

*b*lack, *r*ed, *g*reen, *y*ellow, b*l*ue, *m*agenta, *c*yan, *w*hite

Additionally, the character may be capitalized to get the "bright" version of
the color. E.g. R == bright red.

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
