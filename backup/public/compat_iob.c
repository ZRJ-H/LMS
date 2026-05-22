#include <stdio.h>

extern FILE *__acrt_iob_func(unsigned);

FILE *__iob_func(void) {
	return __acrt_iob_func(0);
}

/* global data: __imp___iob_func points to __iob_func */
void *__imp___iob_func = (void *)__iob_func;
