// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>

// Removes whitespace from the beginning and end of the string (in place)
void trim_string(char* str);

// Returns true if the string is empty or contains only whitespace
bool is_whitespace(const char* str);

// Returns true if the line is a comment (starts with ; or is empty)
bool is_comment(const char* str);

// Splits a string into tokens by delimiter, returns the number of tokens found
// Each token is trimmed. Maximum length per token is 79 chars.
int split_string(const char* str, char delimiter, char tokens[][80], int max_tokens);

// Converts a string to uppercase (in place)
void to_upper_case(char* str);

// Returns true if the given string is a valid label name (starts with a letter, contains only alphanumeric or _)
bool is_valid_label(const char* str);

// Converts a 16-bit word to base-4 string (8 digits, null-terminated)
void convert_to_base4(uint16_t value, char *out);

// Prints an error message and exits the program
void error_exit(const char* msg);

// Utility helpers used across the assembler
const char* strip_extension(const char* filename);
const char* strcat_printf(const char* fmt, const char* arg);
void replace_substring(char* str, const char* pattern, const char* repl);

#endif // UTILS_H
