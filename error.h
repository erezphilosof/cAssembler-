#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

/* Prints an error message and counts it */
void print_error(const char *fmt, ...);

/* Returns the number of errors recorded */
int get_error_count(void);

#endif /* ERROR_H */
