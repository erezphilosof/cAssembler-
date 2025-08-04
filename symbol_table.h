#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

#ifndef MAX_LABEL_LEN
#define MAX_LABEL_LEN 32
#endif

/* representation of an external reference */
typedef struct {
    char name[MAX_LABEL_LEN];
    int address;            /* address of use */
} ExtRef;

/* linked list node for a symbol */
typedef struct Symbol {
    char name[MAX_LABEL_LEN];
    int  address;
    bool is_data;
    bool is_entry;
    bool is_external;
    struct Symbol *next;
} Symbol;

/* the symbol table container */
typedef struct {
    Symbol  *head;          /* linked-list of symbols */
    int      count;         /* number of symbols */
    ExtRef  *externals;     /* dynamic array of external references */
    int      ext_count;
    int      ext_capacity;
} SymbolTable;

void init_symbol_table(SymbolTable *st);
bool add_label(SymbolTable *st, const char *name, int address, bool is_data);
bool add_label_external(SymbolTable *st, const char *name);
Symbol *lookup_symbol(const SymbolTable *st, const char *name);
void relocate_data_symbols(SymbolTable *st, int ic_offset);
void mark_entry(SymbolTable *st, const char *name);
void mark_external(SymbolTable *st, const char *name, int address);
void print_symbol_table(const SymbolTable *st);
void free_symbol_table(SymbolTable *st);

#endif /* SYMBOL_TABLE_H */
