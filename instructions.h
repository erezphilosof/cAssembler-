#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "parser.h"       /* ParsedLine */
#include "symbol_table.h" /* lookup_symbol */
#include "registers.h"    /* is_register, reg_number */
#include "error.h"        /* print_error */

/* CPU state (registers, flags, memory pointer, program counter) */
typedef struct {
    uint16_t *memory;     /* pointer to assembled instruction words */
    uint16_t  PC;         /* program counter */
    uint16_t  regs[8];    /* R0..R7 (unused for encoding but kept for compatibility) */
    bool      zero_flag;
    bool      sign_flag;
    SymbolTable *symtab;  /* symbol table for label resolution */
    ExternalUse *ext_uses; /* list of external symbol usages */
} CPUState;

/* Encode an instruction into machine words.
 * out_words must have capacity for at least 3 words.
 * Returns the number of words encoded (>=1). */
int encode_instruction(const ParsedLine *pl, CPUState *cpu, uint16_t out_words[3]);

#endif /* INSTRUCTIONS_H */

