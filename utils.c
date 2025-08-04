// utils.c
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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

/* Check if operand string is a register */
bool is_register(const char *s) {
    if (s && (s[0]=='r' || s[0]=='R') && isdigit((unsigned char)s[1]) && s[2]=='\0') {
        int n = s[1]-'0';
        return n>=0 && n<=7;
    }
    return false;
}

/* Return register number or -1 */
int reg_number(const char *s) {
    if (is_register(s))
        return s[1]-'0';
    return -1;
}

/* Convert value to base4 string */
void convert_to_base4(unsigned int value, char *out) {
    char buf[32]; int i=0;
    do {
        buf[i++] = "0123"[value % 4];
        value /= 4;
    } while (value && i<31);
    buf[i]='\0';
    /* reverse */
    for (int j=0; j<i; j++) out[j] = buf[i-1-j];
    out[i]='\0';
}

/* Strip extension from filename */
const char *strip_extension(const char *filename) {
    static char buf[256];
    strncpy(buf, filename, sizeof(buf));
    buf[sizeof(buf)-1]='\0';
    char *dot = strrchr(buf,'.');
    if (dot) *dot='\0';
    return buf;
}

/* Simple snprintf wrapper using static buffer */
char *strcat_printf(const char *fmt, const char *arg) {
    static char buf[256];
    snprintf(buf, sizeof(buf), fmt, arg);
    return buf;
}

