-- XCodex Copilot LLM Client Module
-- Handles communication with Gemini API via asynchronous requests

local M = {}

-- State management
local state = {
    is_thinking = false,
    current_request = nil,
    request_id_counter = 0,
    retry_count = 0,
    max_retries = 3
}

-- JSON encoding/decoding (simple implementation)
local json = {}

function json.encode(obj)
    local function encode_string(str)
        str = string.gsub(str, "\\", "\\\\")
        str = string.gsub(str, '"', '\\"')
        str = string.gsub(str, "\n", "\\n")
        str = string.gsub(str, "\r", "\\r")
        str = string.gsub(str, "\t", "\\t")
        return '"' .. str .. '"'
    end
    
    local function encode_value(val)
        local val_type = type(val)
        if val_type == "string" then
            return encode_string(val)
        elseif val_type == "number" then
            return tostring(val)
        elseif val_type == "boolean" then
            return val and "true" or "false"
        elseif val_type == "table" then
            local is_array = true
            local max_index = 0
            for k, v in pairs(val) do
                if type(k) ~= "number" or k ~= math.floor(k) or k < 1 then
                    is_array = false
                    break
                end
                max_index = math.max(max_index, k)
            end
            
            if is_array then
                local parts = {}
                for i = 1, max_index do
                    parts[i] = encode_value(val[i])
                end
                return "[" .. table.concat(parts, ",") .. "]"
            else
                local parts = {}
                for k, v in pairs(val) do
                    table.insert(parts, encode_string(tostring(k)) .. ":" .. encode_value(v))
                end
                return "{" .. table.concat(parts, ",") .. "}"
            end
        elseif val == nil then
            return "null"
        else
            error("Cannot encode value of type " .. val_type)
        end
    end
    
    return encode_value(obj)
end

function json.decode(str)
    local pos = 1
    
    local function skip_whitespace()
        while pos <= #str and str:sub(pos, pos):match("%s") do
            pos = pos + 1
        end
    end
    
    local function decode_string()
        if str:sub(pos, pos) ~= '"' then
            error("Expected string at position " .. pos)
        end
        pos = pos + 1
        local start_pos = pos
        while pos <= #str do
            local char = str:sub(pos, pos)
            if char == '"' then
                local result = str:sub(start_pos, pos - 1)
                pos = pos + 1
                return result
            elseif char == "\\" then
                pos = pos + 2 -- Skip escaped character
            else
                pos = pos + 1
            end
        end
        error("Unterminated string")
    end
    
    local function decode_number()
        local start_pos = pos
        if str:sub(pos, pos) == "-" then pos = pos + 1 end
        while pos <= #str and str:sub(pos, pos):match("%d") do
            pos = pos + 1
        end
        if str:sub(pos, pos) == "." then
            pos = pos + 1
            while pos <= #str and str:sub(pos, pos):match("%d") do
                pos = pos + 1
            end
        end
        return tonumber(str:sub(start_pos, pos - 1))
    end
    
    local function decode_value()
        skip_whitespace()
        local char = str:sub(pos, pos)
        if char == '"' then
            return decode_string()
        elseif char:match("[%d-]") then
            return decode_number()
        elseif str:sub(pos, pos + 3) == "true" then
            pos = pos + 4
            return true
        elseif str:sub(pos, pos + 4) == "false" then
            pos = pos + 5
            return false
        elseif str:sub(pos, pos + 3) == "null" then
            pos = pos + 4
            return nil
        elseif char == "[" then
            pos = pos + 1
            local array = {}
            skip_whitespace()
            if str:sub(pos, pos) ~= "]" then
                repeat
                    table.insert(array, decode_value())
                    skip_whitespace()
                    if str:sub(pos, pos) == "," then
                        pos = pos + 1
                    elseif str:sub(pos, pos) == "]" then
                        break
                    else
                        error("Expected ',' or ']' in array")
                    end
                until false
            end
            pos = pos + 1
            return array
        elseif char == "{" then
            pos = pos + 1
            local object = {}
            skip_whitespace()
            if str:sub(pos, pos) ~= "}" then
                repeat
                    local key = decode_string()
                    skip_whitespace()
                    if str:sub(pos, pos) ~= ":" then
                        error("Expected ':' after key")
                    end
                    pos = pos + 1
                    object[key] = decode_value()
                    skip_whitespace()
                    if str:sub(pos, pos) == "," then
                        pos = pos + 1
                        skip_whitespace()
                    elseif str:sub(pos, pos) == "}" then
                        break
                    else
                        error("Expected ',' or '}' in object")
                    end
                until false
            end
            pos = pos + 1
            return object
        else
            error("Unexpected character: " .. char)
        end
    end
    
    return decode_value()
