# XShell v0.2 - alpha

**_Break the terminal. Break the system. Build anew._**

![Version](https://img.shields.io/badge/version-v0.2--alpha-orange) ![License](https://img.shields.io/badge/license-MIT-blue) ![Build](https://img.shields.io/badge/build-passing-brightgreen)

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

- **Modular Design**: Source code organized into modules for better maintainability and scalability (`src/`, `include/`).
- **Makefile Build System**: Simplified compilation process.
- **Clean, minimal interface** with enhanced prompt displays.
- **Command history** for faster workflow.
- **Cross-platform compatibility** (Windows, Linux, macOS).

---

## Installation

### Prerequisites

- A C Compiler (GCC recommended)
- Make (for using the Makefile)
- Git (optional, for cloning)

### Building XShell

1.  **Clone the repository (or download the source):**
    ```bash
    git clone https://github.com/theawakener0/XShell.git
    cd XShell
    ```

2.  **Compile the project using Make:**
    This will compile the source files from the `src/` directory, using headers from `include/`, and place the executable in the `bin/` directory.

    -   **Linux/macOS:**
        ```bash
        make
        ```
    -   **Windows (MinGW/MSYS2 recommended):**
        ```bash
        make
        ```
        (If using a different environment, you might need to adjust the `Makefile` or compile manually. The `Makefile` is set up for GCC.)

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

## Project Structure

-   `bin/`: Compiled binaries.
-   `include/`: Header files (`.h`).
-   `src/`: Source code files (`.c`).
-   `Makefile`: Build script for the project.
-   `README.md`: This file.
-   `LICENSE`: Project license.

---

## Development Roadmap

- **v0.3-alpha**: Add more commands and features.
- **v0.4-alpha**: Advanced piping and redirection.
- **v0.5-alpha**: Custom scripting language integration.
- **v0.6-alpha**: Distributed command execution.
- **v0.7-beta**: Enhanced Gatekeeper AI integration.
- **v1.0**: First stable release with complete documentation.

---

## Disclaimer

XShell is currently in **alpha stage** (v0.2). It is experimental, unstable, and undergoing rapid development. Its purpose is not merely functional but revolutionary—a statement against conventional computing paradigms.

Use at your own risk. The Xenomench project does not aim for compatibility with legacy systems, but rather to replace them entirely.

---

## About The Developer

XShell is developed by Ahmed (known as "The Xenomench"), a young programmer committed to challenging existing computational frameworks and building something entirely new from their fragments.

---

## License

XShell is released under the [MIT License](LICENSE).

---

**:: X ::**
