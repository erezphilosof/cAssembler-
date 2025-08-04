#include <stdio.h>
#include <string.h>
#include <strings.h>
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