end

-- Build Gemini API request payload
function M.build_request_payload(prompt, config)
    config = config or {}
    
    local payload = {
        contents = {
            {
                parts = {
                    {
                        text = prompt
                    }
                }
            }
        },
        generationConfig = {
            temperature = config.temperature or 0.7,
            maxOutputTokens = config.max_tokens or 8192,
            topP = config.top_p or 0.8,
            topK = config.top_k or 40
        },
        safetySettings = {
            {
                category = "HARM_CATEGORY_HARASSMENT",
                threshold = "BLOCK_MEDIUM_AND_ABOVE"
            },
            {
                category = "HARM_CATEGORY_HATE_SPEECH", 
                threshold = "BLOCK_MEDIUM_AND_ABOVE"
            },
            {
                category = "HARM_CATEGORY_SEXUALLY_EXPLICIT",
                threshold = "BLOCK_MEDIUM_AND_ABOVE"
            },
            {
                category = "HARM_CATEGORY_DANGEROUS_CONTENT",
                threshold = "BLOCK_MEDIUM_AND_ABOVE"
            }
        }
    }
    
    return payload
end

-- Create curl command for API request
function M.build_curl_command(url, payload, headers, timeout)
    timeout = timeout or 30
    
    local cmd_parts = {
        "curl",
        "-s", -- Silent mode
        "-X", "POST",
        "--max-time", tostring(timeout),
        "--header", "Content-Type: application/json"
    }
    
    -- Add API key header
    if headers and headers["x-goog-api-key"] then
        table.insert(cmd_parts, "--header")
        table.insert(cmd_parts, "x-goog-api-key: " .. headers["x-goog-api-key"])
    end
    
    -- Add data payload
    table.insert(cmd_parts, "--data")
    table.insert(cmd_parts, json.encode(payload))
    
    -- Add URL
    table.insert(cmd_parts, url)
    
    return table.concat(cmd_parts, " ")
end

-- Parse API response
function M.parse_response(response_text)
    if not response_text or response_text == "" then
        return nil, "Empty response"
    end
    
    local success, response = pcall(json.decode, response_text)
    if not success then
        return nil, "Failed to parse JSON response: " .. tostring(response)
    end
    
    if response.error then
        return nil, "API Error: " .. (response.error.message or "Unknown error")
    end
    
    if not response.candidates or #response.candidates == 0 then
        return nil, "No candidates in response"
    end
    
    local candidate = response.candidates[1]
    if not candidate.content or not candidate.content.parts or #candidate.content.parts == 0 then
        return nil, "No content in response"
    end
    
    local text = candidate.content.parts[1].text
    if not text or text == "" then
        return nil, "Empty text in response"
    end
    
    -- Clean up the response text
    text = text:gsub("^%s+", ""):gsub("%s+$", "") -- Trim whitespace
    
    return text, nil
end

-- Exponential backoff calculation
function M.calculate_backoff_delay(attempt)
    local base_delay = 1000 -- 1 second base
    local max_delay = 16000 -- 16 seconds max
    local delay = base_delay * (2 ^ (attempt - 1))
    return math.min(delay, max_delay)
end

-- Check if client is currently processing a request
function M.is_thinking()
    return state.is_thinking
end

-- Generate unique request ID
function M.generate_request_id()
    state.request_id_counter = state.request_id_counter + 1
    return "req_" .. state.request_id_counter .. "_" .. os.time()
end

