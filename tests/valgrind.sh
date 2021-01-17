#!/bin/sh
set -eux

# Parameters
plot="${1?}"

# Configurations
vg_opts='--leak-check=yes --track-origins=yes --exit-on-first-error=yes --error-exitcode=1'

# Memory leaks
seq 1 100 | shuf | valgrind ${vg_opts} "${plot}" -c Y -x 5: -y 10:3:1
seq 1 100 | shuf | timeout -sSIGINT 3 valgrind ${vg_opts} "${plot}" -c Y -f -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
