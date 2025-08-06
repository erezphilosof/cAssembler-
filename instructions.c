#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "instructions.h"

/* Opcodes enumeration for encoding
 * MOV=0  CMP=1  ADD=2  SUB=3  LEA=4
 * CLR=5  NOT=6  INC=7  DEC=8
 * JMP=9  BNE=10 JSR=11 RED=12 PRN=13
 * RTS=14 STOP=15
 */
static int opcode_to_num(const char *opc) {
    if      (strcasecmp(opc, "MOV") == 0) return 0;
    else if (strcasecmp(opc, "CMP") == 0) return 1;
    else if (strcasecmp(opc, "ADD") == 0) return 2;
    else if (strcasecmp(opc, "SUB") == 0) return 3;
    else if (strcasecmp(opc, "LEA") == 0) return 4;
    else if (strcasecmp(opc, "CLR") == 0) return 5;
    else if (strcasecmp(opc, "NOT") == 0) return 6;
    else if (strcasecmp(opc, "INC") == 0) return 7;
    else if (strcasecmp(opc, "DEC") == 0) return 8;
    else if (strcasecmp(opc, "JMP") == 0) return 9;
    else if (strcasecmp(opc, "BNE") == 0) return 10;
    else if (strcasecmp(opc, "JSR") == 0) return 11;
    else if (strcasecmp(opc, "RED") == 0) return 12;
    else if (strcasecmp(opc, "PRN") == 0) return 13;
    else if (strcasecmp(opc, "RTS")  == 0) return 14;
    else if (strcasecmp(opc, "STOP") == 0) return 15;
    return -1;
}

enum { AM_IMMEDIATE = 0, AM_DIRECT = 1, AM_REGISTER = 2, AM_MATRIX = 3 };

/* Parse one operand and return addressing mode bits and optional extra word */
static int parse_operand(const char *op,
                         CPUState *cpu,
                         int *mode_out,
                         int *reg_out,
                         uint16_t *extra_out,
                         Symbol **sym_out) {
    if (sym_out) *sym_out = NULL;
    if (!op || op[0] == '\0') {
        *mode_out = 0; *reg_out = 0; *extra_out = 0; return 0;
    }
    if (op[0] == '#') {
        *mode_out = AM_IMMEDIATE;
        *reg_out = 0;

        errno = 0;
        char *endptr;
        long val = strtol(op + 1, &endptr, 10);
        if (errno != 0 || *endptr != '\0' || endptr == op + 1 ||
            val < -32768 || val > 32767) {
            print_error("Invalid number: %s", op + 1);
            *extra_out = 0;
        } else {
            *extra_out = (uint16_t)val;
        }
        return 1; /* needs extra word */
    }
    if (is_register(op)) {
        *mode_out = AM_REGISTER;
        *reg_out = reg_number(op);
        *extra_out = 0;
        return 0;
    }

    /* matrix addressing: <label>[rX][rY] */
    const char *b1 = strchr(op, '[');
    if (b1) {
        const char *b2 = strchr(b1 + 1, ']');
        const char *b3 = b2 ? strchr(b2 + 1, '[') : NULL;
        const char *b4 = b3 ? strchr(b3 + 1, ']') : NULL;
        if (!b2 || !b3 || !b4 || b3 != b2 + 1 || *(b4 + 1) != '\0' || b1 == op) {
            print_error("Invalid matrix operand: %s", op);
            *mode_out = AM_MATRIX;
            *reg_out = 0;
            *extra_out = 0;
            return 0;
        }
        char label[64];
        strncpy(label, op, b1 - op);
        label[b1 - op] = '\0';
        char r1[8];
        char r2[8];
        strncpy(r1, b1 + 1, b2 - b1 - 1);
        r1[b2 - b1 - 1] = '\0';
        strncpy(r2, b3 + 1, b4 - b3 - 1);
        r2[b4 - b3 - 1] = '\0';
        if (!is_register(r1) || !is_register(r2)) {
            print_error("Invalid register in matrix operand");
            *mode_out = AM_MATRIX;
            *reg_out = 0;
            *extra_out = 0;
            return 0;
        }
        int reg1 = reg_number(r1);
        int reg2 = reg_number(r2);
        *mode_out = AM_MATRIX;
        *reg_out = reg1;
        *extra_out = (uint16_t)((reg1 << 3) | reg2);
        Symbol *sym = lookup_symbol(cpu->symtab, label);
        if (sym_out) *sym_out = sym;
        if (!sym) {
            print_error("Unknown label: %s", label);
        }
        return 2; /* label address + registers word */
    }

    /* direct label */
    *mode_out = AM_DIRECT;
    *reg_out = 0;
    Symbol *sym = lookup_symbol(cpu->symtab, op);
    if (sym_out) *sym_out = sym;
    if (!sym) {
        print_error("Unknown label: %s", op);
        *extra_out = 0;
    } else {
        *extra_out = sym->address;
    }
    return 1;
}

/* Encode an instruction into up to 3 words */
int encode_instruction(const ParsedLine *pl, CPUState *cpu, uint16_t out_words[3]) {
    uint16_t word0 = 0;
    int count = 1;

    int opc = opcode_to_num(pl->opcode);
    if (opc < 0) {
        print_error("Unrecognized opcode");
        return 0;
    }
    word0 |= (uint16_t)(opc & 0xF) << 12;

    char src[64] = {0};
    char dst[64] = {0};
    bool has_src = false, has_dst = false;

    /* determine operand forms */
    if (opc <= 4) { /* two-operand instructions */
        sscanf(pl->operands_raw, "%63[^,],%63s", src, dst);
        has_src = has_dst = true;
    } else if (opc == 14 || opc == 15) {
        /* RTS and STOP have no operands */
    } else { /* single operand */
        sscanf(pl->operands_raw, "%63s", dst);
        has_dst = true;
    }

    if (has_src) {
        int mode, reg; uint16_t extra; Symbol *sym = NULL;
        int needs = parse_operand(src, cpu, &mode, &reg, &extra, &sym);
        word0 |= (uint16_t)(mode & 0x7) << 9;
        word0 |= (uint16_t)(reg  & 0x7) << 6;
        if (needs) {
            if (mode == AM_MATRIX) {
                if (sym && sym->type == SYM_EXTERNAL)
                    add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
                out_words[count++] = sym ? sym->address : 0;
                out_words[count++] = extra;
            } else {
                if (sym && sym->type == SYM_EXTERNAL)
                    add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
                out_words[count++] = extra;
            }
        }
    }
    if (has_dst) {
        int mode, reg; uint16_t extra; Symbol *sym = NULL;
        int needs = parse_operand(dst, cpu, &mode, &reg, &extra, &sym);
        word0 |= (uint16_t)(mode & 0x7) << 3;
        word0 |= (uint16_t)(reg  & 0x7);
        if (needs) {
            if (mode == AM_MATRIX) {
                if (sym && sym->type == SYM_EXTERNAL)
                    add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
                out_words[count++] = sym ? sym->address : 0;
                out_words[count++] = extra;
            } else {
                if (sym && sym->type == SYM_EXTERNAL)
                    add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
                out_words[count++] = extra;
            }
        }
    }

    out_words[0] = word0;
    return count;
}

