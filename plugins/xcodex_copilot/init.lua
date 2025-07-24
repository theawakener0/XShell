-- XCodex Copilot Main Module
-- Orchestrates all plugin functionality and provides the main API

local M = {}

-- Import modules
local config = require("xcodex_copilot.config")
local context = require("xcodex_copilot.context")
local llm_client = require("xcodex_copilot.llm_client")
local ui = require("xcodex_copilot.ui")
local chat = require("xcodex_copilot.chat")
local interactive_chat = require("xcodex_copilot.interactive_chat")

-- Plugin metadata
plugin_info = {
    name = "XCodex Copilot",
    version = "1.0.0",
    author = "XCodex Team",
    description = "AI-powered coding assistant with inline suggestions and agentic capabilities"
}

-- Plugin state
local state = {
    initialized = false,
    last_suggestion_time = 0,
    suggestion_debounce_delay = 500 -- milliseconds
}

-- Initialize the plugin
function M.setup(user_config)
    if state.initialized then
        ui.notify("Copilot already initialized", "warning")
        return M
    end
    
    -- Setup configuration
    local success, err = pcall(function()
        config.setup(user_config)
    end)
    
    if not success then
        ui.notify("Configuration error: " .. tostring(err), "error")
        return M
    end
    
    -- Validate API key
    local valid, error_msg = config.validate_api_key(config.get("api_key"))
    if not valid then
        ui.notify("API Key Error: " .. error_msg, "error")
        ui.notify("Set GEMINI_API_KEY environment variable", "info")
        return M
    end
    
    -- Setup UI highlights
    ui.setup_highlights()
    
    -- Initialize chat system
    chat.init()
    
    -- Initialize interactive chat
    interactive_chat.init()
    
    state.initialized = true
    ui.notify("XCodex Copilot initialized successfully", "success")
    
    return M
end

-- Main suggestion function
function M.suggest(mode, task_type)
    if not state.initialized then
        ui.notify("Copilot not initialized. Run :lua require('xcodex_copilot').setup()", "error")
        return
    end
    
    mode = mode or "inline"
    task_type = task_type or "general"
    
    -- Check if already processing
    if llm_client.is_thinking() then
        ui.notify("AI is already thinking... please wait", "warning")
        return
    end
    
    -- Show loading indicator
    ui.show_loading_indicator()
    
    -- Get context from editor
    local ctx_success, ctx = pcall(function()
        return context.get_context_for_mode(mode)
    end)
    
    if not ctx_success then
        ui.hide_loading_indicator()
        ui.notify("Failed to get editor context: " .. tostring(ctx), "error")
        return
    end
    
    -- Validate context
    local valid, validation_error = context.validate_context(ctx)
    if not valid then
        ui.hide_loading_indicator()
        ui.notify("Context validation failed: " .. validation_error, "error")
        return
    end
    
    -- Generate prompt based on mode
    local prompt
    if mode == "inline" then
        prompt = config.prompts.inline(ctx)
    elseif mode == "agent" then
        prompt = config.prompts.agent(ctx, task_type)
    else
        ui.hide_loading_indicator()
        ui.notify("Unknown mode: " .. mode, "error")
        return
    end
    
    -- Make API request
    local request_id = llm_client.request_completion(
        prompt,
        config.config,
        function(result, error_msg)
            -- Hide loading indicator
            ui.hide_loading_indicator()
            
            if error_msg then
                ui.show_error("AI request failed", error_msg)
                return
            end
            
            if not result or result == "" then
                ui.notify("AI returned empty response", "warning")
                return
            end
            
            -- Format the result
            local formatted_result = ui.format_code_suggestion(result, ctx.filetype)
            
            -- Display result based on mode
            if mode == "inline" then
                ui.display_inline_suggestion(formatted_result)
            elseif mode == "agent" then
                ui.display_floating_suggestion(formatted_result)
            end
            
            ui.notify("AI suggestion ready", "success")
        end
    )
    
    if request_id then
        ui.notify("Request sent (ID: " .. request_id .. ")", "info")
    else
        ui.hide_loading_indicator()
        ui.notify("Failed to send request", "error")
    end
end

-- Inline suggestion wrapper
function M.suggest_inline()
    M.suggest("inline")
end

