#include "posix.h"

#include <assert.h>

#include "util.h"

unsigned int
utf8_bytes(const char *utf8)
{
	unsigned char c;

	c = utf8[0];

	if ((c & 0x7f) == c) {
		return 1;
	}else if ((c & 0xdf) == c) {
		return 2;
	}else if ((c & 0xef) == c) {
		return 3;
	}else if ((c & 0xf7) == c) {
		return 4;
	}else {
		return 0;
	}
}
