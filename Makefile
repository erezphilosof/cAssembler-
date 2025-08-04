CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = main.c parser.c macro.c symbol_table.c instructions.c output.c utils.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) assembler

.PHONY: assembler clean
