CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc

SRCS = main.c parser.c macro.c symbol_table.c symbols.c instructions.c output.c utils.c registers.c src/error.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) assembler

.PHONY: assembler clean
