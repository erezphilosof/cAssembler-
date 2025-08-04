#include "error.h"
#include <stdio.h>
#include <stdarg.h>

static int error_count = 0;

void increment_error_count(void) {
    error_count++;
}

int get_error_count(void) {
    return error_count;
}

void print_error(const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    increment_error_count();
}
