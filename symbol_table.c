// symbol_table.c
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Adds a new symbol to the table. Returns pointer to new symbol (or NULL if duplicate).
Symbol* add_symbol(Symbol** table, const char* name, int address, SymbolType type) {
    if (!table || !name) return NULL;
    // Check for duplicates
    Symbol* existing = find_symbol(*table, name);
    if (existing) return NULL; // Duplicate
    Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
    if (!sym) return NULL;
    strncpy(sym->name, name, 31);
    sym->name[31] = '\0';
    sym->address = address;
    sym->type = type;
    sym->next = *table;
    *table = sym;
    return sym;
}

// Finds a symbol by name. Returns pointer if found, else NULL.
Symbol* find_symbol(Symbol* table, const char* name) {
    while (table) {
        if (strcmp(table->name, name) == 0)
            return table;
        table = table->next;
    }
    return NULL;
}

/* Convenience wrapper for external users */
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    return find_symbol(table, name);
}

// Updates the type of a symbol (e.g., for marking as entry or external).
bool update_symbol_type(Symbol* table, const char* name, SymbolType new_type) {
    Symbol* sym = find_symbol(table, name);
    if (!sym) return false;
    sym->type = new_type;
    return true;
}

// Prints all symbols (for debug)
void print_symbol_table(Symbol* table) {
    printf("Symbol Table:\n");
    printf("%-20s %-8s %-6s\n", "Name", "Address", "Type");
    while (table) {
        const char* type_str =
            table->type == SYM_CODE ? "code" :
            table->type == SYM_DATA ? "data" :
            table->type == SYM_ENTRY ? "entry" : "external";
        printf("%-20s %-8d %-8s\n", table->name, table->address, type_str);
        table = table->next;
    }
}

// Frees all memory of the table
void free_symbol_table(Symbol* table) {
    while (table) {
        Symbol* next = table->next;
        free(table);
        table = next;
    }
}

