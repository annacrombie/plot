#!/bin/sh
set -ex

# Linear line
seq 1 1000 | plot

# Colored line
seq 1 1000 | plot -c r

# Average line
seq 1 1000 | plot -a 40 -c c

# Shuffled line
seq 1 1000 | shuf | plot -c G

# Example ramps
{
  seq 1 5
  seq 1 4 | tac
} | plot -d5

# Left/bottom labels
seq 1 1000 | shuf | plot -c Y -x 5: -y 10:3:1

# Right/top labels
seq 1 1000 | shuf | plot -c M -x 5:::2 -y 10:3:2

# Every labels
seq 1 1000 | shuf | plot -c C -x 5:0:0:3:blue -y 10:3:3

# Follow stream
seq 1 100 | shuf | timeout -sSIGINT 3 plot -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Pre-bounded stream
seq 1 100 | timeout -sSIGINT 3 plot -b 0:100 -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Follow file
seq 1 100 | shuf >./tests/tmp.raw
timeout -sSIGINT 3 plot -c Y -f -i ./tests/tmp.raw -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
rm -f ./tests/tmp.raw

# Follow files
seq 1 100 | shuf >./tests/tmp.1.raw
seq 1 150 | shuf >./tests/tmp.2.raw
seq 1 200 | shuf >./tests/tmp.3.raw
timeout -sSIGINT 3 plot -c C -f -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -c G -i ./tests/tmp.3.raw -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw ./tests/tmp.3.raw

# Animate files
seq 1 100 | shuf >./tests/tmp.1.raw
seq 1 150 | shuf >./tests/tmp.2.raw
seq 1 200 | shuf >./tests/tmp.3.raw
plot -c C -A -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -c G -i ./tests/tmp.3.raw -S 200 -x 5: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw ./tests/tmp.3.raw

# Merge files
seq 1 150 | shuf >./tests/tmp.1.raw
seq 1 100 | shuf >./tests/tmp.2.raw
plot -a2 -c C -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -m -S 200 -x 10: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw

# Memory leaks
seq 1 100 | shuf | valgrind --leak-check=yes --track-origins=yes plot -c Y -x 5: -y 10:3:1
seq 1 100 | shuf | timeout -sSIGINT 3 valgrind --leak-check=yes --track-origins=yes plot -c Y -f -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Tests result
echo 'All tests executed successfully'
