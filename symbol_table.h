
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

/* Linked-list node recording a single use of an external symbol */
typedef struct ExternalUse {
    char name[32];   /* symbol name */
    int address;     /* address of the use */
    struct ExternalUse *next;
} ExternalUse;

/*
 * The symbol table is represented as a simple singly linked list.
 * We use a dummy head node (of type SymbolTable) whose 'next' pointer
 * points to the first real symbol.  This allows functions that modify
 * the list to receive a pointer to the head without needing a pointer
 * to a pointer in most call sites.
 */
typedef Symbol SymbolTable; /* alias for clarity (dummy head node) */

/* initialise an empty symbol table (sets the head's fields to defaults) */
void init_symbol_table(SymbolTable *table);

/* Add a label (code or data) to the table.  Returns false on duplicate. */
bool add_label(SymbolTable *table, const char *name, int address, bool is_data);

/* Add an external label to the table.  Returns false on duplicate. */
bool add_label_external(SymbolTable *table, const char *name);

/* Relocate all data symbols by adding 'offset' to their addresses. */
void relocate_data_symbols(SymbolTable *table, int offset);

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

/* Records a use of an external symbol at 'address' */
ExternalUse* add_external_use(ExternalUse **list, const char *name, int address);

/* Frees the list of external symbol uses */
void free_external_uses(ExternalUse *list);

#endif // SYMBOL_TABLE_H

