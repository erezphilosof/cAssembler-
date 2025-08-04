CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = main.c parser.c symbol_table.c output.c utils.c error.c
OBJS = $(SRCS:.c=.o)

assembler: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) assembler

.PHONY: assembler clean
