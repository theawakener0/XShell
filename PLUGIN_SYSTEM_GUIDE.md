# XCodex Plugin System Documentation

## Overview

XCodex now features a comprehensive plugin system powered by Lua, intelligent autocompletion, and enhanced TUI features. This makes XCodex not just a text editor, but a fully extensible development environment.

## Features Added

### 🔌 Lua Plugin System
- **Full Lua API**: Plugins can interact with the editor through a rich API
- **Auto-Loading**: Plugins automatically load from multiple directories on startup
- **Hot Loading**: Load plugins at runtime without restarting
- **Event Hooks**: Plugins can respond to editor events (char insertion, file save, etc.)
- **Plugin Management**: Built-in commands for loading and managing plugins

### 🧠 Intelligent Autocompletion  
- **Syntax-Aware**: Completion based on current file's programming language
- **LSP Integration**: Language Server Protocol support for intelligent suggestions
- **Buffer Analysis**: Suggests identifiers from current file
- **Keyword Completion**: Language-specific keyword suggestions
- **Smart Filtering**: Real-time filtering as you type

### 🌐 LSP (Language Server Protocol) Support
- **Multi-Language Support**: C/C++ (clangd), Rust (rust-analyzer), Python (pylsp), TypeScript, Go (gopls)
- **Auto-Start**: LSP servers automatically start when opening compatible files
- **Code Intelligence**: Hover information, go-to-definition, find references
- **Diagnostics**: Real-time error and warning reporting
- **Code Actions**: Quick fixes and refactoring suggestions
- **Document Symbols**: Navigate through code structure

### 🚀 Auto-Loading System
- **Multiple Locations**: Loads from `./plugins/`, `~/.xcodex/plugins/`, system directories
- **Built-in Fallback**: Provides essential plugins if no external plugins found
- **Smart Detection**: Automatically detects and loads .lua files
- **Startup Integration**: All plugins loaded and initialized on editor startup

### 🎨 Enhanced TUI
- **Plugin Status**: Status bar shows loaded plugin count
- **Completion Popup**: Beautiful floating completion menu
- **Real-time Feedback**: Visual indicators for active features
- **Responsive Design**: Adapts to terminal size changes

## Plugin API Reference

### Available Functions

```lua
-- Get current line content
local line = xcodex.get_current_line()

-- Get cursor position (1-based)
local pos = xcodex.get_cursor_pos()
-- Returns: {line = number, col = number}

-- Set cursor position (1-based)
xcodex.set_cursor_pos(line, col)

-- Insert text at cursor
xcodex.insert_text("Hello World")

-- Get entire file content
local content = xcodex.get_file_content()

-- Set status message
xcodex.set_status_message("Plugin message")

-- Get current filename
local filename = xcodex.get_filename()

-- Get current editor mode
local mode = xcodex.get_mode()
-- Returns: "NORMAL", "INSERT", "VISUAL", etc.
```

### Plugin Hooks

```lua
-- Called when plugin is loaded
function on_load()
    xcodex.set_status_message("Plugin loaded!")
end

-- Called when file is saved
function on_save()
    -- Custom save actions
end

-- Called when character is inserted
function on_char_insert(char)
    -- Respond to character input
end

-- Called when character is deleted
function on_char_delete(char)
    -- Respond to character deletion
end

-- Called when mode changes
function on_mode_change(old_mode, new_mode)
    -- Respond to mode transitions
end
```

## Using the Plugin System

### Auto-Loading (New!)

Plugins now automatically load from multiple locations when XCodex starts:

1. **Current Directory**: `./plugins/`
2. **User Directory**: `~/.xcodex/plugins/` (Linux/macOS) or `%USERPROFILE%\.xcodex\plugins\` (Windows)
3. **System Directory**: `/usr/share/xcodex/plugins/` (Linux) or `C:\Program Files\XShell\plugins\` (Windows)
4. **Built-in Plugins**: If no external plugins found, built-in plugins are automatically created

### Manual Plugin Loading

1. **Single Plugin**:
   ```
   :plugin path/to/plugin.lua
   ```

2. **Plugin Directory**:
   ```
   :plugindir plugins
   ```

3. **Check Loaded Plugins**:
   ```
   :plugins
   ```

### LSP (Language Server Protocol) Usage

#### Supported Languages & Servers:
- **C/C++**: clangd (`sudo apt install clangd` or `brew install llvm`)
- **Rust**: rust-analyzer (`rustup component add rust-analyzer`)
- **Python**: pylsp (`pip install python-lsp-server`)
- **TypeScript/JavaScript**: typescript-language-server (`npm install -g typescript-language-server`)
- **Go**: gopls (`go install golang.org/x/tools/gopls@latest`)

#### LSP Features:
- **Auto-Start**: LSP servers automatically start when opening compatible files
- **Completion**: Enhanced code completion with LSP suggestions
- **Hover**: `Ctrl+H` for hover information (planned)
- **Go-to-Definition**: `Ctrl+]` to jump to definitions (planned)
- **Diagnostics**: Real-time error highlighting
- **Document Formatting**: Code formatting support (planned)

#### LSP Commands:
```
:lsp status          # Show LSP server status
:lsp restart <lang>  # Restart LSP server for language
:lsp stop <lang>     # Stop LSP server
:lsp start <lang>    # Start LSP server
```

3. **Check Loaded Plugins**:
   ```
   :plugins
   ```

### Autocompletion Usage

1. **Trigger Completion**: `Ctrl+N` in insert mode
2. **Navigate**: `↑`/`↓` arrow keys  
3. **Accept**: `Tab` or `Enter`
4. **Cancel**: `Esc`

Auto-completion also triggers automatically when typing identifiers.

## Example Plugins

### Auto Pairs Plugin
```lua
plugin_info = {
    name = "Auto Pairs",
    version = "1.0",
    author = "XCodex Team",
    description = "Automatically insert closing brackets and quotes"
}

