#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"

/* trim in-place, remove comments after ';' */
static void normalize(char *s) {
    /* strip comment */
    char *c = strchr(s, ';');
    if (c) *c = '\0';
    trim_string(s);            /* leading/trailing spaces, from utils */
}

/* Identify empty/comment/directive/instruction/label-only */
static StatementType identify_statement_type(const char *s) {
    if (*s == '\0')        return STMT_EMPTY;
    if (*s == ';')         return STMT_COMMENT;
    if (*s == '.')         return STMT_DIRECTIVE;
    if (strchr(s, ':'))    return STMT_LABEL_ONLY;
    /* else assume mnemonic */
    return STMT_INSTRUCTION;
}

/* Map directive token -> enum */
static DirectiveType directive_from_token(const char *tok) {
    if (strcasecmp(tok, "data")   == 0) return DIR_DATA;
    if (strcasecmp(tok, "string") == 0) return DIR_STRING;
    if (strcasecmp(tok, "mat")    == 0) return DIR_MAT;
    if (strcasecmp(tok, "entry")  == 0) return DIR_ENTRY;
    if (strcasecmp(tok, "extern") == 0) return DIR_EXTERN;
    return DIR_INVALID;
}

/* Count comma-separated items in data directive */
static int count_data_items(const char *args) {
    int count = 0;
    char buf[MAX_LINE_LEN];
    strncpy(buf, args, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    char *tok = strtok(buf, ",");
    while (tok) {
        ++count;
        tok = strtok(NULL, ",");
    }
    return count;
}

/* Parse one line into ParsedLine */
bool parse_line(const char *src, ParsedLine *out, int line_no) {
    char buf[MAX_LINE_LEN];
    strncpy(buf, src, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    normalize(buf);

    memset(out, 0, sizeof(*out));
    out->line_number = line_no;

    StatementType st = identify_statement_type(buf);
    out->type = st;
    if (st==STMT_EMPTY || st==STMT_COMMENT) return true;

    char *p = buf;

    /* LABEL: handling */
    if (st==STMT_LABEL_ONLY || strchr(p, ':')) {
        char *col = strchr(p, ':');
        size_t len = col - p;
        if (len >= MAX_LABEL_LEN) {
            print_error("Line too long label");
            return false;
        }
        strncpy(out->label, p, len);
        out->label[len] = '\0';
        if (!is_valid_label(out->label)) {
            print_error("Invalid label name");
            return false;
        }
        out->has_label = true;
        p = col+1;
        trim_string(p);
        if (*p=='\0') {
            out->type = STMT_LABEL_ONLY;
            return true;
        }
        /* recalc kind */
        st = identify_statement_type(p);
        out->type = st;
    }

    /* Directive */
    if (out->type==STMT_DIRECTIVE) {
        /* read ".token" */
        if (p[0]!='.') {
            print_error("Directive missing dot");
            return false;
        }
        char tok[MAX_OPCODE_LEN];
        if (sscanf(p+1, "%9s", tok)!=1) {
            print_error("Malformed directive");
            return false;
        }
        DirectiveType dt = directive_from_token(tok);
        if (dt==DIR_INVALID) {
            print_error("Unknown directive");
            return false;
        }
        out->dir_type = dt;
        /* skip ".tok" + whitespace */
        p = p+1+strlen(tok);
        trim_string(p);
        strncpy(out->directive_args, p, MAX_LINE_LEN-1);
        return true;
    }

    /* Instruction */
    if (out->type==STMT_INSTRUCTION) {
        /* read opcode */
        char opc[MAX_OPCODE_LEN];
        if (sscanf(p, "%9s", opc)!=1) {
            print_error("Missing opcode");
            return false;
        }
        strncpy(out->opcode, opc, MAX_OPCODE_LEN-1);
        /* skip it */
        p += strlen(opc);
        trim_string(p);
        strncpy(out->operands_raw, p, MAX_LINE_LEN-1);
        return true;
    }

    print_error("Unhandled line");
    return false;
}

/* First pass: build symbol table, count IC/DC */
bool first_pass(FILE *src, SymbolTable *symtab, int *IC_out, int *DC_out) {
    char line[MAX_LINE_LEN];
    int IC=0, DC=0, ln=0;

    while (fgets(line, sizeof(line), src)) {
        ++ln;
        ParsedLine pl;
        if (!parse_line(line, &pl, ln))
            continue;  /* error already logged */

        /* label addition */
        if (pl.has_label && pl.type!=STMT_LABEL_ONLY) {
            bool is_data = (pl.type==STMT_DIRECTIVE &&
                           (pl.dir_type==DIR_DATA ||
                            pl.dir_type==DIR_STRING ||
                            pl.dir_type==DIR_MAT));
            add_label(symtab, pl.label, is_data ? DC : IC, is_data);
        }

        /* handle directives */
        if (pl.type==STMT_DIRECTIVE) {
            switch (pl.dir_type) {
            case DIR_DATA: {
                int n = count_data_items(pl.directive_args);
                DC += n;
                break;
            }
            case DIR_STRING: {
                /* count length of quoted string + 1 for '\0' */
                char *start = strchr(pl.directive_args, '"');
                if (!start) {
                    print_error("Missing opening quote");
                    break;
                }
                char *end = strrchr(pl.directive_args, '"');
                if (!end || end==start) {
                    print_error("Missing closing quote");
                    break;
                }
                DC += (int)(end - start - 1) + 1;
                break;
            }
            case DIR_MAT:
                DC += 1;  /* placeholder; user to refine */
                break;
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
        if (pl.type==STMT_INSTRUCTION) {
            IC += 1;  /* placeholder: one word per instruction */
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

