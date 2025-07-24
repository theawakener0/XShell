-- XCodex Copilot Configuration Module
-- Manages all configurable parameters for the AI assistant

local M = {}

-- Default configuration
M.defaults = {
    -- API Configuration
    api_key = os.getenv("GEMINI_API_KEY") or "YOUR_GEMINI_API_KEY_HERE",
    gemini_model = "gemini-2.0-flash",
    base_url = "https://generativelanguage.googleapis.com/v1beta/models",
    
    -- Request Configuration
    request_timeout = 30,
    max_tokens = 8192,
    temperature = 0.7,
    
    -- UI Configuration
    ui_floating_width = 80,
    ui_floating_height = 20,
    ui_floating_border = "rounded",
    ui_floating_highlight = "CopilotSuggestion",
    
    -- Behavior Configuration
    auto_suggest = true,
    suggestion_delay = 500, -- milliseconds
    max_context_lines = 100,
    
    -- Keybindings
    keybinds = {
        inline_suggest = "<C-s>",
        agent_mode = "<leader>ag",
        chat_toggle = "<leader>ch",
        accept_suggestion = "<Tab>",
        reject_suggestion = "<Esc>",
        dismiss_window = "<C-c>",
    }
}

-- Current configuration (starts with defaults)
M.config = {}

-- Deep copy function for tables
local function deep_copy(orig)
    local copy
    if type(orig) == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deep_copy(orig_key)] = deep_copy(orig_value)
        end
        setmetatable(copy, deep_copy(getmetatable(orig)))
    else
        copy = orig
    end
    return copy
end

-- Initialize configuration
function M.setup(user_config)
    -- Start with defaults
    M.config = deep_copy(M.defaults)
    
    -- Override with user configuration
    if user_config then
        for key, value in pairs(user_config) do
            if type(value) == "table" and type(M.config[key]) == "table" then
                -- Merge table values
                for sub_key, sub_value in pairs(value) do
                    M.config[key][sub_key] = sub_value
                end
            else
                M.config[key] = value
            end
        end
    end
    
    -- Validate essential configuration
    if M.config.api_key == "YOUR_GEMINI_API_KEY_HERE" then
        error("Please set your Gemini API key via GEMINI_API_KEY environment variable or config.api_key")
    end
    
    return M.config
end

-- Get configuration value
function M.get(key)
    if not M.config or not M.config[key] then
        return M.defaults[key]
    end
    return M.config[key]
end

-- Prompt Templates
M.prompts = {}

-- Generate inline completion prompt
function M.prompts.inline(ctx)
    local prompt = string.format([[
You are an intelligent code completion assistant. Given the current code context, provide a smart, contextually appropriate completion.

Current file: %s
Language: %s
Current line: %d
Current column: %d

Code before cursor:
```
%s
```

Code after cursor:
```
%s
```

Instructions:
1. Provide ONLY the text that should be inserted at the cursor position
2. Be concise and relevant to the immediate context
3. Follow the coding style and patterns in the existing code
4. Don't include explanations or markdown formatting
5. If no completion is appropriate, respond with an empty string

Response:]], 
        ctx.filename or "untitled",
        ctx.filetype or "text",
        ctx.cursor_line,
        ctx.cursor_col,
        ctx.before_cursor or "",
        ctx.after_cursor or ""
    )
    
    return prompt
end

