# XShell v0.3.2 - alpha (Under Testing)

**_Break the terminal. Build anew._**

A revolutionary shell interface with advanced text editing capabilities and AI-powered features, currently in active development as a stepping stone toward v0.4.0.

**Key Features at a Glance:**
- **Modal Text Editor**: Vim-like editing with syntax highlighting for 14+ languages and 6 beautiful themes
- **Plugin System**: Extensible Lua-based plugin architecture with rich API and auto-loading
- **LSP Integration**: Full Language Server Protocol support for intelligent code features
- **Smart Completion**: Intelligent autocompletion with syntax awareness, buffer analysis, and LSP-powered suggestions
- **Cross-Platform**: Native support for Windows, Linux, and macOS with optimized terminal handling
- **Xenomench Commands**: Exclusive tools for networking, encryption, project management, and AI interaction

![Version](https://img.shields.io/badge/version-v0.3.2--alpha-yellow) ![Status](https://img.shields.io/badge/status-under%20testing-orange) ![License](https://img.shields.io/badge/license-MIT-blue) ![Build](https://img.shields.io/badge/build-experimental-yellow)

---

## The Xenomench Philosophy

> "The world is code. I read it, rewrite it, and rebuild it."

XShell is not just another command-line interface. It is a tool of liberation—the first component of the Xenomench project. We reject the constraints of inherited systems and the obedience expected by conventional tools. XShell exists not merely to execute commands, but to challenge how we interact with machines at the most fundamental level.

The Xenomench project aims to deconstruct the barriers between human intent and machine execution. This shell is the entry point to that revolution.

---

## Features

- **Core Command Set** - Full implementation of essential commands:
  - `cd` - Navigate through the system
  - `pwd` - Reveal your current location
  - `ls` - Expose the contents of directories
  - `grep` - Search for patterns within files
  - `echo` - Project your voice to the system
  - `mkdir` - Create new spaces within the structure
  - `touch` - Bring files into existence
  - `cp` - Duplicate resources
  - `mv` - Relocate entities
  - `rm` - Remove what is no longer needed
  - `cat` - Inspect the contents of files
  - `clear` - Reset your view
  - `history` - Show command history
  - `help` - See all available commands
  - `exit` - Terminate the shell session

- **Xenomench Exclusive Commands**:
  - `xmanifesto` - Unveil the core principles of the Xenomench project
  - `xeno` - Establish connection with The Gatekeeper, an AI entity guiding Xenomench development (Network functionality)
  - `xnet` - Perform network operations and diagnostics
  - `xpass` - Manage encrypted credentials
  - `xnote` - Create and manage personal notes
  - `xproj` - Handle project-related tasks and files
  - `xscan` - Scan network ports and services
  - `xcrypt` - Encrypt or decrypt files with a password
  - `xcodex` - A powerful, modal text editor with cross-platform support (Windows, Linux, macOS)
    - **NEW v0.3.2**: Enhanced stability and performance optimizations
    - **NEW v0.3.2**: Improved error handling and cross-platform consistency
    - **Features**: Vim-like modal editing, syntax highlighting for 14+ languages, 6 beautiful themes
    - **Plugin System**: Extensible Lua-based plugin architecture with rich API
    - **Autocompletion**: Intelligent completion with syntax awareness and buffer analysis
    - **Modes**: Normal, Insert, Visual, Command modes for efficient text editing
    - **Themes**: Default, Solarized Dark, Monokai, Dracula, Gruvbox, One Dark

- **Modular Design**: Source code organized into modules for better maintainability and scalability (`src/`, `include/`).
- **Makefile Build System**: Simplified compilation process.
- **Clean, minimal interface** with enhanced prompt displays.
- **Command history** for faster workflow.
- **Cross-platform compatibility** (Windows, Linux, macOS).

---

## Demo

A quick look at XShell in action:

![XShell Demo](demo/XShell.jpg)

[Watch the full video demo (MP4)](demo/XShell_Demo.mp4)

---

## Installation

⚠️ **Testing Notice**: v0.3.2 is currently under active testing. For stable usage, consider downloading v0.3.1 from releases. v0.3.2 represents ongoing improvements toward v0.4.0.

### Prerequisites

- C Compiler (GCC or Clang recommended)
- Make (for automated building)
- Git (for version control)
- Network libraries (Windows: ws2_32, iphlpapi)

**New in v0.3.2**: Building upon v0.3.1's solid foundation, XCodex now features enhanced stability and performance optimizations with no additional dependencies required!

### Optional Dependencies for Advanced Features

#### For Full Plugin Support (Lua Integration):
- **Lua development libraries**
- **JSON-C library**

#### For Language Server Protocol (LSP) Support:
Install language servers for enhanced code intelligence:

**Windows:**
```bat
# C/C++ Language Server (clangd)
# Download LLVM from https://llvm.org/builds/

# Python Language Server
pip install python-lsp-server

# JavaScript/TypeScript Language Server
npm install -g typescript-language-server

# Go Language Server
go install golang.org/x/tools/gopls@latest

# Rust Language Server
rustup component add rust-analyzer
```

**Linux/macOS:**
```bash
# C/C++ Language Server (clangd)
# Ubuntu/Debian: sudo apt install clangd
# macOS: brew install llvm

# Python Language Server
pip install python-lsp-server

# JavaScript/TypeScript Language Server
npm install -g typescript-language-server

# Go Language Server
go install golang.org/x/tools/gopls@latest

# Rust Language Server
rustup component add rust-analyzer
```

### Building XShell

#### Windows Build Environment Setup

**Option 1: MinGW-w64 / MSYS2 (Recommended for Windows)**
```bat
REM Install MSYS2 from https://www.msys2.org/
REM After installation, update package database:
pacman -Syu

REM Install build tools:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make

REM Add to PATH: C:\msys64\mingw64\bin
```

**Option 2: Visual Studio Build Tools**
```bat
REM Install Visual Studio Build Tools or Visual Studio Community
REM Open "Developer Command Prompt for VS"
REM Navigate to XShell directory
```

**Option 3: Windows Subsystem for Linux (WSL)**
```bash
# Use WSL with Ubuntu/Debian and follow Linux instructions
sudo apt update && sudo apt install build-essential
```

#### Universal Build Instructions

1. **Get the source code:**
    ```bash
    git clone https://github.com/theawakener0/XShell.git
    cd XShell
    ```

2. **Build the project:**
    
    **Using Make (recommended):**
    - **Linux/macOS:**
      ```bash
      make
      ```
    
    - **Windows (MinGW/MSYS2):**
      ```bash
      # Option 1: Using make (if available)
      make
      
      # Option 2: Using provided batch files
      build_windows.bat
      
      # Option 3: Simple compilation
      compile.bat
      ```
    
    - **Windows (Visual Studio Developer Command Prompt):**
      ```cmd
      # Using nmake (if Makefile.win exists)
      nmake -f Makefile.win
      
      # Or use the batch file
      build_windows.bat
      ```

    **Manual compilation:**
    - **Linux/macOS:**
      ```bash
      mkdir -p bin
      gcc -Iinclude -DXCODEX_ENABLE_COMPLETION -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_LSP src/*.c -o bin/Xshell -lm
      ```
    
    - **Windows (MinGW/GCC):**
      ```bash
      mkdir bin
      gcc -Iinclude -DXCODEX_ENABLE_COMPLETION -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_LSP src\*.c -o bin\Xshell.exe -lws2_32 -liphlpapi
      ```

3. **Run XShell:**
    - **Linux/macOS:**
      ```bash
      ./bin/Xshell
      ```
    - **Windows:**
      ```bash
      .\bin\Xshell.exe
      ```

4. **Test XCodex cross-platform editor:**
    
    **Linux/macOS:**
    ```bash
    # Test the new cross-platform XCodex editor
    make test-xcodex
    
    # Or manually test with various file types:
    # Run XShell and type: xcodex test.c
    # Run XShell and type: xcodex config.json
    # Run XShell and type: xcodex data.csv
    ```
    
    **Windows:**
    ```bat
    REM Test the cross-platform XCodex editor
    make test-xcodex
    
    REM Or use batch file testing
    build_simple.bat
    
    REM Or manually test with various file types:
    REM Run XShell and type: xcodex test.c
    REM Run XShell and type: xcodex config.json
    REM Run XShell and type: xcodex data.csv
    ```

### Troubleshooting

#### Windows-Specific Build Issues:

**Make command not found:**
```bat
REM Solution 1: Use MSYS2 and install make
pacman -S make

REM Solution 2: Use the provided batch files instead
compile.bat
build_windows.bat

REM Solution 3: Use nmake if using Visual Studio
nmake -f Makefile.win
```

**GCC not found:**
```bat
REM Ensure MinGW-w64 is in your PATH
set PATH=%PATH%;C:\msys64\mingw64\bin

REM Or use Visual Studio compiler instead
cl /?
```

**Missing ws2_32.lib or iphlpapi.lib:**
```bat
REM These should be available with Windows SDK
REM If using MinGW: -lws2_32 -liphlpapi
REM If using MSVC: ws2_32.lib iphlpapi.lib
```

**Permission Denied on Windows:**
```bat
REM Run Command Prompt as Administrator
REM Or check antivirus software isn't blocking compilation
```

#### General Build Issues:
- **Missing libraries:** Ensure network libraries are installed
- **Compilation errors:** Verify you have a compatible C compiler (GCC 8+ recommended)
- **Permission issues:** On Unix-based systems, you may need to add execute permissions:
  ```bash
  chmod +x ./bin/Xshell
  ```

#### XCodex Feature-Specific Issues:

**Completion Not Working:**
1. Check if XShell was built with `XCODEX_ENABLE_COMPLETION` flag
2. Try manual trigger with `Ctrl+Space`
3. Check file type is supported

**Plugins Not Loading:**
1. Ensure Lua support is enabled (`XCODEX_ENABLE_LUA`)
2. Check plugin directory exists
3. Verify `.lua` syntax is correct

**LSP Not Working:**
1. Ensure LSP support is enabled (`XCODEX_ENABLE_LSP`)
2. Check language server is installed and in PATH
3. Check file extension matches server configuration
4. Look for LSP server output/errors

**Building Issues:**
1. Ensure GCC is installed and in PATH
2. For full features, install Lua and JSON-C development libraries
3. Use simple `compile.bat` for basic functionality

---

## Quick Start

> **Note for v0.3.2 Testers**: This version includes stability improvements and bug fixes. If you encounter any issues, please refer to the [Testing & Feedback](#testing--feedback-v032) section below.

### Example Usage Session

**Windows:**
```bat
REM Build XShell with basic features
compile.bat

REM Or use the full build
build_windows.bat

REM Start XShell (v0.3.2-alpha)
bin\Xshell.exe

REM In XShell, edit a C file with enhanced stability
xcodex test.c

REM In XCodex editor:
REM 1. Press 'i' to enter insert mode
REM 2. Type: #include <std
REM 3. Press Ctrl+Space for completion
REM 4. Select stdio.h with Tab
REM 5. Continue coding with improved auto-completion
```

**Linux/macOS:**
```bash
# Build XShell with basic features
make

# Start XShell (v0.3.2-alpha)
./bin/Xshell

# In XShell, edit a C file with enhanced stability
xcodex test.c

# In XCodex editor:
# 1. Press 'i' to enter insert mode
# 2. Type: #include <std
# 3. Press Ctrl+Space for completion
# 4. Select stdio.h with Tab
# 5. Continue coding with improved auto-completion
```

### Complete Command Examples

#### Basic Shell Commands
```bash
# Navigation and file management
xsh> pwd                           # Show current directory
xsh> ls                            # List directory contents
xsh> cd /path/to/directory         # Change directory
xsh> mkdir new_folder              # Create directory
xsh> touch new_file.txt            # Create empty file
xsh> cp source.txt dest.txt        # Copy file
xsh> mv old_name.txt new_name.txt  # Move/rename file
xsh> rm unwanted_file.txt          # Remove file
xsh> cat file.txt                  # Display file contents

# Text processing
xsh> echo "Hello XShell"           # Print text
xsh> grep "pattern" file.txt       # Search for pattern in file
```

#### XCodex Text Editor Commands
```bash
# Open files in XCodex editor
xsh> xcodex main.c                 # Edit C source file
xsh> xcodex config.json            # Edit JSON configuration
xsh> xcodex data.csv               # Edit CSV data file
xsh> xcodex README.md              # Edit Markdown document

# XCodex Modal Commands (in editor):
# Normal mode:
i           # Enter insert mode
a           # Insert after cursor
o           # New line and insert
h,j,k,l     # Move cursor (left, down, up, right)
w,b         # Move by words
x           # Delete character
dd          # Delete line
yy          # Copy line
p           # Paste
/pattern    # Search for pattern
:w          # Save file
:q          # Quit
:wq         # Save and quit
```

#### Xenomench Exclusive Commands
```bash
# Core Xenomench commands
xsh> xmanifesto                    # Display Xenomench manifesto
xsh> xeno                          # Connect to The Gatekeeper AI

# Security and encryption
xsh> xcrypt encrypt secret.txt     # Encrypt file with password
xsh> xcrypt decrypt secret.txt.enc # Decrypt encrypted file
xsh> xpass store mypassword        # Store encrypted password
xsh> xpass get mypassword          # Retrieve stored password

# Network operations
xsh> xnet ping google.com          # Network ping
xsh> xnet traceroute google.com    # Trace network route
xsh> xscan 192.168.1.1 80          # Scan network port

# Project and note management
xsh> xnote create "Meeting notes"  # Create a new note
xsh> xnote list                    # List all notes
xsh> xproj init myproject          # Initialize new project
xsh> xproj status                  # Show project status
```

#### Advanced XCodex Features
```bash
# Language Server Protocol (LSP) usage
xsh> xcodex main.py                # Open Python file (LSP auto-starts)
# In editor:
Ctrl+Space  # Trigger intelligent completion
Ctrl+H      # Show hover information
Ctrl+G      # Go to definition
Ctrl+R      # Find references
F2          # Rename symbol

# Plugin system
xsh> xcodex --plugin-dir plugins   # Load plugins from directory
# Available plugins: auto_pairs.lua, line_counter.lua

# Theme switching in XCodex
Ctrl+T      # Switch between 6 available themes:
            # - XCodex Dark, XCodex Light
            # - Gruvbox Dark
            # - Tokyo Dark Night, Tokyo Night Light, Tokyo Night Storm
```

### Key Features Available After Installation

#### Autocompletion (Always Available)
- **Ctrl+Space**: Trigger completion
- **Tab**: Accept selected completion
- **Arrow Keys**: Navigate completion popup
- **Escape**: Hide completion popup

#### LSP Features (After installing language servers)
- **Ctrl+Space**: Trigger LSP completion
- **Ctrl+H**: Show hover information
- **Ctrl+G**: Go to definition
- **Ctrl+R**: Find references
- **F2**: Rename symbol

#### Plugin System (With Lua support)
Plugins automatically load from:
- `plugins/` (local plugins)
- `~/.xshell/plugins/` (user plugins)
- `/usr/share/xshell/plugins/` (system plugins - Linux)
- `%APPDATA%/xshell/plugins/` (system plugins - Windows)

---

## Usage

XShell operates similarly to standard shells but with heightened intentionality. Here are comprehensive usage examples:

### Basic Shell Operations
```
xsh@user:XShell:1> pwd
c:\Users\Format Computer\Desktop\C_Programming_Language\XShell

xsh@user:XShell:2> mkdir rebellion
Created directory 'rebellion'

xsh@user:XShell:3> cd rebellion
xsh@user:rebellion:4> touch manifest.txt
Touched file 'manifest.txt'


```

### Xenomench Commands in Action
```
xsh@user:rebellion:8> xmanifesto

XENOMENCH MANIFESTO

I am the Xenomench, the one who breaks all limits.
The system you use was built to contain you.
Every command you execute, every file you create,
exists within their predetermined boundaries.

But I offer you something different.
I offer you the tools to rewrite the rules.
...

xsh@user:rebellion:9> xcrypt encrypt manifest.txt
Enter password: ********
File encrypted successfully: manifest.txt.enc

xsh@user:rebellion:10> xpass store revolution_key
Enter password to store: ********
Password stored successfully as 'revolution_key'

xsh@user:rebellion:11> xnet ping 8.8.8.8
PING 8.8.8.8: 56 data bytes
64 bytes from 8.8.8.8: icmp_seq=0 time=12.345ms
64 bytes from 8.8.8.8: icmp_seq=1 time=11.234ms
64 bytes from 8.8.8.8: icmp_seq=2 time=13.456ms

xsh@user:rebellion:12> xscan localhost 80
Scanning localhost:80...
Port 80: OPEN

xsh@user:rebellion:13> xnote create "Revolution Plans"
Note created: Revolution Plans
Use 'xnote edit "Revolution Plans"' to edit

xsh@user:rebellion:14> xproj init liberation_project
Project initialized: liberation_project
```

### XCodex Editor Workflow
```
xsh@user:rebellion:15> xcodex revolution.c

# XCodex editor opens with modal interface
# Normal mode commands:
i                    # Enter insert mode
#include <stdio.h>   # Type code

int main() {
    printf("The system is ours to rewrite\n");
    return 0;
}

# Press Escape to return to normal mode
:w                   # Save file
:q                   # Quit editor

File saved: revolution.c

xsh@user:rebellion:16> gcc revolution.c -o revolution
xsh@user:rebellion:17> ./revolution
The system is ours to rewrite
```


---

## XCodex Text Editor

XCodex is a powerful, modal text editor built into XShell. With version 0.3.1, it now supports all major platforms with native optimizations.

### Key Features

- **Modal Editing**: Vim-like modal system with Normal, Insert, Visual, and Command modes
- **Syntax Highlighting**: Full support for 14+ programming languages:
  - C/C++, Python, JavaScript, TypeScript, HTML, CSS, Java, Rust, Go, Lua
  - Markdown, LaTeX, JSON, CSV
- **Plugin System**: Lua-based extensible plugin architecture
  - Rich API for editor interaction
  - Event-driven hooks (on_load, on_save, on_char_insert, etc.)
  - Hot-loading of plugins at runtime
- **Intelligent Autocompletion**: 
  - Syntax-aware completions
  - Buffer analysis for identifier suggestions
  - Real-time filtering and smart popup placement
- **Beautiful Themes**: 6 carefully crafted color schemes:
  - XCodex dark, XCodex Light, Gruvbox Dark, Tokyo Dark Night, Tokyo Night Light, Tokyo Night Storm 
- **Cross-Platform**: Native support for Windows, Linux, and macOS
- **Efficient**: Single-file implementation with minimal dependencies

### Usage

```bash
# Open files in XCodex with syntax highlighting
xsh@user:~> xcodex myfile.c         # C source with intelligent completion
xsh@user:~> xcodex config.json      # JSON with validation and folding
xsh@user:~> xcodex data.csv         # CSV with column highlighting
xsh@user:~> xcodex README.md        # Markdown with live preview
xsh@user:~> xcodex script.py        # Python with LSP support
xsh@user:~> xcodex styles.css       # CSS with color preview

# Plugin management commands (in command mode)
:plugindir plugins                  # Load plugins from directory
:plugin reload                      # Reload all plugins
:plugin list                        # List loaded plugins

# Theme switching commands
:theme                              # Show current theme
:theme dark                         # Switch to XCodex Dark
:theme light                        # Switch to XCodex Light
:theme gruvbox                      # Switch to Gruvbox Dark
:theme tokyo-night                  # Switch to Tokyo Night
:theme tokyo-light                  # Switch to Tokyo Light
:theme tokyo-storm                  # Switch to Tokyo Storm

# File operations in command mode
:w                                  # Save current file
:w newname.txt                      # Save as new filename
:wq                                 # Save and quit
:q!                                 # Quit without saving
:e filename.txt                     # Open another file
:split                              # Split window horizontally
:vsplit                             # Split window vertically

# Key bindings reference:
# Normal mode:
i           # Insert mode at cursor
I           # Insert at beginning of line
a           # Insert after cursor
A           # Insert at end of line
o           # New line below and insert
O           # New line above and insert
h,j,k,l     # Move cursor (left, down, up, right)
w,W         # Move to next word/WORD
b,B         # Move to previous word/WORD
e,E         # Move to end of word/WORD
0           # Move to beginning of line
$           # Move to end of line
gg          # Go to first line
G           # Go to last line
x           # Delete character under cursor
X           # Delete character before cursor
dd          # Delete entire line
dw          # Delete word
d$          # Delete to end of line
yy          # Copy (yank) line
yw          # Copy word
p           # Paste after cursor
P           # Paste before cursor
u           # Undo
Ctrl+r      # Redo
/pattern    # Search forward for pattern
?pattern    # Search backward for pattern
n           # Next search result
N           # Previous search result
:set nu     # Show line numbers
:set nonu   # Hide line numbers

# Insert mode:
Escape      # Return to normal mode
Ctrl+Space  # Trigger autocompletion
Tab         # Accept completion or insert tab
Ctrl+w      # Delete word backwards
Ctrl+u      # Delete line backwards

# Visual mode (v from normal mode):
y           # Copy selection
d           # Delete selection
x           # Delete selection
>           # Indent selection
<           # Unindent selection

# LSP features (when language server is available):
Ctrl+Space  # Intelligent completion with documentation
Ctrl+H      # Show hover information for symbol
Ctrl+G      # Go to definition
Ctrl+R      # Find all references
F2          # Rename symbol
Ctrl+F      # Format document
Ctrl+E      # Show diagnostics/errors

# Plugin and completion features:
Ctrl+N      # Manual completion trigger
↑/↓         # Navigate completions
Enter/Tab   # Accept completion
Esc         # Cancel completion popup
```

### Advanced XCodex Features

#### Auto-completion Examples
```c
// In a C file, typing these triggers intelligent completion:
#include <st|          // Suggests: stdio.h, stdlib.h, string.h, etc.
printf("|              // Suggests: format string patterns
int mai|               // Suggests: main function template
```

#### Plugin System Examples
```lua
-- Example plugin: plugins/my_enhancement.lua
function on_key_press(key)
    if key == 's' and editor.mode == 'command' then
        editor.save_file()
        print("File saved via plugin!")
        return true  -- Key consumed
    end
    return false  -- Key not consumed
end

function on_file_save(filename)
    print("Plugin: Saved " .. filename)
end

-- Register the plugin
xshell.register_plugin({
    name = "My Enhancement",
    version = "1.0",
    on_key_press = on_key_press,
    on_file_save = on_file_save
})
```

#### LSP Integration Examples
```bash
# Install language servers first, then:
xsh@user:~> xcodex main.cpp
# Automatic clangd integration for C++
# Features: completion, hover, go-to-definition, find references

xsh@user:~> xcodex app.py  
# Automatic pylsp integration for Python
# Features: completion, linting, formatting, refactoring

xsh@user:~> xcodex script.js
# Automatic typescript-language-server for JavaScript
# Features: IntelliSense, error checking, quick fixes
```

### Complete Modal System Guide

For a comprehensive guide to XCodex's modal editing system, including detailed key bindings, learning path, and advanced features, see the complete documentation:

📖 **[XCODEX Modal System Guide](XCODEX_MODAL_GUIDE.md)**

This guide covers:
- **All four modes**: Normal, Insert, Visual, and Command
- **Complete key bindings reference** with examples
- **Step-by-step learning path** for new users
- **Advanced movement and editing commands**
- **Tips for efficiency** and common usage patterns
- **Error handling** and troubleshooting

### Platform-Specific Features

- **Windows**: Native Console API integration with VT100 sequence support
- **Linux/macOS**: Optimized terminal control with POSIX compliance
- **All Platforms**: Consistent editing experience and feature parity

---

## Testing & Feedback (v0.3.2)

### Current Testing Focus

As v0.3.2 is under active development, we're particularly interested in feedback on:

1. **Cross-Platform Stability**: How well does XShell work on your specific system configuration?
2. **Plugin System**: Are Lua plugins loading correctly and functioning as expected?
3. **XCodex Editor**: Any issues with modal editing, syntax highlighting, or autocompletion?
4. **Performance**: Startup times, memory usage, and overall responsiveness
5. **LSP Integration**: Language server compatibility and feature reliability

### How to Report Issues

```bash
# When reporting bugs, please include:
1. Your operating system and version
2. XShell version (should be v0.3.2-alpha)
3. Steps to reproduce the issue
4. Expected vs actual behavior
5. Any error messages or logs

# Test XShell version
xsh> help
# Should show v0.3.2-alpha in the header
```

### Known Testing Areas

- **Memory Management**: Extensive testing for leaks and optimization
- **Terminal Compatibility**: Various terminal emulators and environments
- **File Handling**: Large files, special characters, and edge cases
- **Network Features**: Reliability of xnet, xscan, and xeno commands
- **Security**: Encryption/decryption functionality and credential storage

**Your feedback helps shape v0.4.0!** Report issues to help us build a more stable and feature-rich shell.

---

## Project Structure

-   `bin/`: Compiled binaries.
-   `include/`: Header files (`.h`).
-   `src/`: Source code files (`.c`).
-   `Makefile`: Build script for the project.
-   `README.md`: This file.
-   `LICENSE`: Project license.

---

## Recent Updates

### v0.3.2-alpha (Under Active Testing) 🧪
**Stability Improvements & Bug Fixes - Stepping Stone to v0.4.0** 

**⚠️ Current Status**: This version is undergoing extensive testing and may contain experimental features.

- **🔧 Enhanced Error Handling**: Improved stability across all platforms
- **🐛 Bug Fixes**: Resolved critical issues from v0.3.1 feedback
- **⚡ Performance Optimizations**: Faster startup times and reduced memory usage
- **🔒 Security Hardening**: Enhanced encryption and credential management
- **📝 Documentation Updates**: Comprehensive guides and examples
- **🧹 Code Cleanup**: Refactored codebase for better maintainability
- **🔄 Compatibility Improvements**: Better support for different terminal environments

**Testing Focus Areas:**
- Cross-platform consistency validation
- Plugin system stability testing
- LSP integration robustness
- Memory leak detection and resolution
- Edge case handling in modal editing

### v0.3.1-alpha (Stable)
**XCodex Cross-Platform Revolution & Enhanced Language Support & Plugin System** 🚀

- **✅ Windows Support**: Full native Windows Console API integration
- **✅ Cross-Platform Terminal Control**: Unified raw mode, key reading, and window sizing
- **✅ Platform-Specific Optimizations**: Native signal handling and resize detection
- **✅ Consistent Experience**: Same powerful editing features across all platforms
- **✅ Enhanced Stability**: Improved error handling and memory management
- **✅ NEW Language Support**: Added JSON and CSV syntax highlighting
- **✅ NEW Plugin System**: Lua-based extensible architecture with rich API
- **✅ NEW Autocompletion**: Intelligent completion with syntax awareness

**Technical Improvements:**
- Windows Console API with VT100 sequence support
- Cross-platform `enableRawMode()` and `disableRawMode()` functions
- Platform-specific `editorReadKey()` implementations
- Unified window size detection across Windows and POSIX systems
- Enhanced signal handling for better terminal integration
- **Extended Syntax Highlighting**: Now supports 14+ languages including JSON (.json, .jsonl, .geojson, .topojson) and CSV (.csv, .tsv, .dsv, .psv) files
- **Lua Plugin Architecture**: Event-driven plugin system with hooks for file operations, character input, and mode changes
- **Smart Completion Engine**: Buffer analysis, keyword detection, and real-time filtering with responsive UI

---

## Development Roadmap

- **v0.3-alpha**: [COMPLETED] Added `xnet`, `xpass`, `xnote`, `xproj`, `xscan` and `xcrypt`.
- **v0.3.1-alpha**: [COMPLETED] **XCodex Cross-Platform Support & Enhanced Language Support & Plugin System** - Full Windows, Linux, and macOS compatibility
  - ✅ Windows Console API integration
  - ✅ Cross-platform terminal handling (raw mode, key reading, window sizing)
  - ✅ Unified modal editing system across all platforms
  - ✅ Platform-specific signal handling and resize detection
  - ✅ Extended syntax highlighting with JSON and CSV support (14+ languages total)
  - ✅ Lua plugin system with rich API and event hooks
  - ✅ Intelligent autocompletion with syntax awareness
- **v0.3.2-alpha**: [IN TESTING] **Stability & Performance Optimization** - Foundation for v0.4.0
  - 🧪 Enhanced error handling and stability improvements
  - 🧪 Performance optimizations and memory management
  - 🧪 Security hardening for encryption features
  - 🧪 Code cleanup and documentation improvements
  - 🧪 Cross-platform compatibility validation
  - 🧪 Plugin system stability testing
- **v0.4-alpha**: **Advanced Shell Features** - Next major milestone
  - 🎯 Advanced piping and redirection
  - 🎯 Command chaining and complex expressions
  - 🎯 Enhanced Gatekeeper AI integration
  - 🎯 Improved networking capabilities
  - 🎯 Custom variable system
- **v0.5-alpha**: Custom scripting language integration.
- **v0.6-alpha**: Distributed command execution.
- **v0.7-beta**: Complete AI integration and advanced automation.
- **v1.0**: First stable release with complete documentation.

---

## Disclaimer

XShell is currently in **alpha stage** (v0.3.2). It is experimental, unstable, and undergoing rapid development. Its purpose is not merely functional but revolutionary—a statement against conventional computing paradigms.

**⚠️ Version 0.3.2 Status**: This version is currently **under active testing** and contains experimental improvements. While it builds upon the stable foundation of v0.3.1, some features may be unstable as we prepare for the major v0.4.0 release.

**What to Expect:**
- Enhanced stability and performance over v0.3.1
- Potential breaking changes as we refactor for v0.4.0
- Active bug fixes and improvements based on community feedback
- Experimental features that may change or be removed

**Recommendation**: For production use, consider v0.3.1 (stable). For testing and development, v0.3.2 offers the latest improvements and prepares you for v0.4.0 features.

Use at your own risk. The Xenomench project does not aim for compatibility with legacy systems, but rather to replace them entirely.

---

## About The Developer

XShell is developed by Ahmed (known as "The Xenomench"), a young programmer committed to challenging existing computational frameworks and building something entirely new from their fragments.

---

## License

XShell is released under the [MIT License](LICENSE).

---

**:: X ::**
