// utils.c
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

