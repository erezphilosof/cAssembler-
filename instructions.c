#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "instructions.h"

/* Opcodes enumeration for encoding */
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
    else if (strcasecmp(opc, "STOP") == 0) return 14;
    return -1;
}

enum { AM_IMMEDIATE = 0, AM_DIRECT = 1, AM_REGISTER = 2 };

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
        *extra_out = (uint16_t)atoi(op + 1);
        return 1; /* needs extra word */
    }
    if (is_register(op)) {
        *mode_out = AM_REGISTER;
        *reg_out = reg_number(op);
        *extra_out = 0;
        return 0;
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
    } else if (opc == 14) {
        /* STOP has no operands */
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
            if (sym && sym->type == SYM_EXTERNAL)
                add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
            out_words[count++] = extra;
        }
    }
    if (has_dst) {
        int mode, reg; uint16_t extra; Symbol *sym = NULL;
        int needs = parse_operand(dst, cpu, &mode, &reg, &extra, &sym);
        word0 |= (uint16_t)(mode & 0x7) << 3;
        word0 |= (uint16_t)(reg  & 0x7);
        if (needs) {
            if (sym && sym->type == SYM_EXTERNAL)
                add_external_use(&cpu->ext_uses, sym->name, cpu->PC + count);
            out_words[count++] = extra;
        }
    }

    out_words[0] = word0;
    return count;
}

