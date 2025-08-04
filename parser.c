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

/* Split comma-separated operands into array */
int split_operands(const char *src, char ops[][MAX_LINE_LEN]) {
    int count = 0;
    const char *p = src;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (*p=='\0') break;
        const char *start = p;
        while (*p && *p!=',') p++;
        size_t len = p-start;
        if (len >= MAX_LINE_LEN) len = MAX_LINE_LEN-1;
        strncpy(ops[count], start, len);
        ops[count][len]='\0';
        clean_operand_whitespace(ops[count]);
        count++;
        if (*p==',') p++;
        if (count>=3) break;
    }
    return count;
}

void clean_operand_whitespace(char *op) {
    trim_string(op);
}

bool check_operand_syntax(const char *op) {
    if (op[0]=='#') {
        /* immediate */
        const char *p=op+1;
        if (*p=='+'||*p=='-') p++;
        if (!isdigit((unsigned char)*p)) return false;
        while (isdigit((unsigned char)*p)) p++;
        return *p=='\0';
    }
    if (is_register(op)) return true;
    return is_valid_label(op);
}

int parse_operands(const char *raw, char ops[][MAX_LINE_LEN]) {
    int n = split_operands(raw, ops);
    for (int i=0;i<n;i++) {
        if (!check_operand_syntax(ops[i])) {
            print_error("Bad operand syntax");
        }
    }
    return n;
}

void handle_data_directive(const ParsedLine *pl, int *DC) {
    char buf[MAX_LINE_LEN];
    strncpy(buf, pl->directive_args, sizeof(buf));
    buf[sizeof(buf)-1]='\0';
    int n=0;
    char *tok=strtok(buf,",");
    while(tok){ n++; tok=strtok(NULL,","); }
    *DC += n;
}

void handle_string_directive(const ParsedLine *pl, int *DC) {
    char *start = strchr(pl->directive_args,'"');
    char *end   = start ? strrchr(pl->directive_args,'"') : NULL;
    if (!start || !end || end<=start) {
        print_error("Bad string directive");
        return;
    }
    *DC += (int)(end-start-1) + 1; /* include null terminator */
}

void handle_mat_directive(const ParsedLine *pl, int *DC) {
    (void)pl;
    (*DC)++; /* simplistic placeholder */
}

void handle_entry_extern(SymbolTable *symtab, const ParsedLine *pl) {
    if (pl->dir_type == DIR_EXTERN) {
        add_symbol(symtab, pl->directive_args, 0, SYM_EXTERNAL);
    } else if (pl->dir_type == DIR_ENTRY) {
        update_symbol_type(symtab, pl->directive_args, SYM_ENTRY);
    }
}

void add_label_to_symbol_table(SymbolTable *symtab, const ParsedLine *pl, int IC, int DC) {
    if (!pl->has_label) return;
    bool is_data = (pl->type==STMT_DIRECTIVE &&
                   (pl->dir_type==DIR_DATA || pl->dir_type==DIR_STRING || pl->dir_type==DIR_MAT));
    int addr = is_data ? DC : IC;
    add_symbol(symtab, pl->label, addr, is_data?SYM_DATA:SYM_CODE);
}

void update_IC_DC(const ParsedLine *pl, int *IC, int *DC) {
    if (pl->type == STMT_DIRECTIVE) {
        switch(pl->dir_type) {
        case DIR_DATA:   handle_data_directive(pl, DC); break;
        case DIR_STRING: handle_string_directive(pl, DC); break;
        case DIR_MAT:    handle_mat_directive(pl, DC); break;
        case DIR_ENTRY:
        case DIR_EXTERN:
            /* no change to counts */
            break;
        default:
            break;
        }
    } else if (pl->type == STMT_INSTRUCTION) {
        (*IC)++;
    }
}

void resolve_labels(SymbolTable *symtab, const ParsedLine *pl) {
    if (pl->type != STMT_INSTRUCTION) return;
    char ops[3][MAX_LINE_LEN];
    int n = parse_operands(pl->operands_raw, ops);
    for (int i=0;i<n;i++) {
        if (ops[i][0] != '#' && !is_register(ops[i])) {
            if (!find_symbol(symtab, ops[i]))
                print_error("Undefined label");
        }
    }
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

        add_label_to_symbol_table(symtab, &pl, IC, DC);
        update_IC_DC(&pl, &IC, &DC);
    }

    /* relocate all data symbols by IC */
    relocate_data_symbols(symtab, IC);

    if (IC_out) *IC_out = IC;
    if (DC_out) *DC_out = DC;
    return (get_error_count() == 0);
}

/* Second pass: resolve labels, handle .entry/.extern */
bool second_pass(FILE *src, SymbolTable *symtab) {
    char line[MAX_LINE_LEN];
    int ln=0;
    while (fgets(line, sizeof(line), src)) {
        ++ln;
        ParsedLine pl;
        if (!parse_line(line, &pl, ln))
            continue;
        if (pl.type==STMT_DIRECTIVE &&
            (pl.dir_type==DIR_ENTRY || pl.dir_type==DIR_EXTERN)) {
            handle_entry_extern(symtab, &pl);
        }
        if (pl.type==STMT_INSTRUCTION) {
            resolve_labels(symtab, &pl);
        }
    }
    return (get_error_count()==0);
}

