#include <ctype.h>
#include <string.h>

#include "registers.h"

bool is_register(const char *token) {
    if (!token) return false;
    /* Accept names like r0..r7 (case-insensitive) */
    if (strlen(token) != 2) return false;
    if (token[0] != 'r' && token[0] != 'R') return false;
    if (token[1] < '0' || token[1] > '7') return false;
    return true;
}

int reg_number(const char *token) {
    if (!is_register(token)) return -1;
    return token[1] - '0';
}

