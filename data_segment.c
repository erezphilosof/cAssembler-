#include "data_segment.h"
#include <stdlib.h>

void init_data_segment(DataSegment *ds) {
    ds->words = NULL;
    ds->count = 0;
    ds->capacity = 0;
}

void free_data_segment(DataSegment *ds) {
    free(ds->words);
    ds->words = NULL;
    ds->count = 0;
    ds->capacity = 0;
}

bool append_data_word(DataSegment *ds, uint16_t value) {
    if (ds->count == ds->capacity) {
        int newcap = ds->capacity ? ds->capacity * 2 : 64;
        uint16_t *tmp = realloc(ds->words, newcap * sizeof(uint16_t));
        if (!tmp) return false;
        ds->words = tmp;
        ds->capacity = newcap;
    }
    ds->words[ds->count++] = value;
    return true;
}
