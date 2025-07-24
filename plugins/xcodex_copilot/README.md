# XCodex Copilot Setup and Usage Guide

## Overview

XCodex Copilot is a modular, non-blocking AI assistant plugin that integrates with your XCodex editor to provide intelligent code suggestions and agentic programming assistance.

## Features

- **Inline Code Completion**: Context-aware code suggestions as you type
- **Agentic Assistance**: Complex tasks like refactoring, test generation, and code explanation
- **Interactive Chat**: Toggle a chat window to have conversations with the AI about your code
  - **Command-based Chat**: Send messages via commands for quick questions
  - **Interactive Window**: GitHub Copilot-style window where you can type directly
- **Non-blocking Operations**: All API calls are asynchronous to prevent UI freezes
- **Modular Architecture**: Clean separation of concerns across multiple Lua modules
- **Configurable**: Extensive customization options for behavior and UI
- **Multi-language Support**: Works with C, C++, Python, JavaScript, Rust, Go, and more

## Installation

1. **Copy Plugin Files**: The plugin should already be installed in your `XShell/plugins/` directory:
   ```
   XShell/plugins/
   ├── xcodex_copilot.lua              # Main plugin file
   └── xcodex_copilot/                 # Plugin modules
       ├── init.lua                    # Main orchestrator
       ├── config.lua                  # Configuration management
       ├── context.lua                 # Editor context extraction
       ├── llm_client.lua             # Gemini API client
       ├── ui.lua                     # TUI management
       ├── chat.lua                   # Interactive chat system
       └── interactive_chat.lua       # Real-time chat window interface
   ```

2. **Set API Key**: Configure your Gemini API key:
   ```bash
   # Option 1: Environment variable (recommended)
   export GEMINI_API_KEY="your_api_key_here"
   
   # Option 2: Will be configured in Lua
   ```

