-- XCodex Copilot Chat Module
-- Manages interactive chat conversations with the AI about code

local M = {}

-- Chat state management
local chat_state = {
    window_open = false,
    conversation_history = {},
    current_session_id = nil,
    max_history_length = 50,
    chat_width = 80,
    chat_height = 25
}

-- Chat message types
local MESSAGE_TYPES = {
    USER = "user",
    ASSISTANT = "assistant",
    SYSTEM = "system",
    ERROR = "error"
}

-- Initialize chat system
function M.init()
    chat_state.conversation_history = {}
    chat_state.current_session_id = M.generate_session_id()
    return true
end

-- Generate unique session ID
function M.generate_session_id()
    return "chat_" .. os.time() .. "_" .. math.random(1000, 9999)
end

-- Add message to conversation history
function M.add_message(type, content, metadata)
    metadata = metadata or {}
    
    local message = {
        type = type,
        content = content,
        timestamp = os.time(),
        session_id = chat_state.current_session_id,
        metadata = metadata
    }
    
    table.insert(chat_state.conversation_history, message)
    
    -- Trim history if too long
    if #chat_state.conversation_history > chat_state.max_history_length then
        table.remove(chat_state.conversation_history, 1)
    end
    
    return message
end

-- Get conversation context for AI
function M.get_conversation_context()
    local context_messages = {}
    
    -- Get recent messages (last 10 exchanges)
    local recent_count = math.min(20, #chat_state.conversation_history)
    local start_index = math.max(1, #chat_state.conversation_history - recent_count + 1)
    
    for i = start_index, #chat_state.conversation_history do
        local msg = chat_state.conversation_history[i]
        if msg.type == MESSAGE_TYPES.USER or msg.type == MESSAGE_TYPES.ASSISTANT then
            table.insert(context_messages, {
                role = msg.type == MESSAGE_TYPES.USER and "user" or "assistant",
                content = msg.content
            })
        end
    end
    
    return context_messages
end

-- Build chat prompt with context
function M.build_chat_prompt(user_message, code_context)
    local context = require("xcodex_copilot.context")
    local ctx = code_context or context.get_current_buffer_context()
    
    local system_prompt = string.format([[
You are an intelligent programming assistant integrated into the XCodex code editor. You are having a conversation with a developer about their code.

Current Context:
- File: %s
- Language: %s
- Total lines: %d
- Cursor at: Line %d, Column %d

Code around cursor:
```%s
%s
```

Guidelines:
1. Be conversational and helpful
2. Reference the code context when relevant
3. Provide specific, actionable advice
4. Ask clarifying questions when needed
5. Keep responses concise but thorough
6. Use code examples when helpful
7. Remember our conversation history

Current user message: %s]], 
        ctx.filename or "untitled",
        ctx.filetype or "text",
        ctx.total_lines or 0,
        ctx.cursor_line or 1,
        ctx.cursor_col or 1,
        ctx.filetype or "",
        ctx.full_content or ""
    )
    
    -- Build conversation history for context
    local conversation_context = M.get_conversation_context()
    local full_prompt = system_prompt
    
    if #conversation_context > 0 then
        full_prompt = full_prompt .. "\n\nConversation History:\n"
        for _, msg in ipairs(conversation_context) do
            full_prompt = full_prompt .. string.format("%s: %s\n", 
                msg.role == "user" and "Developer" or "Assistant", 
                msg.content)
        end
    end
    
    full_prompt = full_prompt .. "\n\nPlease respond to: " .. user_message
    
    return full_prompt
end

-- Send chat message and get response
function M.send_message(message, callback)
    if not message or message:match("^%s*$") then
        callback(nil, "Empty message")
        return
    end
    
    -- Add user message to history
    M.add_message(MESSAGE_TYPES.USER, message)
    
    -- Get current code context
    local context = require("xcodex_copilot.context")
    local code_context = context.get_current_buffer_context()
    
    -- Build chat prompt
    local prompt = M.build_chat_prompt(message, code_context)
    
    -- Get configuration
    local config = require("xcodex_copilot.config")
    local llm_client = require("xcodex_copilot.llm_client")
    
    -- Send to LLM
    llm_client.request_completion(
        prompt,
        config.config,
        function(response, error_msg)
            if error_msg then
                M.add_message(MESSAGE_TYPES.ERROR, "Error: " .. error_msg)
                callback(nil, error_msg)
                return
            end
            
            if not response or response == "" then
                M.add_message(MESSAGE_TYPES.ERROR, "Empty response from AI")
                callback(nil, "Empty response")
                return
            end
            
            -- Add assistant response to history
            M.add_message(MESSAGE_TYPES.ASSISTANT, response)
            callback(response, nil)
        end
    )
end

-- Toggle chat window
function M.toggle_window()
    if chat_state.window_open then
        M.close_window()
    else
        M.open_window()
    end
end

-- Open chat window
function M.open_window()
    if chat_state.window_open then
        return false, "Chat window already open"
    end
    
    chat_state.window_open = true
    
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat window opened - use copilot_chat_send \"message\" to chat", "info")
    
    -- Show recent conversation if any
    if #chat_state.conversation_history > 0 then
        M.display_conversation()
    else
        ui.notify("Welcome to Copilot Chat! Ask me anything about your code.", "info")
    end
    
    return true
end

-- Close chat window
function M.close_window()
    if not chat_state.window_open then
        return false, "Chat window not open"
    end
    
    chat_state.window_open = false
    
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat window closed", "info")
    
    return true
end

-- Display conversation history
function M.display_conversation()
    local ui = require("xcodex_copilot.ui")
    
    if #chat_state.conversation_history == 0 then
        ui.notify("No conversation history", "info")
        return
    end
    
    -- Build conversation display
    local conversation_text = "=== Copilot Chat ===\n\n"
    
    -- Show recent messages (last 10 exchanges)
    local recent_count = math.min(20, #chat_state.conversation_history)
    local start_index = math.max(1, #chat_state.conversation_history - recent_count + 1)
    
    for i = start_index, #chat_state.conversation_history do
        local msg = chat_state.conversation_history[i]
        local timestamp = os.date("%H:%M:%S", msg.timestamp)
        
        if msg.type == MESSAGE_TYPES.USER then
            conversation_text = conversation_text .. string.format("[%s] You: %s\n\n", timestamp, msg.content)
        elseif msg.type == MESSAGE_TYPES.ASSISTANT then
            conversation_text = conversation_text .. string.format("[%s] Copilot: %s\n\n", timestamp, msg.content)
        elseif msg.type == MESSAGE_TYPES.ERROR then
            conversation_text = conversation_text .. string.format("[%s] Error: %s\n\n", timestamp, msg.content)
        end
    end
    
    conversation_text = conversation_text .. "\nUse copilot_chat_send \"your message\" to continue the conversation"
    
    ui.display_floating_suggestion(conversation_text)
end

-- Clear conversation history
function M.clear_history()
    chat_state.conversation_history = {}
    chat_state.current_session_id = M.generate_session_id()
    
    local ui = require("xcodex_copilot.ui")
    ui.notify("Chat history cleared", "info")
end

-- Get chat status
function M.get_status()
    return {
        window_open = chat_state.window_open,
        message_count = #chat_state.conversation_history,
        session_id = chat_state.current_session_id,
        max_history = chat_state.max_history_length
    }
end

-- Export conversation to text
function M.export_conversation()
    if #chat_state.conversation_history == 0 then
        return "No conversation to export"
    end
    
    local export_text = "XCodex Copilot Chat Export\n"
    export_text = export_text .. "Session: " .. chat_state.current_session_id .. "\n"
    export_text = export_text .. "Date: " .. os.date("%Y-%m-%d %H:%M:%S") .. "\n"
    export_text = export_text .. string.rep("=", 50) .. "\n\n"
    
    for _, msg in ipairs(chat_state.conversation_history) do
        local timestamp = os.date("%H:%M:%S", msg.timestamp)
        local sender = ""
        
        if msg.type == MESSAGE_TYPES.USER then
            sender = "Developer"
        elseif msg.type == MESSAGE_TYPES.ASSISTANT then
            sender = "Copilot"
        elseif msg.type == MESSAGE_TYPES.ERROR then
            sender = "System Error"
        else
            sender = "System"
        end
        
        export_text = export_text .. string.format("[%s] %s:\n%s\n\n", timestamp, sender, msg.content)
    end
    
    return export_text
end

-- Quick chat commands for common questions
M.quick_commands = {
    ["explain"] = "Can you explain what this code does?",
    ["optimize"] = "How can I optimize this code for better performance?",
    ["debug"] = "I'm having an issue with this code. Can you help me debug it?",
    ["refactor"] = "Can you suggest ways to refactor this code to make it cleaner?",
    ["test"] = "What tests should I write for this code?",
    ["security"] = "Are there any security concerns with this code?",
    ["style"] = "How can I improve the coding style and readability?",
    ["docs"] = "Can you help me write documentation for this code?"
}

-- Execute quick command
function M.quick_command(command)
    local message = M.quick_commands[command]
    if not message then
        local ui = require("xcodex_copilot.ui")
        ui.notify("Unknown quick command: " .. command, "error")
        return false
    end
    
    return M.send_message(message, function(response, error_msg)
        local ui = require("xcodex_copilot.ui")
        if error_msg then
            ui.show_error("Chat failed", error_msg)
        else
            ui.display_floating_suggestion(response)
        end
    end)
end

-- Is chat window open?
function M.is_window_open()
    return chat_state.window_open
end

-- Get recent messages for status display
function M.get_recent_messages(count)
    count = count or 5
    local recent = {}
    local start_index = math.max(1, #chat_state.conversation_history - count + 1)
    
    for i = start_index, #chat_state.conversation_history do
        table.insert(recent, chat_state.conversation_history[i])
    end
    
    return recent
end

return M
