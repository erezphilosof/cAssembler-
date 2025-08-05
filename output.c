#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "utils.h"  // ל-format של שורות, convert_to_base4 וכד'

bool write_object_file(const char *filename,
                       const uint16_t *instructions,
                       int instruction_count,
                       const uint16_t *data,
                       int data_count,
                       int base_address)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ob"); return false; }

    // שורה ראשונה: מספר הוראות ומספר מילים בקובץ נתונים
    fprintf(f, "%d %d\n", instruction_count, data_count);

    // הוראות מקודדות עם כתובות
    int address = base_address;
    for (int i = 0; i < instruction_count; i++, address++) {
        char addr_buf[32], word_buf[32];
        convert_to_base4((uint16_t)address, addr_buf);
        convert_to_base4(instructions[i], word_buf);
        fprintf(f, "%s %s\n", addr_buf, word_buf);
    }
    // ואחריהן קטע הנתונים
    for (int i = 0; i < data_count; i++, address++) {
        char addr_buf[32], word_buf[32];
        convert_to_base4((uint16_t)address, addr_buf);
        convert_to_base4(data[i], word_buf);
        fprintf(f, "%s %s\n", addr_buf, word_buf);
    }
    fclose(f);
    return true;
}

bool write_entries_file(const char *filename,
                        const Symbol *symtab)
{
    /* first scan to see if there are any entry symbols */
    const Symbol *s = symtab;
    while (s && s->type != SYM_ENTRY)
        s = s->next;
    if (!s)
        return false; /* no entries: don't create the file */

    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ent"); return false; }

    /* s already points to the first entry symbol */
    for (; s; s = s->next) {
        if (s->type == SYM_ENTRY) {
            char buf[32];
            convert_to_base4(s->address, buf);
            fprintf(f, "%s %s\n", s->name, buf);
        }
    }
    fclose(f);
    return true;
}

bool write_externals_file(const char *filename,
                          const ExternalUse *uses)
{
    /* If there are no recorded external usages, nothing to do */
    if (!uses)
        return false;

    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ext"); return false; }
    for (const ExternalUse *u = uses; u; u = u->next) {
        char buf[32];
        convert_to_base4(u->address, buf);
        fprintf(f, "%s %s\n", u->name, buf);
    }
    fclose(f);
    return true;
}

