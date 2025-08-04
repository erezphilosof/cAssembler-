CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc

SRCS = main.c parser.c first_pass.c second_pass.c macro.c symbol_table.c symbols.c instructions.c output.c utils.c registers.c data_segment.c src/error.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) assembler

.PHONY: assembler clean
