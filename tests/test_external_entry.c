#include <assert.h>
#include <stdint.h>
#include "second_pass.h"
#include "symbol_table.h"
#include "error.h"

/* Stub for encode_instruction to satisfy linker */
int encode_instruction(const ParsedLine *pl, CPUState *cpu, uint16_t out_words[3]) {
    (void)pl; (void)cpu; (void)out_words;
    return 0;
}

int main(void) {
    Symbol *symtab = NULL;
    add_symbol(&symtab, "EXTSYM", 0, SYM_EXTERNAL);

    CPUState cpu = {0};
    uint16_t memory[1] = {0};
    cpu.memory = memory;
    cpu.PC = 0;
    cpu.symtab = symtab;
    cpu.ext_uses = NULL;

    ParsedLine line = {0};
    line.type = STMT_DIRECTIVE;
    line.dir_type = DIR_ENTRY;
    line.directive_args = "EXTSYM";

    ParsedLine lines[] = { line };

    bool ok = second_pass(lines, 1, &cpu);
    assert(!ok);
    assert(get_error_count() == 2);
    return 0;
}
