
// symbol_table.h
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// Symbol type: code, data, entry, external
typedef enum {
    SYM_CODE,
    SYM_DATA,
    SYM_EXTERNAL,
    SYM_ENTRY
} SymbolType;

// The symbol structure (linked list node)
typedef struct Symbol {
    char            name[32];
    int             address;
    SymbolType      type;
    struct Symbol  *next;
} Symbol;

// Wrapper for head pointer so callers can treat it as an object
typedef struct {
    Symbol *head;
} SymbolTable;

/* Initialize empty table */
void     init_symbol_table(SymbolTable *table);

/* Adds a new symbol. Returns pointer to new symbol or NULL on duplicate/alloc failure */
Symbol*  add_symbol(SymbolTable *table, const char* name, int address, SymbolType type);

/* Find a symbol by name */
Symbol*  find_symbol(SymbolTable *table, const char* name);

/* Update symbol type (used for .entry/.extern) */
bool     update_symbol_type(SymbolTable *table, const char* name, SymbolType new_type);

/* Relocate data symbols by adding IC to their addresses */
void     relocate_data_symbols(SymbolTable *table, int IC);

/* Print table (debug) */
void     print_symbol_table(const SymbolTable *table);

/* Free all memory */
void     free_symbol_table(SymbolTable *table);

#endif // SYMBOL_TABLE_H

