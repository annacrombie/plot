#!/bin/sh -eux

plot="$1"

valgrind_()
{
	valgrind --leak-check=yes \
		--track-origins=yes \
		--exit-on-first-error=yes \
		--error-exitcode=1 \
		"$@"
}

# Memory leaks
seq 1 100 | shuf | valgrind_ "$plot" -c Y -x 5: -y 10:3:1
seq 1 100 | shuf | timeout -sSIGINT 3 valgrind_ "$plot" -c Y -f -x 5: -y 10:3:1 || [ "${?}" -eq 124 ]
