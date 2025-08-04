#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_symbol_table(SymbolTable *st) {
    st->head = NULL;
    st->count = 0;
    st->externals = NULL;
    st->ext_count = 0;
    st->ext_capacity = 0;
}

static bool symbol_exists(const SymbolTable *st, const char *name) {
    return lookup_symbol(st, name) != NULL;
}

bool add_label(SymbolTable *st, const char *name, int address, bool is_data) {
    if (symbol_exists(st, name))
        return false;
    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym) return false;
    strncpy(sym->name, name, MAX_LABEL_LEN-1);
    sym->name[MAX_LABEL_LEN-1] = '\0';
    sym->address = address;
    sym->is_data = is_data;
    sym->is_entry = false;
    sym->is_external = false;
    sym->next = st->head;
    st->head = sym;
    st->count++;
    return true;
}

bool add_label_external(SymbolTable *st, const char *name) {
    if (!add_label(st, name, 0, false))
        return false;
    Symbol *sym = st->head; /* newly added */
    sym->is_external = true;
    return true;
}

Symbol *lookup_symbol(const SymbolTable *st, const char *name) {
    for (Symbol *s = st->head; s; s = s->next) {
        if (strcmp(s->name, name) == 0)
            return s;
    }
    return NULL;
}

void relocate_data_symbols(SymbolTable *st, int ic_offset) {
    for (Symbol *s = st->head; s; s = s->next) {
        if (s->is_data && !s->is_external)
            s->address += ic_offset;
    }
}

void mark_entry(SymbolTable *st, const char *name) {
    Symbol *s = lookup_symbol(st, name);
    if (s)
        s->is_entry = true;
}

static void ensure_ext_capacity(SymbolTable *st) {
    if (st->ext_count >= st->ext_capacity) {
        int newcap = st->ext_capacity ? st->ext_capacity * 2 : 4;
        st->externals = realloc(st->externals, newcap * sizeof(ExtRef));
        st->ext_capacity = newcap;
    }
}

void mark_external(SymbolTable *st, const char *name, int address) {
    ensure_ext_capacity(st);
    ExtRef *er = &st->externals[st->ext_count++];
    strncpy(er->name, name, MAX_LABEL_LEN-1);
    er->name[MAX_LABEL_LEN-1] = '\0';
    er->address = address;
}

void print_symbol_table(const SymbolTable *st) {
    printf("Symbol Table:\n");
    for (const Symbol *s = st->head; s; s = s->next) {
        printf("%-20s %04d %s%s%s\n", s->name, s->address,
               s->is_external ? "external" : (s->is_data ? "data" : "code"),
               s->is_entry ? " entry" : "",
               "");
    }
}

void free_symbol_table(SymbolTable *st) {
    Symbol *s = st->head;
    while (s) {
        Symbol *next = s->next;
        free(s);
        s = next;
    }
    free(st->externals);
    st->head = NULL;
    st->externals = NULL;
    st->count = st->ext_count = st->ext_capacity = 0;
}

