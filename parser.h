#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

#include "utils.h"          /* trim_string, is_valid_label, print_error */
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
bool  second_pass(FILE *src,
                  SymbolTable *symtab);

/* operand helpers */
int   parse_operands(const char *raw, char ops[][MAX_LINE_LEN]);
int   split_operands(const char *src, char ops[][MAX_LINE_LEN]);
void  clean_operand_whitespace(char *op);
bool  check_operand_syntax(const char *op);

/* directive handlers */
void  handle_data_directive(const ParsedLine *pl, int *DC);
void  handle_string_directive(const ParsedLine *pl, int *DC);
void  handle_mat_directive(const ParsedLine *pl, int *DC);
void  handle_entry_extern(SymbolTable *symtab, const ParsedLine *pl);

/* symbol table and counters */
void  add_label_to_symbol_table(SymbolTable *symtab, const ParsedLine *pl, int IC, int DC);
void  update_IC_DC(const ParsedLine *pl, int *IC, int *DC);
void  resolve_labels(SymbolTable *symtab, const ParsedLine *pl);

#endif /* PARSER_H */

