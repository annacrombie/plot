#!/usr/bin/env ruby
require 'mkmf'

find_header('plot.h', File.join(__dir__, '../src/'))
$objs = %w[asciiplot.o plot.o]

create_header
create_makefile('asciiplot')
