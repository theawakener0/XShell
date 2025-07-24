-- XCodex Copilot Interactive Chat Window
-- Provides a GitHub Copilot-style interactive chat interface

local M = {}

-- Chat window state
local chat_window_state = {
    active = false,
    buffer_content = {},
    input_line = "",
    cursor_pos = 0,
    scroll_offset = 0,
    window_height = 15,
    window_width = 80,
    input_height = 3,
    chat_history_height = 12,
    prompt_prefix = "> ",
    mode = "input", -- "input" or "display"
    thinking = false
}

-- Chat conversation state
local conversation = {
    messages = {},
    current_typing = "",
    max_messages = 50
}

-- Initialize the interactive chat window
function M.init()
    chat_window_state.buffer_content = {}
    conversation.messages = {}
    chat_window_state.active = false
    return true
end

-- Toggle the interactive chat window
function M.toggle()
    if chat_window_state.active then
        M.close()
    else
        M.open()
    end
end

-- Open the interactive chat window
function M.open()
    if chat_window_state.active then
        return false
    end
    
    chat_window_state.active = true
    chat_window_state.mode = "input"
    chat_window_state.input_line = ""
    chat_window_state.cursor_pos = 0
    
    -- Initialize the display
    M.refresh_display()
    
    -- Show instructions
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat opened - Type your message and press Enter", "info")
    
    -- Start input capture mode
    M.start_input_capture()
    
    return true
end

-- Close the interactive chat window
function M.close()
    if not chat_window_state.active then
        return false
    end
    
    chat_window_state.active = false
    M.stop_input_capture()
    
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat closed", "info")
    
    return true
end

