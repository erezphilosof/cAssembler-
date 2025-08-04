// symbol_table.c
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_symbol_table(SymbolTable *table) {
    if (table) table->head = NULL;
}

/* Adds a new symbol to the table. Returns pointer to new symbol (or NULL if duplicate). */
Symbol* add_symbol(SymbolTable *table, const char* name, int address, SymbolType type) {
    if (!table || !name) return NULL;
    Symbol *existing = find_symbol(table, name);
    if (existing) return NULL; /* duplicate */
    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym) return NULL;
    strncpy(sym->name, name, sizeof(sym->name)-1);
    sym->name[sizeof(sym->name)-1] = '\0';
    sym->address = address;
    sym->type = type;
    sym->next = table->head;
    table->head = sym;
    return sym;
}

/* Finds a symbol by name. */
Symbol* find_symbol(SymbolTable *table, const char* name) {
    Symbol *cur = table ? table->head : NULL;
    while (cur) {
        if (strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Updates the type of a symbol. */
bool update_symbol_type(SymbolTable *table, const char* name, SymbolType new_type) {
    Symbol *sym = find_symbol(table, name);
    if (!sym) return false;
    sym->type = new_type;
    return true;
}

/* Relocate data symbols by IC */
void relocate_data_symbols(SymbolTable *table, int IC) {
    Symbol *cur = table ? table->head : NULL;
    while (cur) {
        if (cur->type == SYM_DATA)
            cur->address += IC;
        cur = cur->next;
    }
}

/* Prints all symbols (for debug) */
void print_symbol_table(const SymbolTable *table) {
    const Symbol *cur = table ? table->head : NULL;
    printf("Symbol Table:\n");
    printf("%-20s %-8s %-8s\n", "Name", "Address", "Type");
    while (cur) {
        const char* type_str =
            cur->type == SYM_CODE ? "code" :
            cur->type == SYM_DATA ? "data" :
            cur->type == SYM_ENTRY ? "entry" : "external";
        printf("%-20s %-8d %-8s\n", cur->name, cur->address, type_str);
        cur = cur->next;
    }
}

/* Frees all memory of the table */
void free_symbol_table(SymbolTable *table) {
    if (!table) return;
    Symbol *cur = table->head;
    while (cur) {
        Symbol *next = cur->next;
        free(cur);
        cur = next;
    }
    table->head = NULL;
}

