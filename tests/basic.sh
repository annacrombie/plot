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
seq 1 100 | shuf >tmp1
seq 1 150 | shuf >tmp2
seq 1 200 | shuf >tmp3
"$plot" -c C -A -i tmp1 -c R -i tmp2 -c G -i tmp3 -S 200 -x 5: -y 10:3:1

# Merge files
seq 1 150 | shuf >tmp1
seq 1 100 | shuf >tmp2
"$plot" -a2 -c C -i tmp1 -c R -i tmp2 -m -S 200 -x 10: -y 10:3:1