-- Agent mode wrapper with different task types
function M.suggest_agent(task_type)
    task_type = task_type or "general"
    M.suggest("agent", task_type)
end

-- Specific agent mode functions
function M.refactor()
    M.suggest_agent("refactor")
end

function M.generate_tests()
    M.suggest_agent("test")
end

function M.explain_code()
    M.suggest_agent("explain")
end

function M.debug_code()
    M.suggest_agent("debug")
end

function M.optimize_code()
    M.suggest_agent("optimize")
end

-- Accept current suggestion
function M.accept()
    return ui.accept_suggestion()
end

-- Reject current suggestion
function M.reject()
    return ui.reject_suggestion()
end

-- Show current suggestion
function M.show()
    return ui.show_suggestion_window()
end

-- Cancel current request
function M.cancel()
    if llm_client.cancel_request() then
        ui.hide_loading_indicator()
        ui.notify("Request cancelled", "info")
        return true
    else
        ui.notify("No active request to cancel", "warning")
        return false
    end
end

-- Get plugin status
function M.status()
    local status_info = {
        initialized = state.initialized,
        api_configured = config.get("api_key") ~= "YOUR_GEMINI_API_KEY_HERE",
        request_status = llm_client.get_request_status(),
        ui_status = {
            suggestion_visible = ui.is_suggestion_visible(),
            current_suggestion = ui.get_current_suggestion(),
            chat_visible = ui.is_chat_visible(),
            active_window = ui.get_active_window()
        },
        chat_status = chat.get_status(),
        interactive_chat_active = interactive_chat.is_active()
    }
    
    -- Display status
    ui.notify("=== XCodex Copilot Status ===", "info")
    ui.notify("Initialized: " .. tostring(status_info.initialized), "info")
    ui.notify("API Configured: " .. tostring(status_info.api_configured), "info")
    ui.notify("Currently Thinking: " .. tostring(status_info.request_status.is_thinking), "info")
    ui.notify("Suggestion Visible: " .. tostring(status_info.ui_status.suggestion_visible), "info")
    ui.notify("Chat Window Open: " .. tostring(status_info.chat_status.window_open), "info")
    ui.notify("Chat Messages: " .. tostring(status_info.chat_status.message_count), "info")
    ui.notify("Interactive Chat Active: " .. tostring(status_info.interactive_chat_active), "info")
    
    return status_info
end

-- Reset plugin state
function M.reset()
    llm_client.reset_state()
    ui.clear_all()
    state.last_suggestion_time = 0
    ui.notify("Copilot state reset", "info")
end

-- Show help
function M.help()
    ui.show_help()
end

-- Chat functionality
function M.chat_toggle()
    if not state.initialized then
        ui.notify("Copilot not initialized", "error")
        return false
    end
    
    return chat.toggle_window()
end

function M.chat_send(message)
    if not state.initialized then
        ui.notify("Copilot not initialized", "error")
        return false
    end
    
    if not chat.is_window_open() then
        chat.open_window()
    end
    
    ui.show_loading_indicator()
    
    chat.send_message(message, function(response, error_msg)
        ui.hide_loading_indicator()
        
        if error_msg then
            ui.show_error("Chat failed", error_msg)
        else
            ui.display_chat_window(response, "Copilot Response")
        end
    end)
    
    return true
end

function M.chat_show()
    if not chat.is_window_open() then
        ui.notify("Chat window not open. Use copilot_chat_toggle to open.", "warning")
        return false
    end
    
    chat.display_conversation()
    return true
end

function M.chat_clear()
    chat.clear_history()
    ui.notify("Chat history cleared", "info")
    return true
end

function M.chat_quick(command)
    if not state.initialized then
        ui.notify("Copilot not initialized", "error")
        return false
    end
    
    return chat.quick_command(command)
end

function M.chat_export()
    local export_text = chat.export_conversation()
    ui.display_floating_suggestion(export_text)
    return true
end

-- Interactive Chat Functions
function M.chat_window_toggle()
    if not state.initialized then
        ui.notify("Copilot not initialized", "error")
        return false
    end
    
    return interactive_chat.toggle()
end

function M.chat_window_send(message)
    if not state.initialized then
        ui.notify("Copilot not initialized", "error")
        return false
    end
    
    return interactive_chat.quick_send(message)
end

