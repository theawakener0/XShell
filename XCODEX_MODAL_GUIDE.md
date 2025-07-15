# XCodex Modal System Documentation

## Overview

XCodex now features a powerful modal editing system inspired by Vim, making it easy to learn and use. The modal system provides efficient navigation and editing capabilities while maintaining simplicity.

## The Four Modes

### 1. NORMAL Mode (Default)
- **Purpose**: Navigate and execute commands
- **Indicator**: Status bar shows "NORMAL"
- **How to enter**: Press `Esc` from any mode, or start XCodex

### 2. INSERT Mode
- **Purpose**: Insert and edit text
- **Indicator**: Status bar shows "INSERT"
- **How to enter**: Press `i`, `a`, `o`, `I`, `A`, or `O` from NORMAL mode

### 3. VISUAL Mode
- **Purpose**: Select text (basic implementation)
- **Indicator**: Status bar shows "VISUAL"
- **How to enter**: Press `v` from NORMAL mode

### 4. COMMAND Mode
- **Purpose**: Execute editor commands
- **Indicator**: Status bar shows ":"
- **How to enter**: Press `:` from NORMAL mode

## Key Bindings Reference

### Mode Switching (from NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `i` | Insert before cursor | Enter INSERT mode at current cursor position |
| `I` | Insert at line start | Move to beginning of line and enter INSERT mode |
| `a` | Insert after cursor | Move cursor right and enter INSERT mode |
| `A` | Insert at line end | Move to end of line and enter INSERT mode |
| `o` | Open line below | Create new line below and enter INSERT mode |
| `O` | Open line above | Create new line above and enter INSERT mode |
| `v` | Visual mode | Enter VISUAL mode for text selection |
| `:` | Command mode | Enter COMMAND mode for ex commands |

### Basic Movement (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `h` | Move left | Move cursor one character left |
| `j` | Move down | Move cursor one line down |
| `k` | Move up | Move cursor one line up |
| `l` | Move right | Move cursor one character right |
| `←↓↑→` | Arrow keys | Alternative movement keys |

### Word Movement (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `w` | Next word | Move to start of next word |
| `b` | Previous word | Move to start of previous word |

### Line Movement (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `0` | Line start | Move to beginning of line |
| `$` | Line end | Move to end of line |

### File Navigation (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `gg` | First line | Go to first line of file |
| `G` | Last line | Go to last line of file |
| `5G` | Go to line 5 | Go to specific line (replace 5 with any number) |

### Editing Commands (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `x` | Delete character | Delete character under cursor |
| `X` | Delete left | Delete character before cursor |

### Motion Count Prefixes

You can prefix most movement commands with a number to repeat them:

| Examples | Description |
|----------|-------------|
| `5j` | Move down 5 lines |
| `3w` | Move forward 3 words |
| `10h` | Move left 10 characters |
| `2x` | Delete 2 characters |

### File Operations (Available in all modes)

| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+S` | Save | Save current file |
| `Ctrl+Q` | Quit | Quit editor (prompts if unsaved changes) |

### Display Options

| Key | Action | Description |
|-----|--------|-------------|
| `Ctrl+T` | Cycle themes | Switch between available themes |
| `Ctrl+N` | Toggle line numbers | Show/hide line numbers |

### Search (NORMAL mode)

| Key | Action | Description |
|-----|--------|-------------|
| `/` | Search forward | Search for text in file |
| `Ctrl+F` | Find | Alternative search command |

### INSERT Mode Keys

| Key | Action | Description |
|-----|--------|-------------|
| `Esc` | Exit INSERT mode | Return to NORMAL mode |
| `Ctrl+C` | Exit INSERT mode | Alternative way to return to NORMAL mode |
| `Enter` | New line | Insert a new line |
| `Backspace` | Delete left | Delete character to the left |
| `Delete` | Delete right | Delete character to the right |
| `←↓↑→` | Movement | Move cursor (stays in INSERT mode) |

### COMMAND Mode Commands

| Command | Action | Description |
|---------|--------|-------------|
| `:q` | Quit | Quit if no unsaved changes |
| `:q!` | Force quit | Quit without saving changes |
| `:w` | Write | Save current file |
| `:wq` | Write and quit | Save file and quit |
| `:5` | Go to line 5 | Go to specific line number |

### VISUAL Mode (Basic)

| Key | Action | Description |
|-----|--------|-------------|
| `Esc` | Exit VISUAL mode | Return to NORMAL mode |
| `hjkl` | Extend selection | Move cursor to extend selection |
| `w`, `b` | Word selection | Extend selection by words |
| `0`, `$` | Line selection | Extend selection to line boundaries |

## Learning Path

### Step 1: Basic Navigation
1. Open a file with XCodex
2. Practice basic movement: `h`, `j`, `k`, `l`
3. Try line movement: `0`, `$`
4. Practice word movement: `w`, `b`

### Step 2: Mode Switching
1. Enter INSERT mode with `i`
2. Type some text
3. Press `Esc` to return to NORMAL mode
4. Try other insert commands: `a`, `o`, `A`, `O`

### Step 3: Basic Editing
1. Use `x` to delete characters
2. Use `i` to insert text
3. Practice entering and exiting INSERT mode

### Step 4: File Operations
1. Make changes to a file
2. Save with `Ctrl+S`
3. Try command mode with `:w`
4. Practice quitting with `:q`

### Step 5: Advanced Movement
1. Use number prefixes: `5j`, `3w`
2. Practice `gg` and `G` for file navigation
3. Use `:5` to go to specific lines

## Tips for Efficiency

1. **Stay in NORMAL mode**: Only enter INSERT mode when actually typing text
2. **Use number prefixes**: `5j` is faster than pressing `j` five times
3. **Learn the home row**: `hjkl` keeps your fingers on the home row
4. **Combine commands**: `A` (append at end) is faster than `$` then `a`
5. **Use word movement**: `w` and `b` are faster than character movement
6. **Practice mode switching**: Smooth transitions between modes improve efficiency

## Common Patterns

### Quick edits
1. Navigate to position (NORMAL mode)
2. Enter INSERT mode (`i`, `a`, etc.)
3. Make changes
4. Press `Esc` to return to NORMAL mode

### Line operations
1. `0` - Go to start of line
2. `A` - Go to end of line and enter INSERT mode
3. `o` - Create new line below
4. `O` - Create new line above

### File navigation
1. `gg` - Top of file
2. `G` - Bottom of file
3. `:50` - Go to line 50
4. `5G` - Go to line 5

## Error Messages

- **"No write since last change"**: Use `:w` to save or `:q!` to quit without saving
- **"WARNING!!! File has unsaved changes"**: Press `Ctrl+Q` multiple times or save first
- **"Unknown command"**: Check command syntax in COMMAND mode

## Customization

The modal system respects XCodex's existing features:
- **Themes**: Use `Ctrl+T` to cycle through themes
- **Line numbers**: Use `Ctrl+N` to toggle line numbers
- **Syntax highlighting**: Automatic based on file extension

## Future Enhancements

The current implementation provides a solid foundation. Future versions may include:
- Copy/paste operations (yank/put)
- Advanced text objects
- Macros and registers
- Advanced search and replace
- More visual mode operations

## Getting Help

- Status bar shows current mode and helpful information
- Press `Esc` to return to NORMAL mode if stuck
- Use `:q!` to quit without saving if needed
- Practice with simple text files first

---

*This modal system makes XCodex a powerful yet approachable editor that grows with your skills.*
