CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc -I.

SRCS = main.c parser.c first_pass.c second_pass.c macro.c symbol_table.c symbols.c instructions.c output.c utils.c registers.c data_segment.c src/error.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

TEST_SRCS = tests/test_reserved_labels.c utils.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

test_reserved_labels: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@

test: test_reserved_labels
	./test_reserved_labels

clean:
	rm -f $(OBJS) assembler $(TEST_OBJS) test_reserved_labels

.PHONY: assembler clean test test_reserved_labels
