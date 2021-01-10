#!/bin/sh
set -e

# Test prepare function
test_prepare() {
  echo ''
}

# Test finish function
test_finish() {
  sleep 1
  echo ''
}

# Linear line
test_prepare
seq 1 1000 | ./build/plot
test_finish

# Colored line
test_prepare
seq 1 1000 | ./build/plot -c r
test_finish

# Average line
test_prepare
seq 1 1000 | ./build/plot -a 40 -c c
test_finish

# Shuffled line
test_prepare
seq 1 1000 | shuf | ./build/plot -c G
test_finish

# Example ramps
test_prepare
{
  seq 1 5
  seq 1 4 | tac
} | ./build/plot -d5
test_finish

# Left/bottom labels
test_prepare
seq 1 1000 | shuf | ./build/plot -c Y -x 5: -y 10:3:1
test_finish

# Right/top labels
test_prepare
seq 1 1000 | shuf | ./build/plot -c M -x 5:::2 -y 10:3:2
test_finish

# Every labels
test_prepare
seq 1 1000 | shuf | ./build/plot -c C -x 5:0:0:3:blue -y 10:3:3
test_finish

# Follow stream
test_prepare
seq 1 100 | shuf | ./build/plot -c Y -f -S 200 -x 5: -y 10:3:1
test_finish

# Pre-bounded stream
test_prepare
seq 1 100 | ./build/plot -b 0:100 -c Y -f -S 200 -x 5: -y 10:3:1
test_finish

# Follow file
test_prepare
seq 1 100 | shuf >./tests/tmp.raw
./build/plot -c Y -f -i ./tests/tmp.raw -S 200 -x 5: -y 10:3:1
rm -f ./tests/tmp.raw
test_finish

# Follow files
test_prepare
seq 1 100 | shuf >./tests/tmp.1.raw
seq 1 150 | shuf >./tests/tmp.2.raw
seq 1 200 | shuf >./tests/tmp.3.raw
./build/plot -c C -f -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -c G -i ./tests/tmp.3.raw -S 200 -x 5: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw ./tests/tmp.3.raw
test_finish

# Merge files
test_prepare
seq 1 150 | shuf >./tests/tmp.1.raw
seq 1 100 | shuf >./tests/tmp.2.raw
./build/plot -a2 -c C -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -m -S 200 -x 10: -y 10:3:1
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw
test_finish

# Memory leaks
seq 1 100 | shuf | valgrind --leak-check=yes --track-origins=yes ./build/plot -c Y -x 5: -y 10:3:1
seq 1 100 | shuf | valgrind --leak-check=yes --track-origins=yes ./build/plot -c Y -f -x 5: -y 10:3:1
