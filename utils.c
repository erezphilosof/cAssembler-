// utils.c
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

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
    return true;
}

// Prints an error message and exits the program
void error_exit(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Returns newly allocated filename without its extension
char* strip_extension(const char* filename) {
    const char *dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);
    char *res = malloc(len + 1);
    if (!res) return NULL;
    strncpy(res, filename, len);
    res[len] = '\0';
    return res;
}

// Returns newly allocated formatted string (like sprintf)
char* strcat_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *res = malloc(len + 1);
    if (!res) return NULL;
    va_start(ap, fmt);
    vsnprintf(res, len + 1, fmt, ap);
    va_end(ap);
    return res;
}

// Converts value into base-4 string with leading zeros (8 digits)
void convert_to_base4(uint16_t value, char* out) {
    for (int i = 7; i >= 0; --i) {
        out[i] = '0' + (value & 0x3);
        value >>= 2;
    }
    out[8] = '\0';
}

// Replace all occurrences of pattern with replacement in-place
void replace_substring(char* str, const char* pattern, const char* replacement) {
    size_t pat_len = strlen(pattern);
    size_t repl_len = strlen(replacement);
    const char *p = str;
    int count = 0;
    while ((p = strstr(p, pattern)) != NULL) {
        count++;
        p += pat_len;
    }
    size_t orig_len = strlen(str);
    size_t new_len = orig_len + (repl_len > pat_len ? (repl_len - pat_len) * count : 0) + 1;
    char *result = malloc(new_len);
    if (!result) return;
    char *outp = result;
    const char *cur = str;
    while ((p = strstr(cur, pattern)) != NULL) {
        size_t len = p - cur;
        memcpy(outp, cur, len);
        outp += len;
        memcpy(outp, replacement, repl_len);
        outp += repl_len;
        cur = p + pat_len;
    }
    strcpy(outp, cur);
    strcpy(str, result);
    free(result);
}

// Register helpers
bool is_register(const char* str) {
    return (str && (str[0] == 'r' || str[0] == 'R') &&
            str[1] >= '0' && str[1] <= '7' && str[2] == '\0');
}

int reg_number(const char* str) {
    if (!is_register(str)) return -1;
    return str[1] - '0';
}

