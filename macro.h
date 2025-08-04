#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_MACRO_NAME   32
#define MAX_MACRO_PARAMS  8
#define MAX_LINE_LEN    256
#define MAX_MACROS      64
#define MAX_LINES      1024

/* One macro definition */
typedef struct {
    char        name[MAX_MACRO_NAME];
    int         param_count;
    char        params[MAX_MACRO_PARAMS][MAX_MACRO_NAME];
    char       *body[MAX_LINES];
    int         body_len;
} MacroDef;

/* A table of all macros in this file */
typedef struct {
    MacroDef macros[MAX_MACROS];
    int       count;
} MacroTable;

/* Public API: */
void     init_macro_table(MacroTable *mt);
bool     scan_macros(const char *lines[], int line_count, MacroTable *mt);
/* Take input lines + macro table → produce output lines (caller frees) */
char   **expand_macros(const char *lines[], int in_count, int *out_count, MacroTable *mt);

#endif /* MACRO_H */

