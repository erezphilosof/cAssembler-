// utils.c
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>

// Removes whitespace from the beginning and end of the string
void trim_string(char* str) {
    int len = strlen(str);
    int start = 0, end = len - 1;
    while (isspace((unsigned char)str[start])) start++;
    while (end >= start && isspace((unsigned char)str[end])) end--;
    if (start > 0 || end < len - 1) {
        int i = 0;
        while (start <= end)
            str[i++] = str[start++];
        str[i] = '\0';
    }
}

// Returns true if the string is empty or contains only whitespace
bool is_whitespace(const char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) return false;
        str++;
    }
    return true;
}

// Returns true if the line is a comment (starts with ; or is empty)
bool is_comment(const char* str) {
    while (isspace((unsigned char)*str)) str++;
    return *str == ';' || *str == '\0';
}

// Splits a string into tokens by delimiter, returns how many tokens were found
int split_string(const char* str, char delimiter, char tokens[][80], int max_tokens) {
    int count = 0;
    const char *start = str, *p = str;
    while (*p) {
        if (*p == delimiter) {
            int len = p - start;
            if (len > 0 && count < max_tokens) {
                if (len > 79) len = 79;
                strncpy(tokens[count], start, len);
                tokens[count][len] = '\0';
                trim_string(tokens[count]);
                count++;
            }
            start = p + 1;
        }
        p++;
    }
    // Last token
    if (start < p && count < max_tokens) {
        int len = p - start;
        if (len > 79) len = 79;
        strncpy(tokens[count], start, len);
        tokens[count][len] = '\0';
        trim_string(tokens[count]);
        count++;
    }
    return count;
}

// Converts a string to uppercase (in place)
void to_upper_case(char* str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

// Returns true if the string matches an opcode mnemonic, directive, or register
bool is_reserved_word(const char *s) {
    if (!s) return false;

    static const char *opcodes[] = {
        "MOV", "CMP", "ADD", "SUB", "LEA",
        "CLR", "NOT", "INC", "DEC", "JMP",
        "BNE", "JSR", "RED", "PRN", "RTS", "STOP",
        NULL
    };

    static const char *directives[] = {
        "DATA", "STRING", "MAT", "ENTRY", "EXTERN",
        NULL
    };

    static const char *registers[] = {
        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        NULL
    };

    for (int i = 0; opcodes[i]; ++i)
        if (strcasecmp(s, opcodes[i]) == 0)
            return true;

    for (int i = 0; directives[i]; ++i)
        if (strcasecmp(s, directives[i]) == 0)
            return true;

    for (int i = 0; registers[i]; ++i)
        if (strcasecmp(s, registers[i]) == 0)
            return true;

    return false;
}

// Returns true if the given string is a valid label name
bool is_valid_label(const char* str) {
    if (!str || !isalpha((unsigned char)str[0]))
        return false;
    int i = 1;
    while (str[i]) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_')
            return false;
        i++;
    }
    if (is_reserved_word(str))
        return false;
    return true;
}

// Converts a 16-bit word to a base-4 string representation
void convert_to_base4(uint16_t value, char *out) {
    char tmp[9];
    for (int i = 7; i >= 0; --i) {
        tmp[i] = "0123"[value & 0x3];
        value >>= 2;
    }
    tmp[8] = '\0';
    strcpy(out, tmp);
}

// Prints an error message and exits the program
void error_exit(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Replace all occurrences of substring `old` in `src` with `new_sub`.
// Returns a newly allocated string or NULL on allocation failure.
char *replace_substring(const char *src, const char *old, const char *new_sub) {
    if (!src || !old || !new_sub) return NULL;

    size_t src_len = strlen(src);
    size_t old_len = strlen(old);
    size_t new_len = strlen(new_sub);

    if (old_len == 0) {
        char *dup = malloc(src_len + 1);
        if (!dup) return NULL;
        strcpy(dup, src);
        return dup;
    }

    size_t count = 0;
    const char *p = src;
    while ((p = strstr(p, old)) != NULL) {
        count++;
        p += old_len;
    }

    size_t result_len = src_len + count * (new_len - old_len);
    char *result = malloc(result_len + 1);
    if (!result) return NULL;

    const char *src_p = src;
    char *dst_p = result;
    while ((p = strstr(src_p, old)) != NULL) {
        size_t chunk = (size_t)(p - src_p);
        memcpy(dst_p, src_p, chunk);
        dst_p += chunk;
        memcpy(dst_p, new_sub, new_len);
        dst_p += new_len;
        src_p = p + old_len;
    }
    strcpy(dst_p, src_p);

    return result;
}

// Concatenate `base` and `suffix` into newly allocated string.
// Returns NULL on allocation failure.
char *strcat_printf(const char *base, const char *suffix) {
    if (!base) base = "";
    if (!suffix) suffix = "";
    size_t len = strlen(base) + strlen(suffix);
    char *res = malloc(len + 1);
    if (!res) return NULL;
    strcpy(res, base);
    strcat(res, suffix);
    return res;
}

// Strip file extension from `filename` and return a static buffer.
const char *strip_extension(const char *filename) {
    static char buf[256];
    if (!filename) return NULL;
    strncpy(buf, filename, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *dot = strrchr(buf, '.');
    if (dot) *dot = '\0';
    return buf;
}

