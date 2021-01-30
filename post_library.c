/*
 * Copyright (c) 2021 Markus Himmel. All rights reserved.
 * Released under Apache 2.0 license as described in the file LICENSE.
 */

#include <stdlib.h>

#define MAX_TESTCASE_LENGTH 2048

const unsigned char *afl_postprocess(const unsigned char *in_buf,
	unsigned int *len) {
	if (*len > MAX_TESTCASE_LENGTH) return NULL;

	return in_buf;
}
