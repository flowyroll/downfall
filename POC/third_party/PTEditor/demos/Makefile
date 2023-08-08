SRC=$(wildcard *.c)
BIN=$(SRC:.c=)

all: $(BIN)
% : %.c
	gcc $< -o $@
	
clean:
	rm -f $(BIN) *.o
