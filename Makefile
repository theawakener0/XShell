# Makefile for XShell v0.3.1

# Compiler and Flags
CC = gcc
# Add -std=c11 for C11 standard features and cross-platform compatibility
CFLAGS = -Iinclude -Wall -Wextra -g -std=c11
LDFLAGS =

# Feature flags - uncomment to enable features
# CFLAGS += -DXCODEX_ENABLE_LUA
# CFLAGS += -DXCODEX_ENABLE_COMPLETION

# Lua support (optional)
ifeq ($(shell pkg-config --exists lua5.4 && echo yes),yes)
    LUA_CFLAGS = $(shell pkg-config --cflags lua5.4)
    LUA_LIBS = $(shell pkg-config --libs lua5.4)
    CFLAGS += $(LUA_CFLAGS) -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION
    LDFLAGS += $(LUA_LIBS)
else ifeq ($(shell pkg-config --exists lua5.3 && echo yes),yes)
    LUA_CFLAGS = $(shell pkg-config --cflags lua5.3)
    LUA_LIBS = $(shell pkg-config --libs lua5.3)
    CFLAGS += $(LUA_CFLAGS) -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION
    LDFLAGS += $(LUA_LIBS)
else ifeq ($(shell pkg-config --exists lua && echo yes),yes)
    LUA_CFLAGS = $(shell pkg-config --cflags lua)
    LUA_LIBS = $(shell pkg-config --libs lua)
    CFLAGS += $(LUA_CFLAGS) -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION
    LDFLAGS += $(LUA_LIBS)
else
    # Fallback: try common Lua locations
    ifneq ($(wildcard /usr/include/lua5.4),)
        CFLAGS += -I/usr/include/lua5.4 -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION
        LDFLAGS += -llua5.4
    else ifneq ($(wildcard /usr/include/lua),)
        CFLAGS += -I/usr/include/lua -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION
        LDFLAGS += -llua
    endif
endif

# JSON-C support for LSP (optional)
ifeq ($(shell pkg-config --exists json-c && echo yes),yes)
    JSON_CFLAGS = $(shell pkg-config --cflags json-c)
    JSON_LIBS = $(shell pkg-config --libs json-c)
    CFLAGS += $(JSON_CFLAGS) -DXCODEX_ENABLE_LSP
    LDFLAGS += $(JSON_LIBS)
else
    # Fallback: try common json-c locations
    ifneq ($(wildcard /usr/include/json-c),)
        CFLAGS += -I/usr/include/json-c -DXCODEX_ENABLE_LSP
        LDFLAGS += -ljson-c
    endif
endif

# Project Structure
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Automatically find all .c files and generate corresponding .o and .d file names
# Note: src/xcodex.c now contains full cross-platform support (Windows, Linux, macOS)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d) # Dependency files for header tracking

# --- OS-Specific Configuration ---
# Default to Unix-like settings
EXECUTABLE = $(BIN_DIR)/Xshell
RM = rm -f
MKDIR_P = mkdir -p

# Check if the OS is Windows
ifeq ($(OS),Windows_NT)
    EXECUTABLE = $(BIN_DIR)/Xshell.exe
    # Link against Winsock2 and IP Helper libraries on Windows
    LDFLAGS += -lws2_32 -liphlpapi
    RM = del /Q /F
	# Windows has no `mkdir -p`, so we create the directory if it doesn't exist.
    MKDIR_P = if not exist $(@D) mkdir $(@D)
else
    # Link against the math library on Unix-like systems (often needed)
    LDFLAGS += -lm
endif

# --- Build Rules ---

# Default target: build the executable
all: $(EXECUTABLE)

# Rule to link the final executable
$(EXECUTABLE): $(OBJECTS)
	@$(MKDIR_P)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "XShell built successfully: $@"

# Rule to compile .c files into .o files in the OBJ_DIR
# The -MMD and -MP flags generate dependency files (.d) automatically
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# --- Housekeeping Rules ---

# Clean the project
clean:
	-$(RM) $(EXECUTABLE)
	-$(RM) $(OBJECTS)
	-$(RM) $(DEPS)
	-@if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR) 2>nul || rm -rf $(OBJ_DIR)

# Rebuild the project
re: clean all

# Run the shell
run: all
	@echo "Starting XShell..."
	# Use backslashes for the path on Windows
	ifeq ($(OS),Windows_NT)
		.\$(EXECUTABLE)
	else
		./$(EXECUTABLE)
	endif

# Test XCodex editor specifically
test-xcodex: all
	@echo "Testing XCodex cross-platform editor..."
	@echo "Creating test file..."
	@echo "int main() { return 0; }" > test.c
	ifeq ($(OS),Windows_NT)
		@echo "Running: .\$(EXECUTABLE) and typing 'xcodex test.c'"
		@echo "XCodex should now work on Windows with full cross-platform support!"
	else
		@echo "Running: ./$(EXECUTABLE) and typing 'xcodex test.c'"  
		@echo "XCodex should work seamlessly on Unix-like systems!"
	endif

# Test plugins
test-plugins: all
	@echo "Testing XCodex plugin system..."
	@echo "Plugins available in ./plugins directory:"
	@ls plugins/*.lua 2>/dev/null || dir plugins\*.lua 2>nul || echo "No plugins found"
	@echo "Run XShell and try:"
	@echo "  xcodex test.c"
	@echo "  (in command mode) :plugindir plugins"
	@echo "  (in insert mode) type '(' to test auto-pairs"

# Install Lua development files (helper target)
install-lua-dev:
	@echo "Installing Lua development files..."
	ifeq ($(OS),Windows_NT)
		@echo "On Windows, please install Lua manually or use MSYS2:"
		@echo "  pacman -S mingw-w64-x86_64-lua"
	else ifeq ($(shell uname),Darwin)
		@echo "On macOS, use Homebrew:"
		@echo "  brew install lua"
	else
		@echo "On Ubuntu/Debian:"
		@echo "  sudo apt install liblua5.4-dev"
		@echo "On CentOS/RHEL:"
		@echo "  sudo yum install lua-devel"
		@echo "On Arch Linux:"
		@echo "  sudo pacman -S lua"
	endif

# Check if Lua is available
check-lua:
	@echo "Checking Lua installation..."
	@lua -v 2>/dev/null || echo "Lua not found in PATH"
	@pkg-config --exists lua5.4 && echo "lua5.4 pkg-config found" || echo "lua5.4 pkg-config not found"
	@pkg-config --exists lua5.3 && echo "lua5.3 pkg-config found" || echo "lua5.3 pkg-config not found"
	@pkg-config --exists lua && echo "lua pkg-config found" || echo "lua pkg-config not found"

# Include the generated dependency files. The '-' suppresses errors if they don't exist.
-include $(DEPS)

# Phony targets are not real files
.PHONY: all clean re run test-xcodex test-plugins install-lua-dev check-lua
