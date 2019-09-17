plot(1)

# NAME

plot - line graphs on the command line

# SYNOPSIS

_plot_ [OPTIONS]

# DESCRIPTION

plot reads sequences of numbers and outputs a line graph.

# INPUT

Multiple inputs can be specified with *-i*.  Valid numbers are read from the
input and invalid characters are skipped.  For example, the sequence "1, 2
3aaaaa4" would be interpreted as 1, 2, 3, 4.  This allows plot to read any list
of numbers as long as they are delimited by something.

# OPTIONS

*-i* <filename>|-
	add filename (or stdin) as a data source
*-d* [width]:[height]
	set plot dimensions
*-x* [every]:[offset]:[mod]:[color]
	set x label format
*-c* <color>
	color of next data source
*-f*
	"follow" input, only works with stdin
*-h*

# AUTHORS

Maintained by Stone Tickle <lattis@mochiro.moe>
