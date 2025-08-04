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

/* Writes all labels שסומנו כ-entry ל-.ent */
bool write_entries_file(const char *filename,
                        const Symbol *symtab);

/* כותב את כל השימושים בסימבולים חיצוניים ל-.ext */
bool write_externals_file(const char *filename,
                          const ExternalUse *uses);

#endif /* OUTPUT_H */

