-- XCodex Copilot Plugin Registration
-- This file makes the plugin discoverable by XCodex's plugin system

-- Import the main plugin module
local copilot = require("xcodex_copilot.init")

-- Plugin metadata (required by XCodex)
plugin_info = {
    name = "XCodex Copilot",
    version = "1.0.0",
    author = "XCodex Team",
    description = "AI-powered coding assistant with inline suggestions and agentic capabilities"
}

-- Plugin lifecycle hooks for XCodex
function on_load()
    -- Initialize the copilot plugin
    copilot.on_load()
    
    -- Auto-setup with default configuration
    -- Users can override this by calling copilot_setup() with custom config
    local default_config = {
        auto_suggest = false, -- Start conservative
        suggestion_delay = 800,
        temperature = 0.7
    }
    
    -- Only auto-setup if API key is available
    local api_key = os.getenv("GEMINI_API_KEY")
    if api_key and api_key ~= "" then
        copilot.setup(default_config)
        xcodex.set_status_message("XCodex Copilot ready! Try: copilot_suggest_inline")
    else
        xcodex.set_status_message("XCodex Copilot loaded. Set GEMINI_API_KEY and run copilot_setup()")
    end
end

function on_char_insert(char)
    copilot.on_char_insert(char)
end

function on_cursor_move()
    copilot.on_cursor_move()
end

function on_save()
    -- Could implement auto-formatting or linting here
    if copilot.is_initialized then
        xcodex.set_status_message("File saved with Copilot active")
    end
end

-- Export the main module for direct access
return copilot
