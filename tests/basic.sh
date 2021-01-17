#!/bin/sh -eux

plot="$1"

# Linear line
seq 1 1000 | "$plot"

# Colored line
seq 1 1000 | "$plot" -c r

# Average line
seq 1 1000 | "$plot" -a 40 -c c

# Shuffled line
seq 1 1000 | shuf | "$plot" -c G

# Example ramps
{
  seq 1 5
  seq 1 4 | tac
} | "$plot" -d5

# Left/bottom labels
seq 1 1000 | shuf | "$plot" -c Y -x 5: -y 10:3:1

# Right/top labels
seq 1 1000 | shuf | "$plot" -c M -x 5:::2 -y 10:3:2

# Every labels
seq 1 1000 | shuf | "$plot" -c C -x 5:0:0:3:blue -y 10:3:3

# Animate files
seq 1 100 | shuf >./tests/tmp.1.raw
seq 1 150 | shuf >./tests/tmp.2.raw
seq 1 200 | shuf >./tests/tmp.3.raw
"$plot" -c C -A -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -c G -i ./tests/tmp.3.raw -S 200 -x 5: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw ./tests/tmp.3.raw

# Merge files
seq 1 150 | shuf >./tests/tmp.1.raw
seq 1 100 | shuf >./tests/tmp.2.raw
"$plot" -a2 -c C -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -m -S 200 -x 10: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw
