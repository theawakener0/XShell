# XCodex Copilot Interactive Chat Window Guide

## Overview

The Interactive Chat Window provides a GitHub Copilot-style interface where you can type directly in a dedicated chat window and have real-time conversations with the AI about your code.

## Key Features

### 🎯 **Real-Time Interaction**
- Type directly in the chat window
- See AI responses appear instantly
- No need for command-line functions
- Visual conversation flow

### ⌨️ **Natural Typing Experience**
- Normal keyboard input
- Cursor movement with arrow keys
- Backspace for corrections
- Enter to send messages

### 💬 **Live Conversation**
- Message history displayed in real-time
- Clear distinction between user and AI messages
- Timestamps for each message
- Automatic scrolling for long conversations

### 🎨 **Visual Interface**
- ASCII art borders and formatting
- Dedicated input area with cursor indicator
- Status indicators (thinking, ready, etc.)
- Help text always visible

## How to Use

### Opening the Interactive Chat

```lua
-- Open the interactive chat window
:lua copilot_chat_window_toggle
```

### Window Layout

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                           XCodex Copilot Chat                               │
├──────────────────────────────────────────────────────────────────────────────┤
│ [14:30] You: What does this function do?                                    │
│                                                                              │
│ [14:30] Copilot: This function appears to be a quicksort implementation... │
│         It takes an array and recursively sorts it using the divide and     │
│         conquer approach. The pivot element is chosen and...                │
│                                                                              │
│ [14:31] You: How can I optimize it?                                         │
│                                                                              │
│ [14:31] Copilot: 🤔 Thinking...                                            │
│                                                                              │
│                                                                              │
├──────────────────────────────────────────────────────────────────────────────┤
│ > Can you suggest better algorithms?█                                       │
│  [Enter] Send [Esc] Close [↑↓] History                                     │
└──────────────────────────────────────────────────────────────────────────────┘
```

### Controls

| Key | Action |
|-----|--------|
| **Type normally** | Characters appear at cursor position |
| **Enter** | Send the current message to AI |
| **Escape** | Close the chat window |
| **Backspace** | Delete character before cursor |
| **Left Arrow** | Move cursor left |
| **Right Arrow** | Move cursor right |
| **Up Arrow** | Browse previous messages (future feature) |
| **Down Arrow** | Browse next messages (future feature) |

## Usage Examples

### Example 1: Code Explanation
```lua
-- Open chat
:lua copilot_chat_window_toggle

-- Type in the window:
"What does this sorting algorithm do?"
[Press Enter]

-- AI responds with detailed explanation
-- Continue typing:
"What's the time complexity?"
[Press Enter]

-- Get complexity analysis
```

### Example 2: Debugging Session
```lua
-- Open chat
:lua copilot_chat_window_toggle

-- Type:
"This code is causing a segmentation fault on line 42"
[Press Enter]

-- AI analyzes and responds
-- Follow up:
"I tried your suggestion but still getting the error"
[Press Enter]

-- Continue troubleshooting...
```

### Example 3: Code Review
```lua
-- Position cursor on function you want reviewed
:lua copilot_chat_window_toggle

-- Type:
"Can you review this function for potential issues?"
[Press Enter]

-- AI provides review
-- Ask follow-ups:
"What about the memory management here?"
[Press Enter]
```

## Advanced Features

### Context Awareness
The chat window is fully aware of:
- Your current cursor position
- The file you're editing
- The programming language
- The code around your cursor
- Previous conversation history

### Message History
- All messages are stored during the session
- Timestamps are automatically added
- Clear visual distinction between user and AI
- Conversation persists until manually cleared

### Export Functionality
```lua
-- Export your conversation
:lua copilot_chat_window_export
```

### Clear History
```lua
-- Clear the conversation
:lua copilot_chat_window_clear
```

## Tips for Effective Use

### 1. **Be Specific**
Instead of: "Fix this"
Try: "This function returns wrong values when the input array is empty"

### 2. **Provide Context**
- Mention what you're trying to achieve
- Describe the expected behavior
- Explain what's going wrong

### 3. **Ask Follow-ups**
The AI remembers your conversation, so build on previous answers:
- "Can you elaborate on that optimization?"
- "What would happen if I used your first suggestion instead?"

### 4. **Use Natural Language**
- Type as if you're talking to a colleague
- Ask questions naturally
- Don't worry about formal syntax

### 5. **Combine with Code Navigation**
- Move your cursor to different parts of your code
- The AI will understand the new context
- Ask about different functions in the same conversation

## Integration with Other Features

### Quick Commands
You can still use quick commands even with the interactive window:
```lua
:lua copilot_chat_quick("optimize")  -- Quick optimization request
```

### Traditional Chat
The interactive window works alongside the traditional chat:
```lua
:lua copilot_chat_send("Quick question about this function")
```

### Code Suggestions
Use the interactive chat alongside inline suggestions:
```lua
:lua copilot_suggest_inline          -- Get code suggestion
:lua copilot_chat_window_toggle      -- Discuss the suggestion
```

## Troubleshooting

### Window Not Responding
If the window seems unresponsive:
1. Press Escape to close
2. Reopen with `:lua copilot_chat_window_toggle`
3. Check that Copilot is initialized: `:lua copilot_status`

### Can't Type in Window
If typing doesn't work:
1. Make sure the window is active
2. Check status message for mode indicator
3. Try closing and reopening the window

### AI Not Responding
If the AI doesn't respond:
1. Check your API key configuration
2. Verify internet connection
3. Look for error messages in status line

### Display Issues
If the window doesn't display properly:
1. Try `:lua copilot_chat_window_display` to refresh
2. Check terminal size and compatibility
3. Clear and reopen the window

## Technical Details

### Implementation
- Built on XCodex's Lua plugin system
- Uses terminal-based UI with ASCII art
- Integrates with existing chat and LLM client modules
- Maintains conversation state during session

### Performance
- Non-blocking AI requests
- Efficient text rendering
- Minimal memory footprint
- Real-time updates

### Compatibility
- Works with all XCodex supported languages
- Terminal-agnostic (works in any terminal that supports XCodex)
- Cross-platform compatible

## Comparison: Interactive Window vs Command Chat

| Feature | Interactive Window | Command Chat |
|---------|-------------------|--------------|
| **Typing** | Direct in window | Command-line functions |
| **Visual** | Real-time display | Floating popups |
| **Speed** | Immediate | Requires commands |
| **History** | Always visible | Must be requested |
| **UX** | Like GitHub Copilot | Like terminal commands |
| **Best For** | Extended conversations | Quick questions |

## Future Enhancements

Planned improvements for the interactive chat window:
- Syntax highlighting for code in messages
- Message editing and deletion
- Conversation search functionality
- Multiple conversation tabs
- Auto-completion for common queries
- Integration with XCodex themes
- Floating window positioning options

The interactive chat window brings the familiar GitHub Copilot experience directly into your XCodex editor, making AI assistance feel natural and integrated into your coding workflow!