-- Start capturing input for the chat window
function M.start_input_capture()
    -- Store original XCodex mode
    chat_window_state.original_mode = xcodex.get_mode()
    
    -- Set up key event handlers (this would need to be integrated with XCodex's input system)
    -- For now, we'll simulate this with a command-based approach
    xcodex.set_status_message("CHAT MODE - Type 'chat_send' or 'chat_exit'")
end

-- Stop capturing input
function M.stop_input_capture()
    -- Restore original XCodex mode if needed
    xcodex.set_status_message("")
end

-- Handle key input for the chat window
function M.handle_key_input(key)
    if not chat_window_state.active then
        return false
    end
    
    if key == "Enter" or key == "\n" or key == "\r" then
        M.send_current_message()
        return true
    elseif key == "Escape" then
        M.close()
        return true
    elseif key == "Backspace" then
        M.handle_backspace()
        return true
    elseif key == "Left" then
        M.move_cursor_left()
        return true
    elseif key == "Right" then
        M.move_cursor_right()
        return true
    elseif key == "Up" then
        M.history_previous()
        return true
    elseif key == "Down" then
        M.history_next()
        return true
    elseif #key == 1 and key:byte() >= 32 and key:byte() <= 126 then
        -- Printable character
        M.insert_character(key)
        return true
    end
    
    return false
end

-- Insert character at cursor position
function M.insert_character(char)
    local before = chat_window_state.input_line:sub(1, chat_window_state.cursor_pos)
    local after = chat_window_state.input_line:sub(chat_window_state.cursor_pos + 1)
    
    chat_window_state.input_line = before .. char .. after
    chat_window_state.cursor_pos = chat_window_state.cursor_pos + 1
    
    M.refresh_display()
end

-- Handle backspace
function M.handle_backspace()
    if chat_window_state.cursor_pos > 0 then
        local before = chat_window_state.input_line:sub(1, chat_window_state.cursor_pos - 1)
        local after = chat_window_state.input_line:sub(chat_window_state.cursor_pos + 1)
        
        chat_window_state.input_line = before .. after
        chat_window_state.cursor_pos = chat_window_state.cursor_pos - 1
        
        M.refresh_display()
    end
end

-- Move cursor left
function M.move_cursor_left()
    if chat_window_state.cursor_pos > 0 then
        chat_window_state.cursor_pos = chat_window_state.cursor_pos - 1
        M.refresh_display()
    end
end

-- Move cursor right
function M.move_cursor_right()
    if chat_window_state.cursor_pos < #chat_window_state.input_line then
        chat_window_state.cursor_pos = chat_window_state.cursor_pos + 1
        M.refresh_display()
    end
end

-- Add message to conversation
function M.add_message(role, content)
    table.insert(conversation.messages, {
        role = role, -- "user" or "assistant"
        content = content,
        timestamp = os.time()
    })
    
    -- Trim conversation if too long
    if #conversation.messages > conversation.max_messages then
        table.remove(conversation.messages, 1)
    end
    
    M.refresh_display()
end

-- Send current message
function M.send_current_message()
    local message = chat_window_state.input_line:match("^%s*(.-)%s*$") -- Trim whitespace
    
    if message == "" then
        return
    end
    
    -- Add user message to conversation
    M.add_message("user", message)
    
    -- Clear input
    chat_window_state.input_line = ""
    chat_window_state.cursor_pos = 0
    chat_window_state.thinking = true
    
    M.refresh_display()
    
    -- Send to AI
    local chat = require("xcodex_copilot.chat")
    chat.send_message(message, function(response, error_msg)
        chat_window_state.thinking = false
        
        if error_msg then
            M.add_message("assistant", "Error: " .. error_msg)
        else
            M.add_message("assistant", response or "No response received")
        end
    end)
end

-- Build display content
function M.build_display_content()
    local content = {}
    local width = chat_window_state.window_width
    
    -- Title bar
    table.insert(content, "┌" .. string.rep("─", width - 2) .. "┐")
    local title = " XCodex Copilot Chat "
    local title_padding = width - 2 - #title
    local left_pad = math.floor(title_padding / 2)
    local right_pad = title_padding - left_pad
    table.insert(content, "│" .. string.rep(" ", left_pad) .. title .. string.rep(" ", right_pad) .. "│")
    table.insert(content, "├" .. string.rep("─", width - 2) .. "┤")
    
    -- Chat history area
    local history_start = #content + 1
    local available_height = chat_window_state.chat_history_height
    
    -- Get messages to display
    local display_messages = {}
    for i = math.max(1, #conversation.messages - 10), #conversation.messages do
        if conversation.messages[i] then
            table.insert(display_messages, conversation.messages[i])
        end
    end
    
    -- Format messages
    local formatted_lines = {}
    for _, msg in ipairs(display_messages) do
        local prefix = msg.role == "user" and "You: " or "Copilot: "
        local timestamp = os.date("%H:%M", msg.timestamp)
        
        -- Word wrap the message
        local wrapped = M.word_wrap(prefix .. msg.content, width - 4)
        for i, line in ipairs(wrapped) do
            if i == 1 then
                table.insert(formatted_lines, string.format("[%s] %s", timestamp, line))
            else
                table.insert(formatted_lines, "       " .. line)
            end
        end
        table.insert(formatted_lines, "") -- Empty line between messages
    end
    
    -- Add thinking indicator
    if chat_window_state.thinking then
        table.insert(formatted_lines, "[" .. os.date("%H:%M") .. "] Copilot: 🤔 Thinking...")
    end
    
    -- Fill chat history area
    local start_line = math.max(1, #formatted_lines - available_height + 1)
    for i = 1, available_height do
        local line_index = start_line + i - 1
        local line_text = ""
        
        if line_index <= #formatted_lines then
            line_text = formatted_lines[line_index] or ""
        end
        
        -- Truncate if too long
        if #line_text > width - 4 then
            line_text = line_text:sub(1, width - 7) .. "..."
        end
        
        -- Pad to fill width
        line_text = line_text .. string.rep(" ", width - 4 - #line_text)
        
        table.insert(content, "│ " .. line_text .. " │")
    end
    
    -- Separator
    table.insert(content, "├" .. string.rep("─", width - 2) .. "┤")
    
    -- Input area
    local input_prompt = chat_window_state.prompt_prefix .. chat_window_state.input_line
    local cursor_marker = ""
    
    -- Add cursor indicator
    if chat_window_state.cursor_pos <= #chat_window_state.input_line then
        local before_cursor = input_prompt:sub(1, #chat_window_state.prompt_prefix + chat_window_state.cursor_pos)
        local at_cursor = input_prompt:sub(#chat_window_state.prompt_prefix + chat_window_state.cursor_pos + 1, #chat_window_state.prompt_prefix + chat_window_state.cursor_pos + 1)
        local after_cursor = input_prompt:sub(#chat_window_state.prompt_prefix + chat_window_state.cursor_pos + 2)
        
        if at_cursor == "" then
            input_prompt = before_cursor .. "█" .. after_cursor
        else
            input_prompt = before_cursor .. "█" .. after_cursor
        end
    end
    
    -- Truncate input if too long
    if #input_prompt > width - 4 then
        input_prompt = "..." .. input_prompt:sub(-(width - 7))
    end
    
    -- Pad input line
    input_prompt = input_prompt .. string.rep(" ", width - 4 - #input_prompt)
    table.insert(content, "│ " .. input_prompt .. " │")
    
    -- Help line
    local help_text = " [Enter] Send [Esc] Close [↑↓] History "
    help_text = help_text .. string.rep(" ", width - 4 - #help_text)
    table.insert(content, "│ " .. help_text .. " │")
    
    -- Bottom border
    table.insert(content, "└" .. string.rep("─", width - 2) .. "┘")
    
    return content
end

-- Word wrap text to fit within specified width
function M.word_wrap(text, max_width)
    local lines = {}
    local current_line = ""
    
    for word in text:gmatch("%S+") do
        if #current_line + #word + 1 <= max_width then
            if current_line == "" then
                current_line = word
            else
                current_line = current_line .. " " .. word
            end
        else
            if current_line ~= "" then
                table.insert(lines, current_line)
            end
            current_line = word
        end
    end
    
    if current_line ~= "" then
        table.insert(lines, current_line)
    end
    
    return lines
end

-- Refresh the display
function M.refresh_display()
    if not chat_window_state.active then
        return
    end
    
    chat_window_state.buffer_content = M.build_display_content()
    
    -- Update status message with current state
    local status = "CHAT MODE"
    if chat_window_state.thinking then
        status = status .. " - AI Thinking..."
    else
        status = status .. " - Type your message"
    end
    
    xcodex.set_status_message(status)
end

-- Display the chat window (for systems without real window support)
function M.display()
    if not chat_window_state.active then
        return false
    end
    
    M.refresh_display()
    
    -- Print the window content
    print("\n" .. string.rep("=", 50))
    for _, line in ipairs(chat_window_state.buffer_content) do
        print(line)
    end
    print(string.rep("=", 50))
    
    return true
end

-- Command-based interface for XCodex integration
function M.send_text(text)
    if not chat_window_state.active then
        M.open()
    end
    
    chat_window_state.input_line = text
    chat_window_state.cursor_pos = #text
    M.send_current_message()
end

-- Quick send interface
function M.quick_send(message)
    if not chat_window_state.active then
        M.open()
    end
    
    M.add_message("user", message)
    chat_window_state.thinking = true
    M.refresh_display()
    
    local chat = require("xcodex_copilot.chat")
    chat.send_message(message, function(response, error_msg)
        chat_window_state.thinking = false
        
        if error_msg then
            M.add_message("assistant", "Error: " .. error_msg)
        else
            M.add_message("assistant", response or "No response received")
        end
    end)
end

-- Get window state
function M.is_active()
    return chat_window_state.active
end

-- Clear conversation
function M.clear_conversation()
    conversation.messages = {}
    M.refresh_display()
    
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat conversation cleared", "info")
end

-- Export conversation
function M.export_conversation()
    if #conversation.messages == 0 then
        return "No conversation to export"
    end
    
    local export_text = "XCodex Copilot Interactive Chat Export\n"
    export_text = export_text .. "Date: " .. os.date("%Y-%m-%d %H:%M:%S") .. "\n"
    export_text = export_text .. string.rep("=", 50) .. "\n\n"
    
    for _, msg in ipairs(conversation.messages) do
        local timestamp = os.date("%H:%M:%S", msg.timestamp)
        local sender = msg.role == "user" and "You" or "Copilot"
        
        export_text = export_text .. string.format("[%s] %s:\n%s\n\n", timestamp, sender, msg.content)
    end
    
    return export_text
end

-- History navigation (placeholder)
function M.history_previous()
    -- Could implement command history here
    M.refresh_display()
end

function M.history_next()
    -- Could implement command history here
    M.refresh_display()
end

return M
