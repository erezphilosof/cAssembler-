#include "error.h"
#include <stdio.h>

static int error_count = 0;

void print_error(const char *msg) {
    if (msg) {
        fprintf(stderr, "Error: %s\n", msg);
        error_count++;
    }
}

int get_error_count(void) {
    return error_count;
}
