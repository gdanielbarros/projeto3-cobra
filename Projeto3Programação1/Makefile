# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -std=c99 -Iexternal -D_DEFAULT_SOURCE

# Linker Flags
LDFLAGS = -lm -lraylib

# Source Files
SRC = main.c

# Object Files
OBJ = $(SRC:.c=.o)

# Executable
EXE = game

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(EXE)