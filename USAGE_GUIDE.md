# XShell Usage Guide - Windows

## How to Build XShell

### Method 1: Simple Build (Basic Features)
```bat
compile.bat
```
This builds XShell with basic autocompletion support.

### Method 2: Full Build (All Features)
1. Install dependencies:
   - Lua development libraries
   - JSON-C library
2. Run: `build_windows.bat`

## Using Autocompletion

### Basic Autocompletion (Always Available)
1. Open a file in XCodex editor: `xcodex filename.c`
2. Start typing code
3. Press **Ctrl+Space** to trigger completion manually
4. Use **Tab** to accept a completion
5. Use **Arrow Keys** (Up/Down) to navigate completion popup
6. Press **Escape** to close completion popup

### Key Bindings for Completion:
- `Ctrl+Space`: Trigger completion
- `Tab`: Accept selected completion
- `Up Arrow`: Previous completion item
- `Down Arrow`: Next completion item
- `Escape`: Hide completion popup

## Using Plugins (Requires Lua)

### Auto-Loading Plugins
Plugins are automatically loaded from these directories:
- `plugins/` (local plugins)
- `~/.xshell/plugins/` (user plugins)
- `/usr/share/xshell/plugins/` (system plugins - Linux)
- `%APPDATA%/xshell/plugins/` (system plugins - Windows)

### Built-in Plugins
1. **Auto Pairs**: Automatically closes brackets, quotes, etc.
2. **Line Counter**: Counts lines of code

### Creating Custom Plugins
Create `.lua` files in the `plugins/` directory:

```lua
-- plugins/my_plugin.lua
function on_key_press(key)
    if key == 's' and editor.mode == 'command' then
        editor.save_file()
        return true  -- consumed
    end
    return false  -- not consumed
end

function on_file_open(filename)
    print("Opened: " .. filename)
end

-- Register plugin
xshell.register_plugin({
    name = "My Plugin",
    version = "1.0",
    on_key_press = on_key_press,
    on_file_open = on_file_open
})
```

## Using LSP (Language Server Protocol)

### Supported Language Servers
- **clangd**: C/C++ (install LLVM)
- **rust-analyzer**: Rust
- **pylsp**: Python (install python-lsp-server)
- **typescript-language-server**: JavaScript/TypeScript
- **gopls**: Go

### Installing Language Servers

#### Windows:
```bat
# C/C++ (clangd)
# Download LLVM from https://llvm.org/builds/

# Python
pip install python-lsp-server

# JavaScript/TypeScript
npm install -g typescript-language-server

# Go
go install golang.org/x/tools/gopls@latest

# Rust
rustup component add rust-analyzer
```

### Using LSP Features
1. Install a language server (see above)
2. Open a source file: `xcodex main.c`
3. LSP will auto-start for recognized file types
4. Features available:
   - **Intelligent completion**: Better than basic completion
   - **Hover information**: Hover over symbols
   - **Go to definition**: Jump to symbol definitions
   - **Find references**: Find all uses of a symbol
   - **Diagnostics**: Real-time error checking

### LSP Key Bindings:
- `Ctrl+Space`: Trigger LSP completion
- `Ctrl+H`: Show hover information
- `Ctrl+G`: Go to definition
- `Ctrl+R`: Find references
- `F2`: Rename symbol

## Editor Modes and Commands

### XCodex Modal Editor
XShell includes a modal text editor similar to Vi/Vim:

#### Normal Mode (Default):
- `i`: Enter insert mode
- `a`: Enter insert mode after cursor
- `o`: New line and enter insert mode
- `h,j,k,l`: Move cursor (left, down, up, right)
- `w,b`: Move by words
- `x`: Delete character
- `dd`: Delete line
- `yy`: Copy line
- `p`: Paste
- `/`: Search
- `:`: Command mode

#### Insert Mode:
- Type normally
- `Escape`: Return to normal mode
- `Ctrl+Space`: Trigger completion
- `Tab`: Accept completion or insert tab

#### Command Mode:
- `:w`: Save file
- `:q`: Quit
- `:wq`: Save and quit
- `:help`: Show help

## Troubleshooting

### Completion Not Working:
1. Check if XShell was built with `XCODEX_ENABLE_COMPLETION` flag
2. Try manual trigger with `Ctrl+Space`
3. Check file type is supported

### Plugins Not Loading:
1. Ensure Lua support is enabled (`XCODEX_ENABLE_LUA`)
2. Check plugin directory exists
3. Verify `.lua` syntax is correct

### LSP Not Working:
1. Ensure LSP support is enabled (`XCODEX_ENABLE_LSP`)
2. Check language server is installed and in PATH
3. Check file extension matches server configuration
4. Look for LSP server output/errors

### Building Issues:
1. Ensure GCC is installed and in PATH
2. For full features, install Lua and JSON-C development libraries
3. Use simple `compile.bat` for basic functionality

## Example Usage Session

```bat
# Build XShell
compile.bat

# Start XShell
bin\Xshell.exe

# In XShell, edit a C file
xcodex test.c

# In editor:
# 1. Press 'i' to enter insert mode
# 2. Type: #include <std
# 3. Press Ctrl+Space for completion
# 4. Select stdio.h with Tab
# 5. Continue coding with auto-completion
```

This gives you a powerful terminal-based development environment with intelligent code completion!
