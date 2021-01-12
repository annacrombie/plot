#!/usr/bin/env python3

import sys

infile = sys.argv[1]

SEP = "@"

with open(infile, "r") as f:
    for line in f:
        if line[0] == SEP and line[-2] == SEP:
            path = line[1:-2]
            with open(path, "r") as inf:
                print(inf.read(), end="")
        else:
            print(line, end="")
