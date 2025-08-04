#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "utils.h"  // ל-format של שורות, convert_to_base4 וכד'

bool write_object_file(const char *filename,
                       const uint16_t *memory,
                       int instruction_count,
                       int data_count)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ob"); return false; }

    // שורה ראשונה: מספר הוראות ומספר מילים בקובץ נתונים
    fprintf(f, "%d %d\n", instruction_count, data_count);

    // הבא – נניח קידוד ה-machine words בבסיס 4
    for (int i = 0; i < instruction_count + data_count; i++) {
        char buf[32];
        convert_to_base4(memory[i], buf);       // מ-utils
        fprintf(f, "%s\n", buf);
    }
    fclose(f);
    return true;
}

bool write_entries_file(const char *filename,
                        const SymbolTable *symtab)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ent"); return false; }
    const Symbol *cur = symtab ? symtab->head : NULL;
    while (cur) {
        if (cur->type == SYM_ENTRY)
            fprintf(f, "%s %04d\n", cur->name, cur->address);
        cur = cur->next;
    }
    fclose(f);
    return true;
}

bool write_externals_file(const char *filename,
                          const SymbolTable *symtab)
{
    FILE *f = fopen(filename, "w");
    if (!f) { perror("open .ext"); return false; }
    const Symbol *cur = symtab ? symtab->head : NULL;
    while (cur) {
        if (cur->type == SYM_EXTERNAL)
            fprintf(f, "%s %04d\n", cur->name, cur->address);
        cur = cur->next;
    }
    fclose(f);
    return true;
}