-- Asynchronous request to Gemini API
function M.request_completion(prompt, config, callback)
    if state.is_thinking then
        callback(nil, "Another request is already in progress")
        return nil
    end
    
    local request_id = M.generate_request_id()
    state.is_thinking = true
    state.current_request = request_id
    state.retry_count = 0
    
    -- Validate configuration
    local api_config = config or require("xcodex_copilot.config").config
    if not api_config.api_key or api_config.api_key == "YOUR_GEMINI_API_KEY_HERE" then
        state.is_thinking = false
        callback(nil, "API key not configured")
        return request_id
    end
    
    -- Build request
    local payload = M.build_request_payload(prompt, api_config)
    local headers = {
        ["x-goog-api-key"] = api_config.api_key
    }
    local url = string.format("%s/%s:generateContent", 
                             api_config.base_url, 
                             api_config.gemini_model)
    
    local curl_command = M.build_curl_command(url, payload, headers, api_config.request_timeout)
    
    -- Execute async request using os.execute (blocking fallback)
    -- Note: In a real implementation, this should use a non-blocking job system
    -- For XCodex, we would need to implement or use an async job runner
    
    local function execute_request(attempt)
        attempt = attempt or 1
        
        -- Create a temporary file for output
        local temp_file = os.tmpname()
        local full_command = curl_command .. " > " .. temp_file .. " 2>&1"
        
        -- Execute command
        local exit_code = os.execute(full_command)
        
        -- Read response
        local file = io.open(temp_file, "r")
        local response_text = ""
        if file then
            response_text = file:read("*all")
            file:close()
        end
        
        -- Clean up temp file
        os.remove(temp_file)
        
        -- Process response
        if exit_code == 0 or (type(exit_code) == "boolean" and exit_code) then
            local result, error_msg = M.parse_response(response_text)
            state.is_thinking = false
            state.current_request = nil
            
            if result then
                callback(result, nil)
            else
                -- Retry on certain errors
                if attempt < state.max_retries and 
                   (error_msg:find("timeout") or error_msg:find("429") or error_msg:find("503")) then
                    
                    local delay = M.calculate_backoff_delay(attempt)
                    xcodex.set_status_message(string.format("Request failed, retrying in %dms...", delay))
                    
                    -- Simulate delay (in real implementation, use proper timer)
                    -- This is a blocking delay - should be replaced with non-blocking timer
                    os.execute("sleep " .. math.floor(delay / 1000))
                    
                    execute_request(attempt + 1)
                else
                    callback(nil, error_msg)
                end
            end
        else
            -- Command failed
            state.is_thinking = false
            state.current_request = nil
            
            if attempt < state.max_retries then
                local delay = M.calculate_backoff_delay(attempt)
                xcodex.set_status_message(string.format("Network error, retrying in %dms...", delay))
                
                -- Simulate delay
                os.execute("sleep " .. math.floor(delay / 1000))
                execute_request(attempt + 1)
            else
                callback(nil, "Network request failed after " .. state.max_retries .. " attempts")
            end
        end
    end
    
    -- Start the request
    execute_request()
    
    return request_id
end

-- Alternative non-blocking implementation using a simple job queue
-- This would need to be integrated with XCodex's event loop
local job_queue = {}

function M.request_completion_async(prompt, config, callback)
    if state.is_thinking then
        callback(nil, "Another request is already in progress")
        return nil
    end
    
    local request_id = M.generate_request_id()
    state.is_thinking = true
    state.current_request = request_id
    
    -- Add job to queue
    table.insert(job_queue, {
        id = request_id,
        prompt = prompt,
        config = config,
        callback = callback,
        attempt = 1
    })
    
    -- Process queue (this would be called periodically by XCodex)
    M.process_job_queue()
    
    return request_id
end

-- Process pending jobs (should be called by editor's event loop)
function M.process_job_queue()
    if #job_queue == 0 then return end
    
    local job = table.remove(job_queue, 1)
    
    -- Execute the request (simplified version)
    -- In practice, this would spawn a background process
    local success, result = pcall(function()
        return M.request_completion(job.prompt, job.config, job.callback)
    end)
    
    if not success then
        job.callback(nil, "Request execution failed: " .. tostring(result))
    end
end

-- Cancel current request
function M.cancel_request()
    if state.current_request then
        state.is_thinking = false
        state.current_request = nil
        xcodex.set_status_message("Request cancelled")
        return true
    end
    return false
end

-- Get current request status
function M.get_request_status()
    return {
        is_thinking = state.is_thinking,
        current_request = state.current_request,
        retry_count = state.retry_count,
        queue_length = #job_queue
    }
end

-- Reset client state
function M.reset_state()
    state.is_thinking = false
    state.current_request = nil
    state.retry_count = 0
    job_queue = {}
end

return M
