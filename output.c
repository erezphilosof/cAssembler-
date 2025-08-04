#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "utils.h"  // ל-format של שורות, convert_to_base4 וכד'

bool write_object_file(const char *filename,
                       const uint16_t *instructions,
                       int instruction_count,
                       const uint16_t *data,
                       int data_count)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ob"); return false; }

    // שורה ראשונה: מספר הוראות ומספר מילים בקובץ נתונים
    fprintf(f, "%d %d\n", instruction_count, data_count);

    // הוראות מקודדות
    for (int i = 0; i < instruction_count; i++) {
        char buf[32];
        convert_to_base4(instructions[i], buf);
        fprintf(f, "%s\n", buf);
    }
    // ואחריהן קטע הנתונים
    for (int i = 0; i < data_count; i++) {
        char buf[32];
        convert_to_base4(data[i], buf);
        fprintf(f, "%s\n", buf);
    }
    fclose(f);
    return true;
}

bool write_entries_file(const char *filename,
                        const Symbol *symtab)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ent"); return false; }
    // לכל סימבול שסומן .entry בטבלה המקושרת
    for (const Symbol *s = symtab; s; s = s->next) {
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
                          const Symbol *symtab)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ext"); return false; }
    // בדיקה על כל הסימבולים והדפסת אלו המסומנים כ-external
    for (const Symbol *s = symtab; s; s = s->next) {
        if (s->type == SYM_EXTERNAL) {
            char buf[32];
            convert_to_base4(s->address, buf);
            fprintf(f, "%s %s\n", s->name, buf);
        }
    }
    fclose(f);
    return true;
}

