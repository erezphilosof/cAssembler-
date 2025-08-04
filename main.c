// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils.h"
#include "macro.h"
#include "parser.h"
#include "symbol_table.h"
#include "instructions.h"
#include "output.h"
#include "error.h"

static ParsedLine *all_lines = NULL;
static int         line_count = 0;

/* Read whole file into a lines[] array */
static bool read_input(const char *fname, char ***out_lines, int *out_n) {
    FILE *f = fopen(fname,"r");
    if (!f) { print_error("open %s: %s", fname, strerror(errno)); return false; }
    char **lines = malloc(sizeof(char*) * 2048);
    int n = 0;
    char buf[256];
    while (fgets(buf,sizeof(buf),f)) {
        lines[n++] = strdup(buf);
    }
    fclose(f);
    *out_lines = lines;
    *out_n     = n;
    return true;
}

/* Second pass: execute each instruction in order (fills cpu.memory) */
static bool second_pass(SymbolTable *st, CPUState *cpu) {
    for (int i = 0; i < line_count; i++) {
        ParsedLine *pl = &all_lines[i];
        if (pl->type != STMT_INSTRUCTION) continue;
        /* Dispatch based on opcode string */
        if      (strcasecmp(pl->opcode,"MOV")==0) exec_mov(pl,cpu);
        else if (strcasecmp(pl->opcode,"CMP")==0) exec_cmp(pl,cpu);
        else if (strcasecmp(pl->opcode,"ADD")==0) exec_add(pl,cpu);
        else if (strcasecmp(pl->opcode,"SUB")==0) exec_sub(pl,cpu);
        else if (strcasecmp(pl->opcode,"LEA")==0) exec_lea(pl,cpu);
        else if (strcasecmp(pl->opcode,"CLR")==0) exec_clr(pl,cpu);
        else if (strcasecmp(pl->opcode,"NOT")==0) exec_not(pl,cpu);
        else if (strcasecmp(pl->opcode,"INC")==0) exec_inc(pl,cpu);
        else if (strcasecmp(pl->opcode,"DEC")==0) exec_dec(pl,cpu);
        else if (strcasecmp(pl->opcode,"JMP")==0) exec_jmp(pl,cpu);
        else if (strcasecmp(pl->opcode,"BNE")==0) exec_bne(pl,cpu);
        else if (strcasecmp(pl->opcode,"JSR")==0) exec_jsr(pl,cpu);
        else if (strcasecmp(pl->opcode,"RED")==0) exec_red(pl,cpu);
        else if (strcasecmp(pl->opcode,"PRN")==0) exec_prn(pl,cpu);
        else if (strcasecmp(pl->opcode,"STOP")==0) exec_stop(pl,cpu);
        else {
            print_error("Unrecognized opcode in second pass");
            return false;
        }
        cpu->PC++;  /* advance to next memory cell */
    }
    return (get_error_count()==0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("Usage: %s <source.as>", argv[0]);
        return 1;
    }

    /* 1. Read + macro‐expand */
    char **raw; int raw_n;
    if (!read_input(argv[1], &raw, &raw_n)) return 1;
    MacroTable mt; init_macro_table(&mt);
    if (!scan_macros((const char**)raw,raw_n,&mt)) return 1;
    int flat_n;
    char **flat = expand_macros((const char**)raw,raw_n,&flat_n,&mt);

    /* 2. First pass */
    SymbolTable st; init_symbol_table(&st);
    ParsedLine *plarr = malloc(sizeof(*plarr)*flat_n);
    all_lines = plarr;  line_count = flat_n;

    FILE *tmp = tmpfile();
    for(int i=0;i<flat_n;i++) fputs(flat[i],tmp);
    fseek(tmp,0,SEEK_SET);

    int IC, DC;
    if (!first_pass(tmp, &st, &IC, &DC)) {
        print_error("First pass failed");
        return 1;
    }

    /* 3. Allocate CPU & memory */
    CPUState cpu = {0};
    cpu.memory = calloc(IC+DC, sizeof(uint16_t));
    cpu.PC     = 0;

    /* Re‐parse into ParsedLine structs */
    fseek(tmp,0,SEEK_SET);
    for (int i=0; i<flat_n; i++) {
        parse_line(flat[i], &plarr[i], i);
    }
    fclose(tmp);

    /* 4. Second pass: execute instructions */
    if (!second_pass(&st, &cpu)) {
        print_error("Second pass failed");
        return 1;
    }

    /* 5. Emit files */
    const char *base = strip_extension(argv[1]);
    write_object_file(   strcat_printf("%s.ob", base), cpu.memory, IC, DC);
    write_entries_file(  strcat_printf("%s.ent", base), &st);
    write_externals_file(strcat_printf("%s.ext", base), &st);

    /* Cleanup */
    free(cpu.memory);
    free_symbol_table(&st);
    for(int i=0;i<flat_n;i++) free(flat[i]);
    free(flat);
    for(int i=0;i<raw_n;i++) free(raw[i]);
    free(raw);
    free(plarr);

    return 0;
}

