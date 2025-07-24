plugin_info = {
    name = "Line Counter",
    version = "1.1",
    author = "XCodex Team", 
    description = "Shows line statistics and provides line manipulation utilities"
}

function count_lines()
    local content = xcodex.get_file_content()
    local line_count = 0
    local char_count = 0
    local word_count = 0
    
    for line in content:gmatch("[^\r\n]*") do
        line_count = line_count + 1
        char_count = char_count + #line
        
        -- Count words (simple word boundary detection)
        local words = 0
        for word in line:gmatch("%S+") do
            words = words + 1
        end
        word_count = word_count + words
    end
    
    xcodex.set_status_message(string.format("Lines: %d, Words: %d, Chars: %d", 
                                line_count, word_count, char_count))
end

function duplicate_current_line()
    local current_line = xcodex.get_current_line()
    local pos = xcodex.get_cursor_pos()
    
    -- Move to end of line and insert newline
    xcodex.insert_text("\n" .. current_line)
    xcodex.set_status_message("Line duplicated")
end

function on_load()
    xcodex.set_status_message("Line Counter plugin loaded - use :lua count_lines() for stats")
end

-- Export functions for command mode usage
_G.count_lines = count_lines
_G.duplicate_line = duplicate_current_line
