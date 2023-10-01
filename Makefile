CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Werror -ggdb
CLIBS = -lm -lfftw3

SRC = src
OBJ = obj
DEPDIR = .deps

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
DEPS = $(patsubst $(SRC)/%.c, $(DEPDIR)/%.d, $(SRCS))
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

BINDIR = bin
BIN = $(BINDIR)/fafft

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CLIBS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR) $(DEPDIR) $(OBJ)

$(OBJ) $(DEPDIR):
	@mkdir -p $@

# Include the '.d' dependency files
-include $(DEPS)

# I wasn't smart enough to do this myself, so chatGPT helped with this part.
# Rule to generate '.d' dependency files
$(DEPDIR)/%.d: $(SRC)/%.c | $(DEPDIR)
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

