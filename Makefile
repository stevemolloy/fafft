CC = clang
CFLAGS = -Wall -Wpedantic -Werror -ggdb
CLIBS = -lm -lfftw3
DEPS = string_array.h
OBJ = string_array.o fafft.o

all: fafft

fafft: $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(CLIBS) $^

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm fafft *.o
