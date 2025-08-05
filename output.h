#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include "symbol_table.h"

/* Generates the object file (.ob) from memory image */
bool write_object_file(const char *filename,
                       const uint16_t *instructions,
                       int instruction_count,
                       const uint16_t *data,
                       int data_count);

/*
 * Write all symbols marked as .entry to a .ent file. Returns true only if the
 * file was created (i.e., at least one entry symbol exists).
 */
bool write_entries_file(const char *filename,
                        const Symbol *symtab);

/*
 * Write all recorded uses of external symbols to a .ext file. Returns true
 * only if the file was created (i.e., at least one external use exists).
 */
bool write_externals_file(const char *filename,
                          const ExternalUse *uses);

#endif /* OUTPUT_H */

