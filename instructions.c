#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.h"

/* Resolve operand string to a numeric value */
int resolve_operand(const char *operand, CPUState *cpu) {
    if (operand[0] == '#') {
        /* immediate */
        return atoi(operand + 1);
    }
    if (is_register(operand)) {
        /* register */
        int r = reg_number(operand);
        if (r < 0 || r > 7) {
            print_error("Invalid register");
            return 0;
        }
        return cpu->regs[r];
    }
    /* label / memory direct */
    Symbol *sym = lookup_symbol(cpu->symtab, operand);
    if (!sym) {
        print_error("Unknown label");
        return 0;
    }
    if (sym->is_external)
        mark_external(cpu->symtab, sym->name, cpu->PC);
    return cpu->memory[sym->address];
}

/* Write a value back to operand location */
void set_operand(const char *operand, CPUState *cpu, uint16_t value) {
    if (operand[0] == '#') {
        print_error("Cannot STORE to immediate");
        return;
    }
    if (is_register(operand)) {
        int r = reg_number(operand);
        cpu->regs[r] = value;
        return;
    }
    /* direct memory */
    Symbol *sym = lookup_symbol(cpu->symtab, operand);
    if (!sym) {
        print_error("Unknown label for STORE");
        return;
    }
    if (sym->is_external)
        mark_external(cpu->symtab, sym->name, cpu->PC);
    cpu->memory[sym->address] = value;
}

/* Get operand value */
uint16_t get_operand(const char *operand, CPUState *cpu) {
    return (uint16_t)resolve_operand(operand, cpu);
}

/* Set zero/sign flags */
void update_flags(CPUState *cpu, uint16_t result) {
    cpu->zero_flag = (result == 0);
    cpu->sign_flag = ((result & 0x8000) != 0);
}

/* MOV src,dst */
void exec_mov(const ParsedLine *pl, CPUState *cpu) {
    char src[64], dst[64];
    sscanf(pl->operands_raw, "%63[^,],%63s", src, dst);
    uint16_t v = get_operand(src, cpu);
    set_operand(dst, cpu, v);
}

/* CMP op1,op2 */
void exec_cmp(const ParsedLine *pl, CPUState *cpu) {
    char a[64], b[64];
    sscanf(pl->operands_raw, "%63[^,],%63s", a, b);
    uint16_t r = get_operand(a, cpu) - get_operand(b, cpu);
    update_flags(cpu, r);
}

/* ADD op1,op2 */
void exec_add(const ParsedLine *pl, CPUState *cpu) {
    char a[64], b[64];
    sscanf(pl->operands_raw, "%63[^,],%63s", a, b);
    uint16_t r = get_operand(a, cpu) + get_operand(b, cpu);
    update_flags(cpu, r);
    set_operand(b, cpu, r);
}

/* SUB op1,op2 */
void exec_sub(const ParsedLine *pl, CPUState *cpu) {
    char a[64], b[64];
    sscanf(pl->operands_raw, "%63[^,],%63s", a, b);
    uint16_t r = get_operand(a, cpu) - get_operand(b, cpu);
    update_flags(cpu, r);
    set_operand(b, cpu, r);
}

/* LEA src,dst */
void exec_lea(const ParsedLine *pl, CPUState *cpu) {
    char src[64], dst[64];
    sscanf(pl->operands_raw, "%63[^,],%63s", src, dst);
    /* src must be label */
    Symbol *sym = lookup_symbol(cpu->symtab, src);
    if (!sym) { print_error("Unknown label for LEA"); return; }
    if (sym->is_external)
        mark_external(cpu->symtab, sym->name, cpu->PC);
    set_operand(dst, cpu, sym->address);
}

/* CLR op */
void exec_clr(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    set_operand(op, cpu, 0);
}

/* NOT op */
void exec_not(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    uint16_t v = ~get_operand(op, cpu);
    set_operand(op, cpu, v);
}

/* INC op */
void exec_inc(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    uint16_t v = get_operand(op, cpu) + 1;
    set_operand(op, cpu, v);
}

/* DEC op */
void exec_dec(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    uint16_t v = get_operand(op, cpu) - 1;
    set_operand(op, cpu, v);
}

/* JMP label */
void exec_jmp(const ParsedLine *pl, CPUState *cpu) {
    char lbl[64];
    sscanf(pl->operands_raw, "%63s", lbl);
    Symbol *sym = lookup_symbol(cpu->symtab, lbl);
    if (!sym) { print_error("Unknown label for JMP"); return; }
    if (sym->is_external)
        mark_external(cpu->symtab, sym->name, cpu->PC);
    cpu->PC = sym->address;
}

/* BNE label */
void exec_bne(const ParsedLine *pl, CPUState *cpu) {
    char lbl[64];
    sscanf(pl->operands_raw, "%63s", lbl);
    if (!cpu->zero_flag) {
        Symbol *sym = lookup_symbol(cpu->symtab, lbl);
        if (!sym) { print_error("Unknown label for BNE"); return; }
        if (sym->is_external)
            mark_external(cpu->symtab, sym->name, cpu->PC);
        cpu->PC = sym->address;
    }
}

/* JSR label */
void exec_jsr(const ParsedLine *pl, CPUState *cpu) {
    char lbl[64];
    sscanf(pl->operands_raw, "%63s", lbl);
    Symbol *sym = lookup_symbol(cpu->symtab, lbl);
    if (!sym) { print_error("Unknown label for JSR"); return; }
    if (sym->is_external)
        mark_external(cpu->symtab, sym->name, cpu->PC);
    /* store return address in R7 */
    cpu->regs[7] = cpu->PC;
    cpu->PC = sym->address;
}

/* RED op */
void exec_red(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    int v;
    if (scanf("%d", &v)!=1) print_error("RED: failed to read");
    set_operand(op, cpu, (uint16_t)v);
}

/* PRN op */
void exec_prn(const ParsedLine *pl, CPUState *cpu) {
    char op[64];
    sscanf(pl->operands_raw, "%63s", op);
    printf("%d\n", get_operand(op, cpu));
}

/* STOP */
void exec_stop(const ParsedLine *pl, CPUState *cpu) {
    (void)pl; (void)cpu;
    exit(0);
}

