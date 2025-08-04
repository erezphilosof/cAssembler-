
// symbol_table.h
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// Symbol type: code, data, entry, external
typedef enum {
    SYM_CODE,
    SYM_DATA,
    SYM_ENTRY,
    SYM_EXTERNAL
} SymbolType;

// The symbol structure
typedef struct Symbol {
    char name[32];
    int address;
    SymbolType type;
    struct Symbol* next;
} Symbol;

typedef Symbol SymbolTable; /* alias for clarity */

// Adds a new symbol to the table. Returns pointer to new symbol (or NULL if duplicate).
Symbol* add_symbol(Symbol** table, const char* name, int address, SymbolType type);

// Finds a symbol by name. Returns pointer if found, else NULL.
Symbol* find_symbol(Symbol* table, const char* name);

/* Convenience wrapper to find a symbol by name */
Symbol* lookup_symbol(SymbolTable* table, const char* name);

// Updates the type of a symbol (e.g., for marking as entry or external).
bool update_symbol_type(Symbol* table, const char* name, SymbolType new_type);

// Prints all symbols (for debug)
void print_symbol_table(Symbol* table);

// Frees all memory of the table
void free_symbol_table(Symbol* table);

#endif // SYMBOL_TABLE_H

