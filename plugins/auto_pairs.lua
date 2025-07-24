plugin_info = {
    name = "Auto Pairs",
    version = "1.0",
    author = "XCodex Team",
    description = "Automatically insert closing brackets and quotes"
}

local pairs_map = {
    ['('] = ')',
    ['['] = ']',
    ['{'] = '}',
    ['"'] = '"',
    ["'"] = "'"
}

function on_char_insert(char)
    if pairs_map[char] then
        local pos = xcodex.get_cursor_pos()
        xcodex.insert_text(pairs_map[char])
        xcodex.set_cursor_pos(pos.line, pos.col)
        xcodex.set_status_message("Auto-paired: " .. char .. pairs_map[char])
    end
end

function on_load()
    xcodex.set_status_message("Auto Pairs plugin loaded - automatic bracket/quote pairing enabled")
end

function on_save()
    xcodex.set_status_message("File saved with Auto Pairs active")
end