-- Generate agent mode prompt for complex tasks
function M.prompts.agent(ctx, task_type)
    task_type = task_type or "general"
    
    local base_prompt = string.format([[
You are an expert programming assistant working on a %s file. Help the user with the following task.

Current file: %s
Language: %s
Lines: %d

Full code context:
```%s
%s
```

Current cursor position: Line %d, Column %d

]], 
        ctx.filetype or "text",
        ctx.filename or "untitled", 
        ctx.filetype or "text",
        ctx.total_lines or 0,
        ctx.filetype or "",
        ctx.full_content or "",
        ctx.cursor_line,
        ctx.cursor_col
    )
    
    local task_prompts = {
        refactor = base_prompt .. [[
TASK: Code Refactoring
Please analyze the code and suggest improvements. Focus on:
1. Code structure and organization
2. Performance optimizations
3. Readability improvements
4. Best practices compliance
5. Potential bug fixes

Provide the refactored code with explanations for each change.
]],
        
        test = base_prompt .. [[
TASK: Test Generation
Generate comprehensive unit tests for the current code. Include:
1. Test cases for normal functionality
2. Edge cases and error conditions
3. Mock data where appropriate
4. Clear test descriptions

Use appropriate testing framework conventions for the language.
]],
        
        explain = base_prompt .. [[
TASK: Code Explanation
Provide a detailed explanation of the current code including:
1. What the code does (high-level purpose)
2. How it works (implementation details)
3. Key algorithms or patterns used
4. Potential improvements or concerns
5. Dependencies and relationships
]],
        
        debug = base_prompt .. [[
TASK: Debug Assistance
Help identify potential bugs or issues in the code:
1. Analyze for common error patterns
2. Check for logical inconsistencies
3. Identify potential runtime errors
4. Suggest debugging strategies
5. Recommend fixes with explanations
]],
        
        optimize = base_prompt .. [[
TASK: Performance Optimization
Analyze the code for performance improvements:
1. Identify bottlenecks and inefficiencies
2. Suggest algorithmic improvements
3. Recommend better data structures
4. Memory usage optimization
5. Language-specific optimizations
]],
        
        general = base_prompt .. [[
TASK: General Programming Assistance
Help with any programming task or question. Provide:
1. Clear, actionable advice
2. Code examples when relevant
3. Best practices guidance
4. Alternative approaches if applicable
5. Explanations for your recommendations
]]
    }
    
    return task_prompts[task_type] or task_prompts.general
end

-- Generate chat prompt with conversation context
function M.prompts.chat(ctx, message, conversation_history)
    local base_prompt = string.format([[
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

You should:
1. Be conversational and helpful
2. Reference the code context when relevant
3. Provide specific, actionable advice
4. Ask clarifying questions when needed
5. Keep responses concise but thorough
6. Use code examples when helpful
7. Remember our conversation history

]], 
        ctx.filename or "untitled",
        ctx.filetype or "text",
        ctx.total_lines or 0,
        ctx.cursor_line or 1,
        ctx.cursor_col or 1,
        ctx.filetype or "",
        ctx.full_content or ""
    )
    
    -- Add conversation history if available
    if conversation_history and #conversation_history > 0 then
        base_prompt = base_prompt .. "\nConversation History:\n"
        for _, msg in ipairs(conversation_history) do
            local role = msg.role == "user" and "Developer" or "Assistant"
            base_prompt = base_prompt .. string.format("%s: %s\n", role, msg.content)
        end
        base_prompt = base_prompt .. "\n"
    end
    
    base_prompt = base_prompt .. "Developer: " .. message .. "\n\nAssistant:"
    
    return base_prompt
end

-- Validate API key format
function M.validate_api_key(key)
    if not key or key == "" or key == "YOUR_GEMINI_API_KEY_HERE" then
        return false, "API key is required"
    end
    
    -- Basic validation for Gemini API key format
    if not string.match(key, "^[A-Za-z0-9_-]+$") then
        return false, "Invalid API key format"
    end
    
    return true, nil
end

-- Get headers for API requests
function M.get_api_headers()
    return {
        ["Content-Type"] = "application/json",
        ["x-goog-api-key"] = M.config.api_key or M.defaults.api_key
    }
end

-- Get full API URL for requests
function M.get_api_url()
    local base = M.config.base_url or M.defaults.base_url
    local model = M.config.gemini_model or M.defaults.gemini_model
    return string.format("%s/%s:generateContent", base, model)
end

return M
