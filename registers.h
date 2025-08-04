#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdbool.h>

/* Check if the given token is a register name (R0-R7) */
bool is_register(const char *token);

/* Map register token to its numeric code (0-7). Returns -1 if invalid. */
int reg_number(const char *token);

#endif /* REGISTERS_H */
