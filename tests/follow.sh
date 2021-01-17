#!/bin/sh -eux

plot="$1"

# Follow stream
seq 1 100 | shuf | timeout -sSIGINT 3 "$plot" -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Pre-bounded stream
seq 1 100 | timeout -sSIGINT 3 "$plot" -b 0:100 -c Y -f -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Follow file
seq 1 100 | shuf >tmp1
timeout -sSIGINT 3 "$plot" -c Y -f -i tmp1 -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]

# Follow files
seq 1 100 | shuf >tmp1
seq 1 150 | shuf >tmp2
seq 1 200 | shuf >tmp3
timeout -sSIGINT 3 "$plot" -c C -f -i tmp1 -c R -i tmp2 -c G -i tmp3 -S 200 -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
