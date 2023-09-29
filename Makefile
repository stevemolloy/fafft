CC = clang
CFLAGS = -Wall -Wpedantic -Werror -ggdb
CLIBS = -lm -lfftw3

all: fafft

fafft: main.c
	$(CC) $(CFLAGS) $(CLIBS) string_array.c $^ -o $@

clean:
	rm fafft
