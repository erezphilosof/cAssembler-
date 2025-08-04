#ifndef DATA_SEGMENT_H
#define DATA_SEGMENT_H

#include <stdint.h>
#include <stdbool.h>

/* Dynamic array holding assembled data words */
typedef struct {
    uint16_t *words;   /* allocated array */
    int count;         /* how many words used */
    int capacity;      /* allocated capacity */
} DataSegment;

void init_data_segment(DataSegment *ds);
void free_data_segment(DataSegment *ds);
bool append_data_word(DataSegment *ds, uint16_t value);

#endif /* DATA_SEGMENT_H */
