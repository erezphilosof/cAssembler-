#include <stdio.h>
#include "parser.h"
#include "symbol_table.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.as>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("open"); return 1; }
    SymbolTable st; init_symbol_table(&st);
    int IC=0, DC=0;
    if (!first_pass(f, &st, &IC, &DC)) {
        fprintf(stderr, "First pass failed\n");
        fclose(f);
        free_symbol_table(&st);
        return 1;
    }
    fseek(f,0,SEEK_SET);
    if (!second_pass(f, &st)) {
        fprintf(stderr, "Second pass failed\n");
        fclose(f);
        free_symbol_table(&st);
        return 1;
    }
    fclose(f);
    print_symbol_table(&st);
    free_symbol_table(&st);
    return 0;
}
