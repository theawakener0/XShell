-- XCodex Copilot Chat Usage Examples
-- Practical examples of using the interactive chat feature

-- Example 1: Basic Chat Session
--[[
Open your XCodex editor with a C file and try these commands:

1. Initialize Copilot:
   :lua copilot_setup()

2. Open chat window:
   :lua copilot_chat_toggle

3. Ask about your code:
   :lua copilot_chat_send("What does this function do?")

4. Follow up with specific questions:
   :lua copilot_chat_send("How can I make this code more efficient?")

5. View the conversation:
   :lua copilot_chat_show
]]--

-- Example 2: Using Quick Commands
--[[
Quick commands are shortcuts for common questions:

:lua copilot_chat_quick("explain")   -- Explains the current code
:lua copilot_chat_quick("optimize")  -- Suggests optimizations
:lua copilot_chat_quick("debug")     -- Helps with debugging
:lua copilot_chat_quick("refactor")  -- Suggests refactoring
:lua copilot_chat_quick("test")      -- Suggests tests to write
:lua copilot_chat_quick("security")  -- Checks for security issues
:lua copilot_chat_quick("style")     -- Improves code style
:lua copilot_chat_quick("docs")      -- Helps write documentation
]]--

-- Example 3: Context-Aware Conversations
--[[
The chat system is aware of your current cursor position and file content.

Position your cursor on a specific function, then:
:lua copilot_chat_send("I'm having trouble understanding this algorithm")

Move to a different function and ask:
:lua copilot_chat_send("How does this relate to the previous function we discussed?")

The AI will remember the conversation and provide contextual responses.
]]--

-- Example 4: Code Review Session
--[[
Use the chat for code review:

:lua copilot_chat_send("Can you review this function for potential issues?")
:lua copilot_chat_send("What edge cases should I consider?")
:lua copilot_chat_send("Is this following C best practices?")
:lua copilot_chat_send("How would you refactor this to be more readable?")
]]--

-- Example 5: Learning and Teaching
--[[
Use chat to learn new concepts:

:lua copilot_chat_send("Can you explain the difference between malloc and calloc?")
:lua copilot_chat_send("What are the pros and cons of this approach?")
:lua copilot_chat_send("Can you show me an alternative implementation?")
:lua copilot_chat_send("What would happen if I changed this line?")
]]--

-- Example 6: Debugging Session
--[[
When you encounter a bug:

:lua copilot_chat_send("This code is causing a segmentation fault. What could be wrong?")
:lua copilot_chat_send("I'm getting unexpected output. Can you trace through the logic?")
:lua copilot_chat_send("What debugging tools would you recommend for this issue?")
]]--

-- Example 7: Architecture Discussion
--[[
Discuss high-level design:

:lua copilot_chat_send("Is this the best data structure for this use case?")
:lua copilot_chat_send("How would this scale with larger datasets?")
:lua copilot_chat_send("What design patterns could improve this code?")
]]--

-- Example 8: Performance Analysis
--[[
Analyze performance implications:

:lua copilot_chat_send("What's the time complexity of this algorithm?")
:lua copilot_chat_send("Where are the potential bottlenecks?")
:lua copilot_chat_send("How can I profile this code to find slow parts?")
]]--

-- Example 9: Cross-Language Help
--[[
Get help with different languages (if working on multi-language projects):

:lua copilot_chat_send("How would I implement this same logic in Python?")
:lua copilot_chat_send("What's the JavaScript equivalent of this C function?")
:lua copilot_chat_send("How do I interface this C code with Lua?")
]]--

-- Example 10: Managing Chat History
--[[
Manage your conversation:

:lua copilot_chat_show      -- View current conversation
:lua copilot_chat_export    -- Export conversation to text
:lua copilot_chat_clear     -- Clear history for fresh start
:lua copilot_status         -- Check chat status
]]--

-- Practical Workflow Example:
--[[
Here's a complete workflow for reviewing and improving a function:

1. Position cursor on the function you want to work on

2. Start a chat session:
   :lua copilot_chat_toggle

3. Get an overview:
   :lua copilot_chat_quick("explain")

4. Ask for improvements:
   :lua copilot_chat_quick("optimize")

5. Check for issues:
   :lua copilot_chat_quick("debug")

6. Get refactoring suggestions:
   :lua copilot_chat_quick("refactor")

7. Ask about testing:
   :lua copilot_chat_quick("test")

8. Follow up with specific questions:
   :lua copilot_chat_send("Should I use a different algorithm here?")

9. Export the conversation for future reference:
   :lua copilot_chat_export

10. Clear when done:
    :lua copilot_chat_clear
]]--

-- Tips for Effective Chat Usage:

--[[
1. BE SPECIFIC: Instead of "fix this", say "this function returns wrong values when input is negative"

2. PROVIDE CONTEXT: Mention what you're trying to achieve, not just what's broken

3. ASK FOLLOW-UPS: The AI remembers conversation history, so build on previous answers

4. USE QUICK COMMANDS: They're faster for common questions

5. COMBINE WITH OTHER FEATURES: Use chat alongside inline suggestions and agent mode

6. EXPORT IMPORTANT CONVERSATIONS: Save useful discussions for later reference

7. CLEAR HISTORY PERIODICALLY: Start fresh when switching to unrelated topics
]]--

return {
    -- Helper functions for common chat workflows
    
    code_review = function()
        copilot_chat_toggle()
        copilot_chat_quick("explain")
        copilot_chat_quick("debug")
        copilot_chat_quick("security")
        copilot_chat_quick("style")
    end,
    
    optimization_session = function()
        copilot_chat_toggle()
        copilot_chat_quick("optimize")
        copilot_chat_send("What's the time complexity of this code?")
        copilot_chat_send("How would this perform with large datasets?")
    end,
    
    learning_session = function(topic)
        copilot_chat_toggle()
        copilot_chat_send("Can you teach me about " .. (topic or "this code pattern"))
        copilot_chat_send("Can you show me some examples?")
        copilot_chat_send("What are common mistakes to avoid?")
    end
}
