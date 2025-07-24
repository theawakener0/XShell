-- XCodex Copilot Context Module
-- Extracts relevant context from the editor state

local M = {}

-- Get file extension to determine language
local function get_filetype_from_extension(filename)
    if not filename then return "text" end
    
    local ext = filename:match("^.+%.(.+)$")
    if not ext then return "text" end
    
    local language_map = {
        c = "c",
        h = "c",
        cpp = "cpp", cxx = "cpp", cc = "cpp",
        hpp = "cpp", hxx = "cpp", hh = "cpp",
        rs = "rust",
        py = "python", pyw = "python",
        js = "javascript", jsx = "javascript",
        ts = "typescript", tsx = "typescript",
        go = "go",
        java = "java",
        lua = "lua",
        sh = "bash", bash = "bash",
        rb = "ruby",
        php = "php",
        html = "html", htm = "html",
        css = "css",
        json = "json",
        xml = "xml",
        md = "markdown", markdown = "markdown",
        txt = "text"
    }
    
    return language_map[ext:lower()] or "text"
end

-- Split text into lines
local function split_lines(text)
    local lines = {}
    for line in text:gmatch("[^\r\n]*") do
        table.insert(lines, line)
    end
    return lines
end

-- Get lines around cursor with specified context window
local function get_context_window(lines, cursor_line, window_size)
    local start_line = math.max(1, cursor_line - window_size)
    local end_line = math.min(#lines, cursor_line + window_size)
    
    local context_lines = {}
    for i = start_line, end_line do
        table.insert(context_lines, lines[i])
    end
    
    return context_lines, start_line, end_line
end

-- Get current buffer context for AI processing
function M.get_current_buffer_context()
    local context = {}
    
    -- Get basic editor information
    context.filename = xcodex.get_filename()
    context.filetype = get_filetype_from_extension(context.filename)
    
    -- Get cursor position
    local cursor_pos = xcodex.get_cursor_pos()
    context.cursor_line = cursor_pos.line
    context.cursor_col = cursor_pos.col
    
    -- Get full file content
    local full_content = xcodex.get_file_content()
    context.full_content = full_content
    
    -- Split content into lines
    local lines = split_lines(full_content)
    context.total_lines = #lines
    context.lines = lines
    
    -- Get current line
    context.current_line = xcodex.get_current_line()
    
    -- Extract code before and after cursor
    if context.cursor_line > 0 and context.cursor_line <= #lines then
        local current_line_text = lines[context.cursor_line] or ""
        local cursor_col_zero_based = math.max(0, context.cursor_col - 1)
        
        context.before_cursor_line = current_line_text:sub(1, cursor_col_zero_based)
        context.after_cursor_line = current_line_text:sub(cursor_col_zero_based + 1)
    else
        context.before_cursor_line = ""
        context.after_cursor_line = ""
    end
    
    -- Get context window around cursor (for inline suggestions)
    local context_window_size = 10 -- lines before/after cursor
    local context_lines, start_line, end_line = get_context_window(
        lines, context.cursor_line, context_window_size
    )
    
    -- Build before/after cursor context with multiple lines
    local before_lines = {}
    local after_lines = {}
    
    -- Lines before cursor
    for i = start_line, context.cursor_line - 1 do
        if lines[i] then
            table.insert(before_lines, lines[i])
        end
    end
    
    -- Current line before cursor
    if context.before_cursor_line and context.before_cursor_line ~= "" then
        table.insert(before_lines, context.before_cursor_line)
    end
    
    -- Current line after cursor
    if context.after_cursor_line and context.after_cursor_line ~= "" then
        table.insert(after_lines, context.after_cursor_line)
    end
    
    -- Lines after cursor
    for i = context.cursor_line + 1, end_line do
        if lines[i] then
            table.insert(after_lines, lines[i])
        end
    end
    
    context.before_cursor = table.concat(before_lines, "\n")
    context.after_cursor = table.concat(after_lines, "\n")
    
    -- Additional context for agent mode
    context.context_window = {
        lines = context_lines,
        start_line = start_line,
        end_line = end_line
    }
    
    -- Get editor mode
    local mode = xcodex.get_mode()
    context.editor_mode = mode
    
    -- Analyze code patterns for better context
    context.indentation = M.detect_indentation(lines)
    context.language_features = M.analyze_language_features(context.filetype, full_content)
    
    return context
end

-- Detect indentation style (spaces vs tabs, size)
function M.detect_indentation(lines)
    local space_count = 0
    local tab_count = 0
    local space_sizes = {}
    
    for _, line in ipairs(lines) do
        if line:match("^%s+") then
            local leading_spaces = line:match("^( +)")
            local leading_tabs = line:match("^(\t+)")
            
            if leading_spaces then
                space_count = space_count + 1
                local len = #leading_spaces
                space_sizes[len] = (space_sizes[len] or 0) + 1
            elseif leading_tabs then
                tab_count = tab_count + 1
            end
        end
    end
    
    local indent_type = (tab_count > space_count) and "tabs" or "spaces"
    local indent_size = 4 -- default
    
    if indent_type == "spaces" then
        -- Find most common indentation size
        local max_count = 0
        for size, count in pairs(space_sizes) do
            if count > max_count and size > 0 and size <= 8 then
                max_count = count
                indent_size = size
            end
        end
    end
    
    return {
        type = indent_type,
        size = indent_size
    }
end

-- Analyze language-specific features
function M.analyze_language_features(filetype, content)
    local features = {
        has_imports = false,
        has_functions = false,
        has_classes = false,
        has_comments = false,
        style_info = {}
    }
    
    if not content then return features end
    
    -- Common patterns across languages
    if filetype == "c" or filetype == "cpp" then
        features.has_imports = content:match("#include") ~= nil
        features.has_functions = content:match("%w+%s*%([^)]*%)%s*{") ~= nil
        features.has_classes = content:match("class%s+%w+") ~= nil or content:match("struct%s+%w+") ~= nil
        features.has_comments = content:match("//") ~= nil or content:match("/%*") ~= nil
        
    elseif filetype == "python" then
        features.has_imports = content:match("import%s+") ~= nil or content:match("from%s+") ~= nil
        features.has_functions = content:match("def%s+%w+") ~= nil
        features.has_classes = content:match("class%s+%w+") ~= nil
        features.has_comments = content:match("#") ~= nil
        
    elseif filetype == "javascript" or filetype == "typescript" then
        features.has_imports = content:match("import%s+") ~= nil or content:match("require%(") ~= nil
        features.has_functions = content:match("function%s+") ~= nil or content:match("=>[^=]") ~= nil
        features.has_classes = content:match("class%s+%w+") ~= nil
        features.has_comments = content:match("//") ~= nil or content:match("/%*") ~= nil
        
    elseif filetype == "rust" then
        features.has_imports = content:match("use%s+") ~= nil
        features.has_functions = content:match("fn%s+%w+") ~= nil
        features.has_classes = content:match("struct%s+%w+") ~= nil or content:match("impl%s+") ~= nil
        features.has_comments = content:match("//") ~= nil or content:match("/%*") ~= nil
        
    elseif filetype == "go" then
        features.has_imports = content:match("import%s+") ~= nil
        features.has_functions = content:match("func%s+%w+") ~= nil
        features.has_classes = content:match("type%s+%w+%s+struct") ~= nil
        features.has_comments = content:match("//") ~= nil or content:match("/%*") ~= nil
    end
    
    return features
end

-- Get selected text if any (fallback to current line if no selection)
function M.get_selection_context()
    -- XCodex doesn't seem to have selection API exposed yet
    -- Fall back to current line
    return {
        has_selection = false,
        selected_text = xcodex.get_current_line(),
        selection_start = nil,
        selection_end = nil
    }
end

-- Get context for specific modes
function M.get_context_for_mode(mode)
    local base_context = M.get_current_buffer_context()
    
    if mode == "inline" then
        -- For inline suggestions, focus on immediate cursor context
        return {
            filename = base_context.filename,
            filetype = base_context.filetype,
            cursor_line = base_context.cursor_line,
            cursor_col = base_context.cursor_col,
            before_cursor = base_context.before_cursor,
            after_cursor = base_context.after_cursor,
            current_line = base_context.current_line,
            indentation = base_context.indentation
        }
        
    elseif mode == "agent" then
        -- For agent mode, provide full context
        return base_context
        
    else
        return base_context
    end
end

-- Validate context before sending to AI
function M.validate_context(context)
    if not context then
        return false, "No context provided"
    end
    
    if not context.cursor_line or context.cursor_line < 1 then
        return false, "Invalid cursor position"
    end
    
    if not context.filetype then
        context.filetype = "text"
    end
    
    return true, nil
end

return M
