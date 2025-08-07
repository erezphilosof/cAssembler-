#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "parser.h"
#include "symbol_table.h"
#include "utils.h"
#include "registers.h"
#include "error.h"
#include "data_segment.h"

/* Detect matrix operand syntax: <label>[rX][rY]. Returns true if valid. */
static bool is_matrix_operand(const char *op) {
    const char *b1 = strchr(op, '[');
    if (!b1) return false;
    const char *b2 = strchr(b1 + 1, ']');
    const char *b3 = b2 ? strchr(b2 + 1, '[') : NULL;
    const char *b4 = b3 ? strchr(b3 + 1, ']') : NULL;
    if (!b2 || !b3 || !b4 || b3 != b2 + 1 || *(b4 + 1) != '\0' || b1 == op) {
        print_error("Invalid matrix operand");
        return false;
    }
    char r1[8], r2[8];
    strncpy(r1, b1 + 1, b2 - b1 - 1); r1[b2 - b1 - 1] = '\0';
    strncpy(r2, b3 + 1, b4 - b3 - 1); r2[b4 - b3 - 1] = '\0';
    if (!is_register(r1) || !is_register(r2)) {
        print_error("Invalid matrix register");
        return false;
    }
    return true;
}

/* Estimate number of machine words for an instruction */
static int count_instruction_words(const ParsedLine *pl) {
    int words = 1; /* base word */
    /* Instructions without operands (RTS/STOP) occupy just one word */
    if (strcasecmp(pl->opcode, "RTS") == 0 ||
        strcasecmp(pl->opcode, "STOP") == 0)
        return words;

    if (pl->operands_raw[0] == '\0')
        return words;

    char ops[2][80];
    int n = split_string(pl->operands_raw, ',', ops, 2);
    bool reg_op[2] = {false, false};
    bool mat_op[2] = {false, false};
    for (int i = 0; i < n; i++) {
        mat_op[i] = is_matrix_operand(ops[i]);
        if (!mat_op[i])
            reg_op[i] = is_register(ops[i]);
    }

    if (n == 1) {
        if (mat_op[0]) words += 2;
        else words += reg_op[0] ? 0 : 1;
    } else if (n == 2) {
        if (reg_op[0] && reg_op[1]) {
            words += 1; /* two registers share a word */
        } else {
            for (int i = 0; i < 2; i++) {
                if (mat_op[i]) words += 2;
                else if (!reg_op[i]) words++;
            }
        }
    }
    return words;
}

/* First pass: build symbol table, count IC/DC */
bool first_pass(FILE *src, SymbolTable *symtab, int *IC_out, int *DC_out, DataSegment *data_seg) {
    char *line = NULL;
    size_t linecap = 0;
    int IC = 0, DC = 0, ln = 0;

    while (getline(&line, &linecap, src) != -1) {
        ++ln;
        ParsedLine pl;
        if (!parse_line(line, &pl, ln)) {
            free(pl.directive_args);
            free(pl.operands_raw);
            free(line);
            line = NULL;
            continue; /* error already logged */
        }

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
                char tokens[64][80];
                int n = split_string(pl.directive_args, ',', tokens, 64);
                for (int i = 0; i < n; i++) {
                    errno = 0;
                    char *endptr;
                    long val = strtol(tokens[i], &endptr, 10);
                    if (errno != 0 || *endptr != '\0' || endptr == tokens[i] ||
                        val < -32768 || val > 32767) {
                        print_error("Invalid number: %s", tokens[i]);
                        val = 0;
                    }
                    append_data_word(data_seg, (uint16_t)val);
                }
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
                for (char *p = start + 1; p < end; ++p) {
                    append_data_word(data_seg, (uint16_t)(unsigned char)(*p));
                }
                append_data_word(data_seg, 0); /* null terminator */
                DC += (int)(end - start - 1) + 1;
                break;
            }
            case DIR_MAT: {
                char tokens[256][80];
                int n = split_string(pl.directive_args, ',', tokens, 256);
                if (n < 2) {
                    print_error("Invalid .mat directive");
                    break;
                }

                errno = 0;
                char *endptr;
                long rows = strtol(tokens[0], &endptr, 10);
                if (errno != 0 || *endptr != '\0' || endptr == tokens[0] ||
                    rows < -32768 || rows > 32767) {
                    print_error("Invalid number: %s", tokens[0]);
                    rows = 0;
                }

                errno = 0;
                endptr = NULL;
                long cols = strtol(tokens[1], &endptr, 10);
                if (errno != 0 || *endptr != '\0' || endptr == tokens[1] ||
                    cols < -32768 || cols > 32767) {
                    print_error("Invalid number: %s", tokens[1]);
                    cols = 0;
                }

                int expected = (int)(rows * cols);
                for (int i = 0; i < expected; i++) {
                    long val = 0;
                    if (i + 2 < n) {
                        errno = 0;
                        endptr = NULL;
                        val = strtol(tokens[i + 2], &endptr, 10);
                        if (errno != 0 || *endptr != '\0' || endptr == tokens[i + 2] ||
                            val < -32768 || val > 32767) {
                            print_error("Invalid number: %s", tokens[i + 2]);
                            val = 0;
                        }
                    }
                    append_data_word(data_seg, (uint16_t)val);
                }
                DC += expected;
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
            free(pl.directive_args);
            free(pl.operands_raw);
            free(line);
            line = NULL;
            continue;
        }

        /* handle instructions */
        if (pl.type == STMT_INSTRUCTION) {
            IC += count_instruction_words(&pl);
            free(pl.directive_args);
            free(pl.operands_raw);
            free(line);
            line = NULL;
            continue;
        }

        /* label-only or empty/comment: do nothing */
        free(pl.directive_args);
        free(pl.operands_raw);
        free(line);
        line = NULL;
    }

    free(line);

    /* relocate all data symbols by IC */
    relocate_data_symbols(symtab, IC);

    /* apply base address to all symbols */
    relocate_all_symbols(symtab, BASE_ADDRESS);

    if (IC_out) *IC_out = IC;
    if (DC_out) *DC_out = DC;
    return (get_error_count() == 0);
}

