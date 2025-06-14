CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g # Added warning flags and debug info
LDFLAGS =

# Automatically find all .c files in src/
SOURCES = $(wildcard src/*.c)

# Generate object file names from source file names
OBJECTS = $(SOURCES:.c=.o)

EXECUTABLE = bin/Xshell

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p bin # Ensure bin directory exists
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

# Generic rule to compile .c files in src/ to .o files in src/
src/%.o: src/%.c include/%.h # Depend on corresponding header if it exists (optional)
	$(CC) $(CFLAGS) -c $< -o $@

# If you have a main xsh.h that many things depend on:
$(OBJECTS): include/xsh.h include/input.h include/builtins.h include/execute.h include/network.h include/history.h include/utils.h

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) # Remove object files as well

.PHONY: all clean
