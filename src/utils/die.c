#include "die.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// prints error and exits
// could add error codes or logging
void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);  // print formatted error
    va_end(ap);
    fputc('\n', stderr);
    exit(1);  // exit with failure
}