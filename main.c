// main.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "macro.h"
#include "parser.h"
#include "symbol_table.h"
#include "output.h"
#include "error.h"
#include "second_pass.h"
#include "data_segment.h"


/* Read whole file into a lines[] array */
static bool read_input(const char *fname, char ***out_lines, int *out_n) {
    FILE *f = fopen(fname,"r");
    if (!f) { perror("open"); return false; }
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
    DataSegment data_seg; init_data_segment(&data_seg);

    FILE *tmp = tmpfile();
    for(int i=0;i<flat_n;i++) fputs(flat[i],tmp);
    fseek(tmp,0,SEEK_SET);

    int IC, DC;
    if (!first_pass(tmp, &st, &IC, &DC, &data_seg)) {
        print_error("First pass failed");
        return 1;
    }

    /* 3. Allocate CPU & memory */
    CPUState cpu = {0};
    cpu.memory = calloc(IC+DC, sizeof(uint16_t));
    cpu.PC     = 0;
    cpu.symtab = &st;

    /* Re‐parse into ParsedLine structs */
    fseek(tmp,0,SEEK_SET);
    for (int i=0; i<flat_n; i++) {
        parse_line(flat[i], &plarr[i], i);
    }
    fclose(tmp);

    /* 4. Second pass: execute instructions */
    if (!second_pass(plarr, flat_n, &cpu)) {
        print_error("Second pass failed");
        return 1;
    }

    /* 5. Emit files */
    const char *base = strip_extension(argv[1]);
    char *fname;

    /* copy data segment after instructions */
    if (data_seg.count != DC) {
        print_error("Data count mismatch");
    } else {
        memcpy(cpu.memory + IC, data_seg.words, DC * sizeof(uint16_t));
    }

    fname = strcat_printf(base, ".ob");
    write_object_file(fname, cpu.memory, IC, DC);
    free(fname);

    fname = strcat_printf(base, ".ent");
    write_entries_file(fname, &st);
    free(fname);

    fname = strcat_printf(base, ".ext");
    write_externals_file(fname, &st);
    free(fname);

    /* Cleanup */
    free(cpu.memory);
    free_data_segment(&data_seg);
    free_symbol_table(&st);
    for(int i=0;i<flat_n;i++) free(flat[i]);
    free(flat);
    for(int i=0;i<raw_n;i++) free(raw[i]);
    free(raw);
    free(plarr);

    return 0;
}

