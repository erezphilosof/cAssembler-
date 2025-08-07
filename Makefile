CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc -I.

SRCS = main.c parser.c first_pass.c second_pass.c macro.c symbol_table.c symbols.c instructions.c output.c utils.c registers.c data_segment.c src/error.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

TEST_SRCS = tests/test_reserved_labels.c utils.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

TEST_EXT_SRCS = tests/test_external_entry.c second_pass.c symbol_table.c src/error.c
TEST_EXT_OBJS = $(TEST_EXT_SRCS:.c=.o)

test_reserved_labels: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@

test_external_entry: $(TEST_EXT_OBJS)
	$(CC) $(CFLAGS) $(TEST_EXT_OBJS) -o $@

test: test_reserved_labels test_external_entry
	./test_reserved_labels
	./test_external_entry

clean:
	rm -f $(OBJS) assembler $(TEST_OBJS) $(TEST_EXT_OBJS) test_reserved_labels test_external_entry

.PHONY: assembler clean test test_reserved_labels test_external_entry
