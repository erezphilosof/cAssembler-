#ifndef ERROR_H
#define ERROR_H

void print_error(const char *fmt, ...);
void increment_error_count(void);
int get_error_count(void);

#endif /* ERROR_H */