3. **Get Gemini API Key**:
   - Visit [Google AI Studio](https://makersuite.google.com/app/apikey)
   - Create a new API key
   - Copy the key for use with the plugin

## Quick Start

1. **Load XCodex** with the plugin enabled
2. **Initialize the plugin**:
   ```lua
   :lua copilot_setup()
   ```
3. **Try inline suggestions**:
   ```lua
   :lua copilot_suggest_inline
   ```
4. **Try agent mode**:
   ```lua
   :lua copilot_suggest_agent
   ```

5. **Try interactive chat**:
   ```lua
   :lua copilot_chat_toggle
   :lua copilot_chat_send("Can you explain this function?")
   ```

6. **Try the interactive chat window**:
   ```lua
   :lua copilot_chat_window_toggle
   -- Then type directly in the window that opens
   ```

## Configuration

### Basic Setup

```lua
-- Minimal setup with API key
copilot_setup({
    api_key = "your_api_key_here"  -- or use environment variable
})

-- Custom configuration
copilot_setup({
    api_key = os.getenv("GEMINI_API_KEY"),
    gemini_model = "gemini-2.0-flash-exp",
    auto_suggest = true,
    suggestion_delay = 500,
    temperature = 0.7,
    max_tokens = 8192,
    ui_floating_width = 80,
    ui_floating_height = 20
})
```

### Configuration Presets

```lua
-- Conservative mode (manual suggestions only)
copilot_apply_preset("conservative")

-- Aggressive mode (frequent auto-suggestions)
copilot_apply_preset("aggressive")

-- Balanced mode (default)
copilot_apply_preset("balanced")
```

## Available Commands

### Core Commands

| Command | Description |
|---------|-------------|
| `copilot_setup()` | Initialize the plugin with default or custom config |
| `copilot_suggest_inline` | Get inline code completion |
| `copilot_suggest_agent` | Get general agentic assistance |
| `copilot_accept` | Accept current suggestion |
| `copilot_reject` | Reject current suggestion |
| `copilot_show` | Display current suggestion window |
| `copilot_cancel` | Cancel ongoing request |
| `copilot_status` | Show plugin status |
| `copilot_help` | Display help information |

### Agent Mode Commands

| Command | Description |
|---------|-------------|
| `copilot_refactor` | Get refactoring suggestions |
| `copilot_test` | Generate unit tests |
| `copilot_explain` | Explain current code |
| `copilot_debug` | Debug assistance |
| `copilot_optimize` | Performance optimization suggestions |

### Chat Commands

| Command | Description |
|---------|-------------|
| `copilot_chat_toggle` | Open/close the interactive chat window |
| `copilot_chat_send "message"` | Send a message to the AI chat |
| `copilot_chat_show` | Display current chat conversation |
| `copilot_chat_clear` | Clear chat history |
| `copilot_chat_quick command` | Use predefined quick commands |
| `copilot_chat_export` | Export chat conversation to text |

### Interactive Chat Window Commands

| Command | Description |
|---------|-------------|
| `copilot_chat_window_toggle` | Open/close GitHub Copilot-style interactive window |
| `copilot_chat_window_send "message"` | Send message in interactive window |
| `copilot_chat_window_display` | Show the interactive chat window |
| `copilot_chat_window_clear` | Clear interactive chat history |
| `copilot_chat_window_export` | Export interactive chat conversation |

### Interactive Window Controls

When the interactive chat window is open:
- **Type normally**: Characters appear in the input area
- **Enter**: Send your message to the AI
- **Escape**: Close the chat window
- **Backspace**: Delete characters
- **Arrow Keys**: Navigate cursor (Left/Right) and history (Up/Down)
- **Real-time Display**: See conversation history and responses instantly

### Quick Chat Commands

| Quick Command | Equivalent Message |
|---------------|-------------------|
| `explain` | "Can you explain what this code does?" |
| `optimize` | "How can I optimize this code for better performance?" |
| `debug` | "I'm having an issue with this code. Can you help me debug it?" |
| `refactor` | "Can you suggest ways to refactor this code to make it cleaner?" |
| `test` | "What tests should I write for this code?" |
| `security` | "Are there any security concerns with this code?" |
| `style` | "How can I improve the coding style and readability?" |
| `docs` | "Can you help me write documentation for this code?" |

## Usage Examples

### Inline Code Completion

1. **Position cursor** where you want suggestions
2. **Run**: `:lua copilot_suggest_inline`
3. **Wait** for the suggestion to appear
4. **Accept** with `:lua copilot_accept` or **reject** with `:lua copilot_reject`

### Code Refactoring

1. **Position cursor** in the code you want to refactor
2. **Run**: `:lua copilot_refactor`
3. **Review** the suggestions in the floating window
4. **Accept** parts you want to use

### Test Generation

1. **Open file** with functions you want to test
2. **Run**: `:lua copilot_test`
3. **Review** generated test cases
4. **Copy** useful tests to your test files

### Code Explanation

1. **Position cursor** in complex code
2. **Run**: `:lua copilot_explain`
3. **Read** the detailed explanation
4. **Use insights** to improve your understanding

### Interactive Chat

1. **Toggle chat window**: `:lua copilot_chat_toggle`
2. **Send a message**: `:lua copilot_chat_send("How can I improve this function?")`
3. **View conversation**: `:lua copilot_chat_show`
4. **Use quick commands**: `:lua copilot_chat_quick("debug")`

### Interactive Chat Window (GitHub Copilot Style)

1. **Open interactive window**: `:lua copilot_chat_window_toggle`
2. **Type directly**: Start typing your message in the window
3. **Send message**: Press Enter to send
4. **View real-time**: See AI responses appear instantly
5. **Navigate**: Use arrow keys to move cursor and browse history
6. **Close**: Press Escape to close the window

#### Interactive Window Features:
- **Real-time typing**: Type directly in the chat window
- **Live conversation**: See messages and responses as they happen
- **Cursor navigation**: Move cursor with arrow keys
- **Message history**: Browse previous messages with up/down arrows
- **Visual feedback**: Clear indicators for user vs AI messages
- **Instant responses**: AI responses appear in real-time

#### Example Interactive Session:
```lua
-- Open the interactive chat window
:lua copilot_chat_window_toggle

-- Now you can type directly:
-- User types: "What does this function do?"
-- [Press Enter]
-- AI response appears immediately in the window

-- Continue the conversation by typing more:
-- User types: "How can I optimize it?"
-- [Press Enter]
-- AI provides optimization suggestions

-- Use Escape to close when done
```

#### Example Chat Session:
```lua
-- Open chat
:lua copilot_chat_toggle

-- Ask about the current code
:lua copilot_chat_send("What does this function do and how can I make it more efficient?")

-- Follow up questions
:lua copilot_chat_send("Can you show me an example of the optimization you mentioned?")

-- Use quick commands
:lua copilot_chat_quick("security")  -- Asks about security concerns

-- View the conversation
:lua copilot_chat_show

-- Clear history when done
:lua copilot_chat_clear
```

## Keybindings (Optional)

You can set up keybindings in your XCodex configuration:

```lua
-- Example keybindings (adapt to XCodex's keybinding system)
-- These would need to be implemented in XCodex's input handling

-- <C-s> for inline suggestions
-- <leader>ag for agent mode
-- <leader>ch for chat toggle
-- <leader>ci for interactive chat window
-- <Tab> to accept suggestions
-- <Esc> to reject suggestions / close chat window
```

## Advanced Configuration

### Custom Prompts

```lua
local config = require("xcodex_copilot.config")

-- Customize inline completion prompts
config.prompts.inline = function(ctx)
    return "Custom prompt for inline completion: " .. ctx.before_cursor
end

-- Customize agent prompts
config.prompts.agent = function(ctx, task_type)
    return "Custom agent prompt for " .. task_type .. ": " .. ctx.full_content
end
```

### API Configuration

```lua
copilot_setup({
    base_url = "https://generativelanguage.googleapis.com/v1beta/models",
    gemini_model = "gemini-2.0-flash-exp",
    request_timeout = 30,
    temperature = 0.7,
    max_tokens = 8192,
    top_p = 0.8,
    top_k = 40
})
```

### UI Customization

```lua
copilot_setup({
    ui_floating_width = 100,
    ui_floating_height = 25,
    ui_floating_border = "rounded",
    ui_floating_highlight = "CopilotSuggestion"
})
```

## Troubleshooting

### Common Issues

1. **API Key Not Set**:
   ```
   Error: API key not configured
   ```
   **Solution**: Set `GEMINI_API_KEY` environment variable or configure in setup

2. **Network Errors**:
   ```
   Error: Network request failed
   ```
   **Solutions**:
   - Check internet connection
   - Verify API key is valid
   - Check if curl is installed

3. **Empty Responses**:
   ```
   Warning: AI returned empty response
   ```
   **Solutions**:
   - Try different prompts
   - Check if file has enough context
   - Verify model availability

4. **Plugin Not Loading**:
   ```
   Error: Module not found
   ```
   **Solutions**:
   - Ensure all plugin files are in correct locations
   - Check XCodex plugin loading system
   - Verify Lua path configuration

### Debug Commands

```lua
-- Check plugin status
copilot_status()

-- Reset plugin state
copilot_reset()

-- View detailed error information
-- (Check XCodex logs or error output)
```

### Performance Tips

1. **Use appropriate delays**: Don't set `suggestion_delay` too low
2. **Limit context size**: Large files may cause slow responses
3. **Use specific prompts**: More specific requests get better responses
4. **Cancel unnecessary requests**: Use `copilot_cancel` if needed

## File Structure

The plugin is organized into focused modules:

- **`init.lua`**: Main orchestrator, handles user commands and coordinates modules
- **`config.lua`**: Configuration management and prompt templates
- **`context.lua`**: Extracts editor context and analyzes code structure
- **`llm_client.lua`**: Handles Gemini API communication with retry logic
- **`ui.lua`**: Manages TUI elements and user feedback
- **`chat.lua`**: Interactive chat system with conversation history
- **`interactive_chat.lua`**: Real-time interactive chat window (GitHub Copilot style)

## API Compatibility

The plugin is designed to work with XCodex's Lua API:

- `xcodex.get_current_line()`
- `xcodex.get_cursor_pos()`
- `xcodex.set_cursor_pos(line, col)`
- `xcodex.insert_text(text)`
- `xcodex.get_file_content()`
- `xcodex.set_status_message(msg)`
- `xcodex.get_filename()`
- `xcodex.get_mode()`

## Contributing

To extend the plugin:

1. **Add new agent modes** in `config.lua` prompts
2. **Enhance context extraction** in `context.lua`
3. **Improve UI elements** in `ui.lua`
4. **Add new API providers** in `llm_client.lua`

## Security Notes

- **Never commit API keys** to version control
- **Use environment variables** for sensitive configuration
- **Review AI suggestions** before accepting them
- **Be cautious with auto-suggest** in sensitive codebases

## Support

For issues or feature requests:
1. Check this documentation
2. Use `copilot_help` for quick reference
3. Check XCodex plugin documentation
4. Review error messages in status line
