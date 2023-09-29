CC = clang
CFLAGS = -Wall -Wpedantic -Werror -ggdb
CLIBS = 

all: fafft

fafft: main.c
	$(CC) $(CFLAGS) $(CLIBS) $^ -o $@

clean:
	rm fafft
