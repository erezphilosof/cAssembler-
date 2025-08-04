#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "parser.h"       /* ParsedLine */
#include "symbol_table.h" /* lookup_symbol */
#include "utils.h"        /* is_register, reg_number */
#include "error.h"        /* print_error */

/* CPU state (registers, flags, memory pointer, program counter) */
typedef struct {
    uint16_t *memory;     /* pointer to assembled memory image */
    uint16_t  PC;         /* program counter */
    uint16_t  regs[8];    /* R0..R7 */
    bool      zero_flag;
    bool      sign_flag;
} CPUState;

/* Execute one instruction at pl->line_number */
void exec_mov(const ParsedLine *pl, CPUState *cpu);
void exec_cmp(const ParsedLine *pl, CPUState *cpu);
void exec_add(const ParsedLine *pl, CPUState *cpu);
void exec_sub(const ParsedLine *pl, CPUState *cpu);
void exec_lea(const ParsedLine *pl, CPUState *cpu);
void exec_clr(const ParsedLine *pl, CPUState *cpu);
void exec_not(const ParsedLine *pl, CPUState *cpu);
void exec_inc(const ParsedLine *pl, CPUState *cpu);
void exec_dec(const ParsedLine *pl, CPUState *cpu);
void exec_jmp(const ParsedLine *pl, CPUState *cpu);
void exec_bne(const ParsedLine *pl, CPUState *cpu);
void exec_jsr(const ParsedLine *pl, CPUState *cpu);
void exec_red(const ParsedLine *pl, CPUState *cpu);
void exec_prn(const ParsedLine *pl, CPUState *cpu);
void exec_stop(const ParsedLine *pl, CPUState *cpu);

/* Helper routines */
int   resolve_operand(const char *operand, CPUState *cpu);
void  set_operand(const char *operand, CPUState *cpu, uint16_t value);
uint16_t get_operand(const char *operand, CPUState *cpu);
void  update_flags(CPUState *cpu, uint16_t result);

#endif /* INSTRUCTIONS_H */

