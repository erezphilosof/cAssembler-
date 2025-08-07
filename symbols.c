#include "symbol_table.h"
#include "utils.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* initialise an empty symbol table */
void init_symbol_table(SymbolTable *table) {
    if (!table) return;
    table->name[0] = '\0';
    table->address = 0;
    table->type = SYM_CODE; /* default, not really used for head */
    table->next = NULL;
}

/*
 * Add a new label (code or data) to the symbol table.
 * Returns true on success, false if label already exists.
 */
bool add_label(SymbolTable *table, const char *name, int address, bool is_data) {
    if (!table || !name) return false;
    SymbolType type = is_data ? SYM_DATA : SYM_CODE;
    /* add_symbol checks for duplicates; we use the list after the dummy head */
    Symbol *sym = add_symbol(&table->next, name, address, type);
    if (!sym) {
        print_error("Duplicate symbol: %s", name);
        return false;
    }
    return true;
}

/*
 * Add an external label.  Externals have address 0 and type SYM_EXTERNAL.
 */
bool add_label_external(SymbolTable *table, const char *name) {
    if (!table || !name) return false;
    char buf[32];
    strncpy(buf, name, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';
    trim_string(buf);
    if (!is_valid_label(buf)) {
        if (is_reserved_word(buf))
            print_error("Label cannot be a reserved word");
        else
            print_error("Invalid label name");
        return false;
    }
    Symbol *sym = add_symbol(&table->next, buf, 0, SYM_EXTERNAL);
    if (!sym) {
        print_error("Duplicate symbol: %s", buf);
        return false;
    }
    return true;
}

/*
 * Relocate all data symbols by adding 'offset' to their address.
 */
void relocate_data_symbols(SymbolTable *table, int offset) {
    if (!table) return;
    Symbol *curr = table->next;
    while (curr) {
        if (curr->type == SYM_DATA)
            curr->address += offset;
        curr = curr->next;
    }
}

/* Relocate all symbols (code and data) by adding 'offset'. */
void relocate_all_symbols(SymbolTable *table, int offset) {
    if (!table) return;
    Symbol *curr = table->next;
    while (curr) {
        curr->address += offset;
        curr = curr->next;
    }
}