local pairs_map = {
    ['('] = ')',
    ['['] = ']',
    ['{'] = '}',
    ['"'] = '"',
    ["'"] = "'"
}

function on_char_insert(char)
    if pairs_map[char] then
        local pos = xcodex.get_cursor_pos()
        xcodex.insert_text(pairs_map[char])
        xcodex.set_cursor_pos(pos.line, pos.col)
    end
end
```

### Line Statistics Plugin
```lua
plugin_info = {
    name = "Line Counter",
    version = "1.1",
    author = "XCodex Team",
    description = "Show file statistics"
}

function count_lines()
    local content = xcodex.get_file_content()
    local lines = 0
    local words = 0
    
    for line in content:gmatch("[^\r\n]*") do
        lines = lines + 1
        for word in line:gmatch("%S+") do
            words = words + 1
        end
    end
    
    xcodex.set_status_message(string.format("Lines: %d, Words: %d", lines, words))
end
```

## Building with Plugin Support

### Dependencies

**Required for LSP Support:**
- `json-c` library for JSON parsing

**Optional for Plugin System:**
- Lua 5.3 or 5.4 development libraries

### Automatic Detection
The Makefile automatically detects available dependencies:

```bash
make                 # Builds with all available features
make check-deps      # Check all dependencies
make test-plugins    # Test plugin system
make test-lsp        # Test LSP functionality
```

### Manual Installation

**Ubuntu/Debian:**
```bash
# Plugin support
sudo apt install liblua5.4-dev

# LSP support  
sudo apt install libjson-c-dev

# Language servers
sudo apt install clangd                    # C/C++
pip install python-lsp-server             # Python
npm install -g typescript-language-server # TypeScript/JavaScript
rustup component add rust-analyzer        # Rust
go install golang.org/x/tools/gopls@latest # Go
```

**macOS (Homebrew):**
```bash
# Plugin support
brew install lua

# LSP support
brew install json-c

# Language servers
brew install llvm                          # C/C++ (includes clangd)
pip install python-lsp-server             # Python
npm install -g typescript-language-server # TypeScript/JavaScript  
rustup component add rust-analyzer        # Rust
go install golang.org/x/tools/gopls@latest # Go
```

**Windows (MSYS2):**
```bash
# Plugin support
pacman -S mingw-w64-x86_64-lua

# LSP support
pacman -S mingw-w64-x86_64-json-c

# Language servers (use respective package managers)
```

## Advanced Features

### Plugin Development Tips

1. **Error Handling**: Always validate parameters
2. **Performance**: Avoid heavy operations in hooks
3. **Memory**: Lua GC handles most cleanup
4. **State**: Use global variables for persistent state

### Completion Customization

The completion system can be extended through plugins:

```lua
-- Add custom completions
function on_load()
    -- This would require extending the API
    xcodex.add_completion_source("custom", get_custom_completions)
end
```

### TUI Enhancements

- **Status Bar**: Shows plugin count and completion status
- **Modal Indicators**: Clear mode display
- **Theme Integration**: Completion popup respects themes
- **Responsive Layout**: Adapts to terminal resizing

## Troubleshooting

### Common Issues

1. **Lua Not Found**: Install lua development packages
2. **Plugin Errors**: Check plugin syntax and API usage
3. **Completion Not Working**: Ensure `XCODEX_ENABLE_COMPLETION` is defined
4. **Performance**: Limit complex operations in hooks

### Debug Mode

Enable debug output by setting environment variable:
```bash
export XCODEX_DEBUG=1
```

## Future Enhancements

Planned features for upcoming versions:

- � **Enhanced LSP Features**: Signature help, code actions, refactoring
- 📦 **Package Manager**: Built-in plugin package manager with remote repositories
- 🔍 **Fuzzy Finding**: File and symbol search with LSP integration
- 🎯 **Advanced Code Navigation**: Call hierarchy, type hierarchy
- 🔄 **Git Integration**: Version control features with LSP-aware diffs
- 📊 **Project Management**: Workspace and project support with LSP workspaces
- 🌐 **Remote LSP**: Support for remote language servers
- 🧪 **Debug Adapter Protocol**: Integrated debugging support
- 🔌 **Plugin Marketplace**: Online plugin repository and marketplace
- ⚡ **Performance Optimization**: Async LSP communication and caching

## Current Status

### ✅ Implemented (v0.3.2)
- LSP client with JSON-RPC communication
- Auto-starting LSP servers for file types
- Plugin auto-loading from multiple directories
- Built-in fallback plugins
- Multi-language LSP server support
- Basic LSP document synchronization

### 🚧 In Progress
- LSP completion integration
- Enhanced error diagnostics display
- Hover information support
- Go-to-definition functionality

### 📋 Planned
- Advanced LSP features (formatting, code actions)
- Plugin marketplace integration
- Remote development support

---

The plugin system transforms XCodex from a simple text editor into a powerful, extensible development environment while maintaining its lightweight and fast characteristics.
