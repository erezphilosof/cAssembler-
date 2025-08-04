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

// Prints an error message and exits the program
void error_exit(const char* msg);

// Returns newly allocated filename without its extension
char* strip_extension(const char* filename);

// Returns newly allocated formatted string (like sprintf)
char* strcat_printf(const char* fmt, ...);

// Converts value into base-4 string with leading zeros (8 digits)
void convert_to_base4(uint16_t value, char* out);

// Replace all occurrences of pattern with replacement in-place
void replace_substring(char* str, const char* pattern, const char* replacement);

// Register helpers
bool is_register(const char* str);
int  reg_number(const char* str);

#endif // UTILS_H
