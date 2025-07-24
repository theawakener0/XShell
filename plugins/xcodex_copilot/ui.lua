-- XCodex Copilot UI Module
-- Manages TUI elements for displaying suggestions and feedback

local M = {}

-- UI state management
local ui_state = {
    suggestion_window = nil,
    loading_indicator = false,
    suggestion_visible = false,
    current_suggestion = nil,
    chat_window = nil,
    chat_visible = false,
    active_window = nil -- "suggestion" or "chat"
}

-- Display notification message to user
function M.notify(message, level)
    level = level or "info"
    
    local prefix = ""
    if level == "error" then
        prefix = "ERROR: "
    elseif level == "warning" then
        prefix = "WARNING: "
    elseif level == "success" then
        prefix = "SUCCESS: "
    end
    
    xcodex.set_status_message(prefix .. message)
end

-- Show loading indicator
function M.show_loading_indicator()
    if ui_state.loading_indicator then
        return -- Already showing
    end
    
    ui_state.loading_indicator = true
    
    -- Start animation sequence
    local frames = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"}
    local frame_index = 1
    
    local function update_spinner()
        if ui_state.loading_indicator then
            local frame = frames[frame_index]
            xcodex.set_status_message(frame .. " Thinking...")
            frame_index = frame_index % #frames + 1
            
            -- Note: In a real implementation, this would use a timer
            -- For now, we'll just show a static message
        end
    end
    
    -- Show initial loading message
    xcodex.set_status_message("🤔 AI thinking...")
end

-- Hide loading indicator
function M.hide_loading_indicator()
    ui_state.loading_indicator = false
    xcodex.set_status_message("") -- Clear status message
end

-- Create floating window content (simulated for XCodex)
function M.create_floating_content(text, width, height)
    width = width or 80
    height = height or 20
    
    -- Split text into lines
    local lines = {}
    for line in text:gmatch("[^\r\n]*") do
        table.insert(lines, line)
    end
    
    -- Word wrap lines that are too long
    local wrapped_lines = {}
    for _, line in ipairs(lines) do
        if #line <= width - 4 then -- Account for border
            table.insert(wrapped_lines, line)
        else
            -- Simple word wrapping
            local current_line = ""
            for word in line:gmatch("%S+") do
                if #current_line + #word + 1 <= width - 4 then
                    current_line = current_line .. (current_line == "" and "" or " ") .. word
                else
                    if current_line ~= "" then
                        table.insert(wrapped_lines, current_line)
                    end
                    current_line = word
                end
            end
            if current_line ~= "" then
                table.insert(wrapped_lines, current_line)
            end
        end
    end
    
    -- Truncate if too many lines
    if #wrapped_lines > height - 2 then -- Account for border
        wrapped_lines = {table.unpack(wrapped_lines, 1, height - 2)}
        table.insert(wrapped_lines, "... (truncated)")
    end
    
    return wrapped_lines
end

