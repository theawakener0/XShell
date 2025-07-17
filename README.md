# XShell v0.3.1 - alpha

**_Break the terminal. Break the system. Build anew._**

![Version](https://img.shields.io/badge/version-v0.3.1--alpha-orange) ![License](https://img.shields.io/badge/license-MIT-blue) ![Build](https://img.shields.io/badge/build-passing-brightgreen)

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
    - **NEW v0.3.1**: Full cross-platform compatibility with Windows Console API support
    - **Features**: Vim-like modal editing, syntax highlighting for 14+ languages, 6 beautiful themes
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

### Prerequisites

- C Compiler (GCC or Clang recommended)
- Make (for automated building)
- Git (for version control)
- Network libraries (Windows: ws2_32, iphlpapi)

**New in v0.3.1**: XCodex now has full cross-platform support with no additional dependencies required!

### Building XShell

1. **Get the source code:**
    ```bash
    git clone https://github.com/theawakener0/XShell.git
    cd XShell
    ```

2. **Build the project:**
    
    **Using Make (recommended):**
    ```bash
    make
    ```
    
    **Manual compilation:**
    - **Linux/macOS:**
      ```bash
      mkdir -p bin
      gcc -Iinclude src/*.c -o bin/Xshell -lm
      ```
    
    - **Windows:**
      ```bash
      mkdir bin
      gcc -Iinclude src\*.c -o bin\Xshell.exe -lws2_32 -liphlpapi
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
    ```bash
    # Test the new cross-platform XCodex editor
    make test-xcodex
    
    # Or manually test with various file types:
    # Run XShell and type: xcodex test.c
    # Run XShell and type: xcodex config.json
    # Run XShell and type: xcodex data.csv
    ```

### Troubleshooting

- **Missing libraries:** Ensure network libraries are installed
- **Compilation errors:** Verify you have a compatible C compiler (GCC 8+ recommended)
- **Permission issues:** On Unix-based systems, you may need to add execute permissions:
  ```bash
  chmod +x ./bin/Xshell
  ```

3.  **Run XShell:**
    -   **Linux/macOS:**
        ```bash
        ./bin/Xshell
        ```
    -   **Windows:**
        ```bash
        .\bin\Xshell.exe
        ```

---

## Usage

XShell operates similarly to standard shells but with heightened intentionality:

```
xsh@user:XShell:1> pwd
c:\Users\Format Computer\Desktop\C_Programming_Language\XShell

xsh@user:XShell:2> mkdir rebellion
Created directory 'rebellion'

xsh@user:XShell:3> cd rebellion
xsh@user:rebellion:4> touch manifest.txt
Touched file 'manifest.txt'


xsh@user:rebellion:5> xmanifesto

XENOMENCH MANIFESTO

I am the Xenomench, the one who breaks all limits.
...
```

---

## XCodex Text Editor

XCodex is a powerful, modal text editor built into XShell. With version 0.3.1, it now supports all major platforms with native optimizations.

### Key Features

- **Modal Editing**: Vim-like modal system with Normal, Insert, Visual, and Command modes
- **Syntax Highlighting**: Full support for 14+ programming languages:
  - C/C++, Python, JavaScript, TypeScript, HTML, CSS, Java, Rust, Go, Lua
  - Markdown, LaTeX, JSON, CSV
- **Beautiful Themes**: 6 carefully crafted color schemes:
  - XCodex dark, XCodex Light, Gruvbox Dark, Tokyo Dark Night, Tokyo Night Light, Tokyo Night Storm 
- **Cross-Platform**: Native support for Windows, Linux, and macOS
- **Efficient**: Single-file implementation with minimal dependencies

### Usage

```bash
# Open a file in XCodex
xsh@user:~> xcodex myfile.c

# Try the new JSON and CSV support
xsh@user:~> xcodex config.json
xsh@user:~> xcodex data.csv

# Key bindings (Normal mode):
# i - Insert mode
# v - Visual mode  
# : - Command mode
# Ctrl+S - Save
# Ctrl+Q - Quit
# Ctrl+T - Switch theme
# Ctrl+N - Toggle line numbers
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

## Project Structure

-   `bin/`: Compiled binaries.
-   `include/`: Header files (`.h`).
-   `src/`: Source code files (`.c`).
-   `Makefile`: Build script for the project.
-   `README.md`: This file.
-   `LICENSE`: Project license.

---

## Recent Updates

### v0.3.1-alpha (Latest)
**XCodex Cross-Platform Revolution & Enhanced Language Support** 🚀

- **✅ Windows Support**: Full native Windows Console API integration
- **✅ Cross-Platform Terminal Control**: Unified raw mode, key reading, and window sizing
- **✅ Platform-Specific Optimizations**: Native signal handling and resize detection
- **✅ Consistent Experience**: Same powerful editing features across all platforms
- **✅ Enhanced Stability**: Improved error handling and memory management
- **✅ NEW Language Support**: Added JSON and CSV syntax highlighting

**Technical Improvements:**
- Windows Console API with VT100 sequence support
- Cross-platform `enableRawMode()` and `disableRawMode()` functions
- Platform-specific `editorReadKey()` implementations
- Unified window size detection across Windows and POSIX systems
- Enhanced signal handling for better terminal integration
- **Extended Syntax Highlighting**: Now supports 14+ languages including JSON (.json, .jsonl, .geojson, .topojson) and CSV (.csv, .tsv, .dsv, .psv) files

---

## Development Roadmap

- **v0.3-alpha**: [COMPLETED] Added `xnet`, `xpass`, `xnote`, `xproj`, `xscan` and `xcrypt`.
- **v0.3.1-alpha**: [COMPLETED] **XCodex Cross-Platform Support & Enhanced Language Support** - Full Windows, Linux, and macOS compatibility
  - ✅ Windows Console API integration
  - ✅ Cross-platform terminal handling (raw mode, key reading, window sizing)
  - ✅ Unified modal editing system across all platforms
  - ✅ Platform-specific signal handling and resize detection
  - ✅ Extended syntax highlighting with JSON and CSV support (14+ languages total)
- **v0.4-alpha**: Advanced piping and redirection.
- **v0.5-alpha**: Custom scripting language integration.
- **v0.6-alpha**: Distributed command execution.
- **v0.7-beta**: Enhanced Gatekeeper AI integration.
- **v1.0**: First stable release with complete documentation.

---

## Disclaimer

XShell is currently in **alpha stage** (v0.3.1). It is experimental, unstable, and undergoing rapid development. Its purpose is not merely functional but revolutionary—a statement against conventional computing paradigms.

The latest update (v0.3.1) significantly improves cross-platform compatibility, especially for the XCodex text editor, and adds enhanced language support with JSON and CSV syntax highlighting, but the project remains in active development.

Use at your own risk. The Xenomench project does not aim for compatibility with legacy systems, but rather to replace them entirely.

---

## About The Developer

XShell is developed by Ahmed (known as "The Xenomench"), a young programmer committed to challenging existing computational frameworks and building something entirely new from their fragments.

---

## License

XShell is released under the [MIT License](LICENSE).

---

**:: X ::**