function M.chat_window_display()
    return interactive_chat.display()
end

function M.chat_window_clear()
    interactive_chat.clear_conversation()
    return true
end

function M.chat_window_export()
    local export_text = interactive_chat.export_conversation()
    ui.display_floating_suggestion(export_text)
    return true
end

-- Handle key input for interactive chat (to be called by XCodex input system)
function M.handle_chat_input(key)
    return interactive_chat.handle_key_input(key)
end

-- Auto-suggest based on context changes (for future implementation)
function M.auto_suggest()
    if not state.initialized or not config.get("auto_suggest") then
        return
    end
    
    local current_time = os.time() * 1000 -- Convert to milliseconds
    if current_time - state.last_suggestion_time < state.suggestion_debounce_delay then
        return -- Too soon since last suggestion
    end
    
    -- Get current context
    local ctx = context.get_current_buffer_context()
    
    -- Simple heuristic: suggest if user stopped typing for a moment
    -- and there's substantial content before cursor
    if ctx.before_cursor and #ctx.before_cursor > 10 then
        state.last_suggestion_time = current_time
        M.suggest_inline()
    end
end

-- Plugin lifecycle hooks
function M.on_load()
    ui.notify("XCodex Copilot loaded - run :lua require('xcodex_copilot').setup() to initialize", "info")
end

function M.on_char_insert(char)
    -- Check if interactive chat is active
    if interactive_chat.is_active() then
        return interactive_chat.handle_key_input(char)
    end
    
    -- Potential trigger for auto-suggestions
    if state.initialized and config.get("auto_suggest") then
        -- Could implement smart triggering here
        -- For now, we'll be conservative to avoid spam
    end
end

function M.on_cursor_move()
    -- Hide inline suggestions when cursor moves
    if ui.is_suggestion_visible() then
        -- Could implement smart hiding logic
    end
end

-- Export global functions for command mode
function M.export_globals()
    _G.copilot_setup = M.setup
    _G.copilot_suggest_inline = M.suggest_inline
    _G.copilot_suggest_agent = M.suggest_agent
    _G.copilot_refactor = M.refactor
    _G.copilot_test = M.generate_tests
    _G.copilot_explain = M.explain_code
    _G.copilot_debug = M.debug_code
    _G.copilot_optimize = M.optimize_code
    _G.copilot_accept = M.accept
    _G.copilot_reject = M.reject
    _G.copilot_show = M.show
    _G.copilot_cancel = M.cancel
    _G.copilot_status = M.status
    _G.copilot_reset = M.reset
    _G.copilot_help = M.help
    
    -- Chat functions
    _G.copilot_chat_toggle = M.chat_toggle
    _G.copilot_chat_send = M.chat_send
    _G.copilot_chat_show = M.chat_show
    _G.copilot_chat_clear = M.chat_clear
    _G.copilot_chat_quick = M.chat_quick
    _G.copilot_chat_export = M.chat_export
    
    -- Interactive chat functions
    _G.copilot_chat_window_toggle = M.chat_window_toggle
    _G.copilot_chat_window_send = M.chat_window_send
    _G.copilot_chat_window_display = M.chat_window_display
    _G.copilot_chat_window_clear = M.chat_window_clear
    _G.copilot_chat_window_export = M.chat_window_export
end

-- Configuration presets
M.presets = {
    -- Conservative preset
    conservative = {
        auto_suggest = false,
        suggestion_delay = 1000,
        temperature = 0.3,
        max_tokens = 2048
    },
    
    -- Aggressive preset
    aggressive = {
        auto_suggest = true,
        suggestion_delay = 200,
        temperature = 0.8,
        max_tokens = 8192
    },
    
    -- Balanced preset (default)
    balanced = {
        auto_suggest = true,
        suggestion_delay = 500,
        temperature = 0.7,
        max_tokens = 4096
    }
}

-- Apply preset configuration
function M.apply_preset(preset_name)
    local preset = M.presets[preset_name]
    if not preset then
        ui.notify("Unknown preset: " .. preset_name, "error")
        return false
    end
    
    config.setup(preset)
    ui.notify("Applied preset: " .. preset_name, "success")
    return true
end

-- Plugin info for XCodex
M.plugin_info = plugin_info

-- Auto-export globals when loaded
M.export_globals()

return M
