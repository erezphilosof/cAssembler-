#include <strings.h>

#include "second_pass.h"
#include "error.h"

/* Second pass: execute each instruction in order (fills cpu.memory) */
bool second_pass(ParsedLine *lines, int line_count, CPUState *cpu) {
    for (int i = 0; i < line_count; i++) {
        ParsedLine *pl = &lines[i];
        if (pl->type != STMT_INSTRUCTION) continue;
        /* Dispatch based on opcode string */
        if      (strcasecmp(pl->opcode,"MOV")==0) exec_mov(pl,cpu);
        else if (strcasecmp(pl->opcode,"CMP")==0) exec_cmp(pl,cpu);
        else if (strcasecmp(pl->opcode,"ADD")==0) exec_add(pl,cpu);
        else if (strcasecmp(pl->opcode,"SUB")==0) exec_sub(pl,cpu);
        else if (strcasecmp(pl->opcode,"LEA")==0) exec_lea(pl,cpu);
        else if (strcasecmp(pl->opcode,"CLR")==0) exec_clr(pl,cpu);
        else if (strcasecmp(pl->opcode,"NOT")==0) exec_not(pl,cpu);
        else if (strcasecmp(pl->opcode,"INC")==0) exec_inc(pl,cpu);
        else if (strcasecmp(pl->opcode,"DEC")==0) exec_dec(pl,cpu);
        else if (strcasecmp(pl->opcode,"JMP")==0) exec_jmp(pl,cpu);
        else if (strcasecmp(pl->opcode,"BNE")==0) exec_bne(pl,cpu);
        else if (strcasecmp(pl->opcode,"JSR")==0) exec_jsr(pl,cpu);
        else if (strcasecmp(pl->opcode,"RED")==0) exec_red(pl,cpu);
        else if (strcasecmp(pl->opcode,"PRN")==0) exec_prn(pl,cpu);
        else if (strcasecmp(pl->opcode,"STOP")==0) exec_stop(pl,cpu);
        else {
            print_error("Unrecognized opcode in second pass");
            return false;
        }
        cpu->PC++;  /* advance to next memory cell */
    }
    return (get_error_count()==0);
}

