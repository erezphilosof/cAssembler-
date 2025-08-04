#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include <stdbool.h>
#include "parser.h"
#include "instructions.h"

bool second_pass(ParsedLine *lines, int line_count, CPUState *cpu);

#endif /* SECOND_PASS_H */
