#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "symbol_table.h"
#include "utils.h"
#include "registers.h"
#include "error.h"

/* Count comma-separated items using utils split_string */
static int count_data_items(const char *args) {
    char tokens[64][80];
    return split_string(args, ',', tokens, 64);
}

/* Estimate number of machine words for an instruction */
static int count_instruction_words(const ParsedLine *pl) {
    int words = 1; /* base word */
    if (pl->operands_raw[0] == '\0')
        return words;

    char ops[2][80];
    int n = split_string(pl->operands_raw, ',', ops, 2);
    bool reg_op[2] = {false, false};
    for (int i = 0; i < n; i++)
        reg_op[i] = is_register(ops[i]);

    if (n == 1) {
        words += reg_op[0] ? 0 : 1;
    } else if (n == 2) {
        if (reg_op[0] && reg_op[1]) {
            words += 1; /* two registers share a word */
        } else {
            if (!reg_op[0]) words++;
            if (!reg_op[1]) words++;
        }
    }
    return words;
}

/* First pass: build symbol table, count IC/DC */
bool first_pass(FILE *src, SymbolTable *symtab, int *IC_out, int *DC_out) {
    char line[MAX_LINE_LEN];
    int IC = 0, DC = 0, ln = 0;

    while (fgets(line, sizeof(line), src)) {
        ++ln;
        ParsedLine pl;
        if (!parse_line(line, &pl, ln))
            continue; /* error already logged */

        /* label addition */
        if (pl.has_label && pl.type != STMT_LABEL_ONLY) {
            bool is_data = (pl.type == STMT_DIRECTIVE &&
                           (pl.dir_type == DIR_DATA ||
                            pl.dir_type == DIR_STRING ||
                            pl.dir_type == DIR_MAT));
            add_label(symtab, pl.label, is_data ? DC : IC, is_data);
        }

        /* handle directives */
        if (pl.type == STMT_DIRECTIVE) {
            switch (pl.dir_type) {
            case DIR_DATA: {
                int n = count_data_items(pl.directive_args);
                DC += n;
                break;
            }
            case DIR_STRING: {
                char *start = strchr(pl.directive_args, '"');
                if (!start) {
                    print_error("Missing opening quote");
                    break;
                }
                char *end = strrchr(pl.directive_args, '"');
                if (!end || end == start) {
                    print_error("Missing closing quote");
                    break;
                }
                DC += (int)(end - start - 1) + 1;
                break;
            }
            case DIR_MAT: {
                char toks[3][80];
                int n = split_string(pl.directive_args, ',', toks, 3);
                if (n < 2) {
                    print_error("Invalid .mat directive");
                    break;
                }
                int rows = atoi(toks[0]);
                int cols = atoi(toks[1]);
                DC += rows * cols;
                break;
            }
            case DIR_EXTERN:
                add_label_external(symtab, pl.directive_args);
                break;
            case DIR_ENTRY:
                /* entry resolved in second pass */
                break;
            default:
                print_error("Unsupported directive");
            }
            continue;
        }

        /* handle instructions */
        if (pl.type == STMT_INSTRUCTION) {
            IC += count_instruction_words(&pl);
            continue;
        }

        /* label-only or empty/comment: do nothing */
    }

    /* relocate all data symbols by IC */
    relocate_data_symbols(symtab, IC);

    if (IC_out) *IC_out = IC;
    if (DC_out) *DC_out = DC;
    return (get_error_count() == 0);
}

