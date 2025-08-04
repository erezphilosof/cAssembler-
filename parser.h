#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

#include "utils.h"          /* trim_string, is_valid_label */
#include "symbol_table.h"   /* add_label, add_label_external, relocate_data_symbols */
#include "error.h"          /* get_error_count */

#define MAX_LINE_LEN     256
#define MAX_LABEL_LEN     32
#define MAX_OPCODE_LEN    10

/* What kind of statement we found on a line */
typedef enum {
    STMT_EMPTY,
    STMT_COMMENT,
    STMT_LABEL_ONLY,     /* “LABEL:” alone */
    STMT_DIRECTIVE,
    STMT_INSTRUCTION
} StatementType;

/* Which directive type (after the dot) */
typedef enum {
    DIR_DATA,
    DIR_STRING,
    DIR_MAT,
    DIR_ENTRY,
    DIR_EXTERN,
    DIR_INVALID
} DirectiveType;

/* Parsed info for one line of source */
typedef struct {
    StatementType type;
    int           line_number;

    bool          has_label;
    char          label[MAX_LABEL_LEN];

    /* directive-specific */
    DirectiveType dir_type;
    char          directive_args[MAX_LINE_LEN];

    /* instruction-specific */
    char          opcode[MAX_OPCODE_LEN];
    char          operands_raw[MAX_LINE_LEN];
} ParsedLine;

/* Public API */
bool  parse_line(const char *src, ParsedLine *out, int line_no);
bool  first_pass(FILE *src,
                 SymbolTable *symtab,
                 int *IC_out,
                 int *DC_out);

#endif /* PARSER_H */