-- Display floating suggestion window
function M.display_floating_suggestion(text)
    if ui_state.suggestion_visible then
        M.close_suggestion_window()
    end
    
    local config = require("xcodex_copilot.config").config
    local width = config.ui_floating_width or 80
    local height = config.ui_floating_height or 20
    
    -- Store suggestion for later actions
    ui_state.current_suggestion = text
    ui_state.suggestion_visible = true
    ui_state.active_window = "suggestion"
    
    -- Create content
    local content_lines = M.create_floating_content(text, width, height)
    
    -- Create a border
    local border_char = "─"
    local corner_char = "┌┐└┘"
    local vertical_char = "│"
    
    -- Build the floating window content
    local window_content = {}
    
    -- Top border
    table.insert(window_content, "┌" .. string.rep(border_char, width - 2) .. "┐")
    
    -- Content lines
    for _, line in ipairs(content_lines) do
        local padded_line = line .. string.rep(" ", width - 2 - #line)
        table.insert(window_content, vertical_char .. padded_line .. vertical_char)
    end
    
    -- Fill remaining height
    local empty_line = vertical_char .. string.rep(" ", width - 2) .. vertical_char
    for i = #content_lines + 1, height - 2 do
        table.insert(window_content, empty_line)
    end
    
    -- Bottom border with controls
    local controls = " [Tab] Accept [Esc] Dismiss "
    local bottom_padding = width - 2 - #controls
    if bottom_padding > 0 then
        controls = controls .. string.rep(" ", bottom_padding)
    end
    table.insert(window_content, "└" .. controls:sub(1, width - 2) .. "┘")
    
    -- Since XCodex doesn't have floating windows, we'll simulate by showing content
    -- In a status area or by inserting temporary text
    M.notify("AI Suggestion ready - Type 'copilot_show' to view", "info")
    
    -- Store the window content for later display
    ui_state.suggestion_window = {
        content = window_content,
        text = text,
        width = width,
        height = height
    }
    
    return true
end

-- Display chat window
function M.display_chat_window(text, title)
    title = title or "Copilot Chat"
    
    if ui_state.chat_visible then
        M.close_chat_window()
    end
    
    local config = require("xcodex_copilot.config").config
    local width = config.ui_floating_width or 80
    local height = config.ui_floating_height or 20
    
    -- Store chat content
    ui_state.chat_visible = true
    ui_state.active_window = "chat"
    
    -- Create content
    local content_lines = M.create_floating_content(text, width, height)
    
    -- Build the chat window content
    local window_content = {}
    
    -- Top border with title
    local title_text = " " .. title .. " "
    local title_padding = width - 2 - #title_text
    local left_padding = math.floor(title_padding / 2)
    local right_padding = title_padding - left_padding
    
    table.insert(window_content, "┌" .. string.rep("─", left_padding) .. title_text .. string.rep("─", right_padding) .. "┐")
    
    -- Content lines
    for _, line in ipairs(content_lines) do
        local padded_line = line .. string.rep(" ", width - 2 - #line)
        table.insert(window_content, "│" .. padded_line .. "│")
    end
    
    -- Fill remaining height
    local empty_line = "│" .. string.rep(" ", width - 2) .. "│"
    for i = #content_lines + 1, height - 2 do
        table.insert(window_content, empty_line)
    end
    
    -- Bottom border with chat controls
    local controls = " [copilot_chat_send \"msg\"] Send [Esc] Close "
    local bottom_padding = width - 2 - #controls
    if bottom_padding > 0 then
        controls = controls .. string.rep(" ", bottom_padding)
    end
    table.insert(window_content, "└" .. controls:sub(1, width - 2) .. "┘")
    
    M.notify("Chat window ready - Type 'copilot_chat_show' to view", "info")
    
    -- Store the window content for later display
    ui_state.chat_window = {
        content = window_content,
        text = text,
        width = width,
        height = height,
        title = title
    }
    
    return true
end

-- Display inline suggestion (as virtual text or overlay)
function M.display_inline_suggestion(text)
    if not text or text == "" then
        return false
    end
    
    -- Store current suggestion
    ui_state.current_suggestion = text
    ui_state.suggestion_visible = true
    
    -- For XCodex, we'll show inline suggestions as status messages
    -- or insert them as comments that can be accepted/rejected
    
    -- Simple approach: show as status with accept/reject options
    local preview = text:gsub("\n.*", "") -- Show only first line
    if #preview > 50 then
        preview = preview:sub(1, 47) .. "..."
    end
    
    M.notify("Suggestion: " .. preview .. " [Tab to accept]", "info")
    
    return true
end

-- Show the stored suggestion window content
function M.show_suggestion_window()
    if not ui_state.suggestion_window then
        M.notify("No suggestion available", "warning")
        return false
    end
    
    local window = ui_state.suggestion_window
    
    -- Display window content line by line
    M.notify("=== AI Suggestion ===", "info")
    for _, line in ipairs(window.content) do
        print(line) -- This would need to be replaced with proper window display
    end
    
    M.notify("Use 'copilot_accept' or 'copilot_reject'", "info")
    return true
end

-- Show the stored chat window content
function M.show_chat_window()
    if not ui_state.chat_window then
        M.notify("No chat window available", "warning")
        return false
    end
    
    local window = ui_state.chat_window
    
    -- Display window content line by line
    M.notify("=== " .. (window.title or "Copilot Chat") .. " ===", "info")
    for _, line in ipairs(window.content) do
        print(line) -- This would need to be replaced with proper window display
    end
    
    M.notify("Use 'copilot_chat_send \"message\"' to chat", "info")
    return true
end

-- Close suggestion window
function M.close_suggestion_window()
    ui_state.suggestion_window = nil
    ui_state.suggestion_visible = false
    ui_state.current_suggestion = nil
    
    if ui_state.active_window == "suggestion" then
        ui_state.active_window = nil
    end
    
    -- Clear any UI elements
    M.notify("", "info") -- Clear status message
    
    return true
end

-- Close chat window
function M.close_chat_window()
    ui_state.chat_window = nil
    ui_state.chat_visible = false
    
    if ui_state.active_window == "chat" then
        ui_state.active_window = nil
    end
    
    -- Clear any UI elements
    M.notify("Chat window closed", "info")
    
    return true
end

-- Accept current suggestion
function M.accept_suggestion()
    if not ui_state.current_suggestion then
        M.notify("No suggestion to accept", "warning")
        return false
    end
    
    local suggestion = ui_state.current_suggestion
    
    -- Insert the suggestion at cursor position
    xcodex.insert_text(suggestion)
    
    -- Close suggestion UI
    M.close_suggestion_window()
    
    M.notify("Suggestion accepted", "success")
    return true
end

-- Reject current suggestion
function M.reject_suggestion()
    if not ui_state.current_suggestion then
        M.notify("No suggestion to reject", "warning")
        return false
    end
    
    -- Close suggestion UI
    M.close_suggestion_window()
    
    M.notify("Suggestion rejected", "info")
    return true
end

-- Show help information
function M.show_help()
    local help_text = [[
XCodex Copilot Help:

Commands:
  copilot_suggest_inline  - Get inline code suggestion
  copilot_suggest_agent   - Get agentic assistance
  copilot_show           - Show current suggestion
  copilot_accept         - Accept current suggestion  
  copilot_reject         - Reject current suggestion
  copilot_help           - Show this help

Chat Commands:
  copilot_chat_toggle    - Open/close chat window
  copilot_chat_send "msg" - Send message to AI chat
  copilot_chat_show      - Show current chat window
  copilot_chat_clear     - Clear chat history
  copilot_chat_quick cmd - Use quick chat command

Interactive Chat Window:
  copilot_chat_window_toggle - Open/close interactive chat window
  copilot_chat_window_send "msg" - Send message in interactive mode
  copilot_chat_window_display - Display the interactive window
  copilot_chat_window_clear - Clear interactive chat history

Quick Chat Commands:
  explain, optimize, debug, refactor, test, security, style, docs

Keybindings (if configured):
  <C-s>      - Inline suggestion
  <leader>ag - Agent mode
  <leader>ch - Toggle chat
  <leader>ci - Toggle interactive chat window
  <Tab>      - Accept suggestion
  <Esc>      - Reject suggestion / Close interactive chat

Agent Modes:
  refactor   - Code refactoring suggestions
  test       - Generate unit tests
  explain    - Explain code functionality
  debug      - Debug assistance
  optimize   - Performance optimization
  general    - General programming help
]]
    
    M.display_floating_suggestion(help_text)
end

-- Progress indicator for long operations
function M.show_progress(message, percentage)
    percentage = percentage or 0
    local bar_width = 20
    local filled = math.floor(bar_width * percentage / 100)
    local empty = bar_width - filled
    
    local progress_bar = "[" .. string.rep("=", filled) .. string.rep("-", empty) .. "]"
    local status = string.format("%s %s %d%%", message, progress_bar, percentage)
    
    xcodex.set_status_message(status)
end

-- Display error with details
function M.show_error(error_message, details)
    M.notify("AI Error: " .. error_message, "error")
    
    if details then
        -- In a more advanced implementation, this could show a detailed error window
        print("Error details: " .. tostring(details))
    end
end

-- Check if suggestion is currently visible
function M.is_suggestion_visible()
    return ui_state.suggestion_visible
end

-- Check if chat window is currently visible
function M.is_chat_visible()
    return ui_state.chat_visible
end

-- Get active window type
function M.get_active_window()
    return ui_state.active_window
end

-- Get current suggestion text
function M.get_current_suggestion()
    return ui_state.current_suggestion
end

-- Clear all UI state
function M.clear_all()
    M.close_suggestion_window()
    M.close_chat_window()
    M.hide_loading_indicator()
    ui_state = {
        suggestion_window = nil,
        loading_indicator = false,
        suggestion_visible = false,
        current_suggestion = nil,
        chat_window = nil,
        chat_visible = false,
        active_window = nil
    }
end

-- Theme support for highlighting
function M.setup_highlights()
    -- In a real implementation, this would set up syntax highlighting
    -- for the suggestion text and UI elements
    M.notify("Copilot highlighting configured", "info")
end

-- Utility function to format code suggestions
function M.format_code_suggestion(text, language)
    if not text then return "" end
    
    -- Basic formatting for different languages
    local formatted = text
    
    -- Remove common AI response prefixes
    formatted = formatted:gsub("^Here's the code:[\r\n]*", "")
    formatted = formatted:gsub("^```[%w]*[\r\n]*", "")
    formatted = formatted:gsub("[\r\n]*```$", "")
    
    -- Trim whitespace
    formatted = formatted:gsub("^%s+", ""):gsub("%s+$", "")
    
    return formatted
end

return M
