#!/bin/sh -eux

plot="$1"

# Follow stream
seq 1 100 | shuf | timeout -sSIGINT 3 "$plot" -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Pre-bounded stream
seq 1 100 | timeout -sSIGINT 3 "$plot" -b 0:100 -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Follow file
seq 1 100 | shuf >./tests/tmp.raw
timeout -sSIGINT 3 "$plot" -c Y -f -i ./tests/tmp.raw -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
rm -f ./tests/tmp.raw

# Follow files
seq 1 100 | shuf >./tests/tmp.1.raw
seq 1 150 | shuf >./tests/tmp.2.raw
seq 1 200 | shuf >./tests/tmp.3.raw
timeout -sSIGINT 3 "$plot" -c C -f -i ./tests/tmp.1.raw -c R -i ./tests/tmp.2.raw -c G -i ./tests/tmp.3.raw -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
rm -f ./tests/tmp.1.raw ./tests/tmp.2.raw ./tests/tmp.3.raw
