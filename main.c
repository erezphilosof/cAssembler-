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

static bool assemble_file(const char *fname) {
    bool ok = false;
    char **raw = NULL; int raw_n = 0;
    char **flat = NULL; int flat_n = 0;
    MacroTable mt; init_macro_table(&mt);
    SymbolTable st; init_symbol_table(&st);
    ParsedLine *plarr = NULL;
    DataSegment data_seg; init_data_segment(&data_seg);
    FILE *tmp = NULL;
    CPUState cpu = {0};
    int IC = 0, DC = 0;

    if (!read_input(fname, &raw, &raw_n)) goto cleanup;
    if (!scan_macros((const char**)raw, raw_n, &mt)) goto cleanup;
    flat = expand_macros((const char**)raw, raw_n, &flat_n, &mt);

    plarr = malloc(sizeof(*plarr) * flat_n);
    if (!plarr) goto cleanup;

    tmp = tmpfile();
    if (!tmp) goto cleanup;
    for (int i = 0; i < flat_n; i++) fputs(flat[i], tmp);
    fseek(tmp, 0, SEEK_SET);

    if (!first_pass(tmp, &st, &IC, &DC, &data_seg)) {
        print_error("First pass failed");
        goto cleanup;
    }

    cpu.memory = calloc(IC, sizeof(uint16_t));
    cpu.PC = 0;
    cpu.symtab = &st;
    if (!cpu.memory) goto cleanup;

    fseek(tmp, 0, SEEK_SET);
    for (int i = 0; i < flat_n; i++) {
        parse_line(flat[i], &plarr[i], i);
    }
    fclose(tmp); tmp = NULL;

    if (!second_pass(plarr, flat_n, &cpu)) {
        print_error("Second pass failed");
        goto cleanup;
    }

    const char *base = strip_extension(fname);
    char *outname;

    if (data_seg.count != DC) {
        print_error("Data count mismatch");
    }

    outname = strcat_printf(base, ".ob");
    write_object_file(outname, cpu.memory, IC, data_seg.words, DC, 0);
    free(outname);

    outname = strcat_printf(base, ".ent");
    if (!write_entries_file(outname, &st))
        remove(outname);
    free(outname);

    outname = strcat_printf(base, ".ext");
    if (!write_externals_file(outname, cpu.ext_uses))
        remove(outname);
    free(outname);

    ok = true;

cleanup:
    if (tmp) fclose(tmp);
    if (cpu.memory) free(cpu.memory);
    free_data_segment(&data_seg);
    free_external_uses(cpu.ext_uses);
    free_symbol_table(&st);
    /* free all macro definitions */
    free_macro_table(&mt);
    if (flat) { for (int i = 0; i < flat_n; i++) free(flat[i]); free(flat); }
    if (raw) { for (int i = 0; i < raw_n; i++) free(raw[i]); free(raw); }
    free(plarr);
    return ok;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_error("Usage: %s <source.as> [source2.as ...]", argv[0]);
        return 1;
    }

    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (!assemble_file(argv[i]))
            status = 1;
    }
    return status;
}

