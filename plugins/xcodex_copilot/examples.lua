-- XCodex Copilot Example Configuration
-- Copy this file and modify as needed for your setup

-- Example 1: Basic setup with environment variable API key
local basic_config = {
    api_key = os.getenv("GEMINI_API_KEY"),
    auto_suggest = false,  -- Start conservative
    suggestion_delay = 800
}

-- Example 2: Advanced configuration with custom settings
local advanced_config = {
    -- API Configuration
    api_key = os.getenv("GEMINI_API_KEY"),
    gemini_model = "gemini-2.0-flash-exp",
    base_url = "https://generativelanguage.googleapis.com/v1beta/models",
    request_timeout = 30,
    temperature = 0.7,
    max_tokens = 8192,
    
    -- Behavior Configuration
    auto_suggest = true,
    suggestion_delay = 500,
    max_context_lines = 100,
    
    -- UI Configuration
    ui_floating_width = 90,
    ui_floating_height = 25,
    ui_floating_border = "rounded",
    ui_floating_highlight = "CopilotSuggestion"
}

-- Example 3: Development/testing configuration
local dev_config = {
    api_key = os.getenv("GEMINI_API_KEY"),
    gemini_model = "gemini-2.0-flash-exp",
    temperature = 0.3,  -- More deterministic for testing
    max_tokens = 4096,
    auto_suggest = false,  -- Manual control during development
    request_timeout = 15,  -- Shorter timeout for faster feedback
    ui_floating_width = 70,
    ui_floating_height = 15
}

-- Example 4: Production configuration
local production_config = {
    api_key = os.getenv("GEMINI_API_KEY"),
    gemini_model = "gemini-2.0-flash-exp",
    temperature = 0.7,
    max_tokens = 8192,
    auto_suggest = true,
    suggestion_delay = 600,  -- Slightly conservative for production
    request_timeout = 30,
    max_context_lines = 150,
    ui_floating_width = 80,
    ui_floating_height = 20
}

-- Example usage in your XCodex configuration:
-- 
-- To use one of these configurations, add to your XCodex init file:
--
-- require("xcodex_copilot").setup(basic_config)
-- 
-- Or for quick setup:
-- require("xcodex_copilot").setup()  -- Uses defaults with GEMINI_API_KEY

-- Example keybinding setup (pseudo-code for XCodex)
--[[
function setup_copilot_keybindings()
    -- These would need to be adapted to XCodex's actual keybinding system
    
    -- Inline suggestions
    bind_key("<C-s>", function()
        require("xcodex_copilot").suggest_inline()
    end)
    
    -- Agent mode
    bind_key("<leader>ag", function()
        require("xcodex_copilot").suggest_agent()
    end)
    
    -- Accept suggestion
    bind_key("<Tab>", function()
        if require("xcodex_copilot.ui").is_suggestion_visible() then
            require("xcodex_copilot").accept()
            return true  -- Consume the key
        end
        return false  -- Let normal Tab handling proceed
    end)
    
    -- Reject suggestion
    bind_key("<Esc>", function()
        if require("xcodex_copilot.ui").is_suggestion_visible() then
            require("xcodex_copilot").reject()
            return true  -- Consume the key
        end
        return false  -- Let normal Esc handling proceed
    end)
    
    -- Quick refactor
    bind_key("<leader>cr", function()
        require("xcodex_copilot").refactor()
    end)
    
    -- Generate tests
    bind_key("<leader>ct", function()
        require("xcodex_copilot").generate_tests()
    end)
    
    -- Explain code
    bind_key("<leader>ce", function()
        require("xcodex_copilot").explain_code()
    end)
    
    -- Cancel request
    bind_key("<C-c>", function()
        require("xcodex_copilot").cancel()
    end)
end
]]--

-- Export configurations for use
return {
    basic = basic_config,
    advanced = advanced_config,
    dev = dev_config,
    production = production_config
}
