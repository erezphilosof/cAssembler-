#include <strings.h>

#include "second_pass.h"
#include "error.h"
#include "symbol_table.h"

/* Second pass: encode each instruction into cpu->memory */
bool second_pass(ParsedLine *lines, int line_count, CPUState *cpu) {
    for (int i = 0; i < line_count; i++) {
        ParsedLine *pl = &lines[i];

        /* Handle .entry directives: mark symbol as entry */
        if (pl->type == STMT_DIRECTIVE && pl->dir_type == DIR_ENTRY) {
            if (!update_symbol_type(cpu->symtab, pl->directive_args, SYM_ENTRY)) {
                print_error("Undefined entry label: %s", pl->directive_args);
            }
            continue; /* no machine code emitted */
        }

        if (pl->type != STMT_INSTRUCTION) continue;

        uint16_t words[3];
        int count = encode_instruction(pl, cpu, words);
        for (int w = 0; w < count; w++) {
            cpu->memory[cpu->PC++] = words[w];
        }
    }
    return (get_error_count() == 0);
}

