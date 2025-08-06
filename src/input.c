#include "input.h"
#include "xsh.h" // For build_prompt, history, etc.
#include "builtins.h" // For xsh_num_builtins, builtin_str for completion
#include "history.h" // For enhanced smart completion functions

#ifdef __linux__ // For termios, read, isatty, STDIN_FILENO
#include <termios.h>
#include <unistd.h> 
#endif

#ifdef _WIN32
#include <sys/stat.h>
#include <direct.h>
#include <conio.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <ctype.h>
#include <string.h>

// Implementation of input handling functions

char *xsh_read_line(void){
    int bufsize = XSH_RL_BUFSIZE;
    int position = 0;
    int cursor_pos = 0; // Current cursor position in the buffer
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    // History navigation variables
    static int history_index = -1; // -1 means no history navigation active
    static char *original_buffer = NULL; // Store original input when navigating history

    if (!buffer) {
        fprintf(stderr, "xsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0'; // Initialize buffer

#ifdef _WIN32
    while (1) {
        c = _getch(); // Use _getch() to get a character without echoing it
        
        // Handle arrow keys (extended keys on Windows)
        if (c == EXTENDED_KEY) {
            c = _getch(); // Get the actual arrow key code
            
            if (c == ARROW_UP) {
                // Navigate up in history
                if (history_count > 0) {
                    if (history_index == -1) {
                        // First time navigating, save current input
                        if (original_buffer) free(original_buffer);
                        original_buffer = strdup(buffer);
                        history_index = history_count - 1;
                    } else if (history_index > 0) {
                        history_index--;
                    }
                    
                    if (history_index >= 0 && history_index < history_count && history[history_index]) {
                        // Clear current line
                        printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                        
                        // Copy history entry to buffer
                        strncpy(buffer, history[history_index], bufsize - 1);
                        buffer[bufsize - 1] = '\0';
                        position = strlen(buffer);
                        cursor_pos = position;
                        
                        // Display new command
                        printf("%s%s", build_prompt(), buffer);
                    }
                }
                continue;
            }
            
            if (c == ARROW_DOWN) {
                // Navigate down in history
                if (history_index != -1) {
                    if (history_index < history_count - 1) {
                        history_index++;
                        
                        if (history[history_index]) {
                            // Clear current line
                            printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                            
                            // Copy history entry to buffer
                            strncpy(buffer, history[history_index], bufsize - 1);
                            buffer[bufsize - 1] = '\0';
                            position = strlen(buffer);
                            cursor_pos = position;
                            
                            // Display new command
                            printf("%s%s", build_prompt(), buffer);
                        }
                    } else {
                        // Reached end of history, restore original input
                        if (original_buffer) {
                            // Clear current line
                            printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                            
                            strncpy(buffer, original_buffer, bufsize - 1);
                            buffer[bufsize - 1] = '\0';
                            position = strlen(buffer);
                            cursor_pos = position;
                            
                            // Display original input
                            printf("%s%s", build_prompt(), buffer);
                            
                            free(original_buffer);
                            original_buffer = NULL;
                        }
                        history_index = -1;
                    }
                }
                continue;
            }
            
            if (c == ARROW_LEFT) {
                // Move cursor left
                if (cursor_pos > 0) {
                    cursor_pos--;
                    printf("\b"); // Move cursor back one position
                }
                continue;
            }
            
            if (c == ARROW_RIGHT) {
                // Move cursor right
                if (cursor_pos < position) {
                    cursor_pos++;
                    printf("%c", buffer[cursor_pos - 1]); // Print the character at new position
                }
                continue;
            }
            
            // For other extended keys, just continue
            continue;
        }
        
        // Reset history navigation when user types something
        if (c != EXTENDED_KEY && history_index != -1) {
            history_index = -1;
            if (original_buffer) {
                free(original_buffer);
                original_buffer = NULL;
            }
        }
        
        if (c == TAB_KEY) {
            buffer[position] = '\0'; // Null-terminate the current input

            // Find the word at cursor position for completion
            char completion_word[XSH_MAXLINE];
            int word_start = cursor_pos;
            int word_end = cursor_pos;
            
            // Find the start of the current word (go backwards from cursor)
            while (word_start > 0 && buffer[word_start - 1] != ' ' && buffer[word_start - 1] != '\t') {
                word_start--;
            }
            
            // Find the end of the current word (go forwards from cursor)
            while (word_end < position && buffer[word_end] != ' ' && buffer[word_end] != '\t') {
                word_end++;
            }
            
            // Extract the word to complete
            int word_len = word_end - word_start;
            if (word_len < XSH_MAXLINE - 1) {
                strncpy(completion_word, buffer + word_start, word_len);
                completion_word[word_len] = '\0';
            } else {
                completion_word[0] = '\0';
            }

            char* completion = complete_command(completion_word);
            if (completion) {
                // Replace the current word with the completion
                int completion_len = strlen(completion);
                int new_position = position - word_len + completion_len;
                int new_cursor_pos = cursor_pos - (cursor_pos - word_start) + completion_len;
                
                // Shift the rest of the buffer
                if (completion_len != word_len) {
                    memmove(buffer + word_start + completion_len, 
                           buffer + word_end, 
                           position - word_end + 1); // +1 for null terminator
                }
                
                // Insert the completion
                memcpy(buffer + word_start, completion, completion_len);
                position = new_position;
                cursor_pos = new_cursor_pos;
                
                // Redraw the entire line
                printf("\r%s%s", build_prompt(), buffer);
                
                // Move cursor to correct position
                for (int i = position; i > cursor_pos; i--) {
                    printf("\b");
                }
                
                free(completion);
            } else {
                int match_count = 0;
                char** matches = find_matches(completion_word, &match_count);

                if (match_count > 0) {
                    printf("\n");
                    display_matches(matches, match_count);

                    // Redisplay prompt and current input
                    printf("\n%s%s", build_prompt(), buffer);
                    
                    // Move cursor to correct position
                    for (int i = position; i > cursor_pos; i--) {
                        printf("\b");
                    }

                    // Free memory for matches
                    for (int i = 0; i < match_count; i++) {
                        free(matches[i]); // Free each match
                    }
                    free(matches); // Free the array of matches
                }
            }
            continue;
        }

        if (c == '\r' || c == '\n') { // Enter key
            _putch('\n'); // Echo a newline
            buffer[position] = '\0'; // Null-terminate the input
            
            // Clean up history navigation state
            if (original_buffer) {
                free(original_buffer);
                original_buffer = NULL;
            }
            history_index = -1;
            
            return buffer; // Return the input line
        }

        if (c == '\b' && cursor_pos > 0) { // Backspace
            // Move all characters from cursor_pos to the left
            for (int i = cursor_pos - 1; i < position - 1; i++) {
                buffer[i] = buffer[i + 1];
            }
            position--;
            cursor_pos--;
            
            // Redraw the line from cursor position
            printf("\b");
            for (int i = cursor_pos; i < position; i++) {
                _putch(buffer[i]);
            }
            _putch(' '); // Clear the last character
            
            // Move cursor back to correct position
            for (int i = position; i >= cursor_pos; i--) {
                printf("\b");
            }
            
            buffer[position] = '\0'; // Null-terminate at new position
            continue;
        }
        
        // For other printable characters
        if (isprint(c)) {
            // Insert character at cursor position
            if (cursor_pos < position) {
                // Shift characters to the right
                for (int i = position; i > cursor_pos; i--) {
                    buffer[i] = buffer[i - 1];
                }
            }
            
            buffer[cursor_pos] = c;
            position++;
            cursor_pos++;
            
            // Redraw from cursor position to end
            for (int i = cursor_pos - 1; i < position; i++) {
                _putch(buffer[i]);
            }
            
            // Move cursor back to correct position
            for (int i = position; i > cursor_pos; i--) {
                printf("\b");
            }
        }

        // Common buffer resizing logic
        if (position >= bufsize -1) { // -1 to ensure space for null terminator
            bufsize += XSH_RL_BUFSIZE; // Increase buffer size
            char *new_buffer = realloc(buffer, sizeof(char) * bufsize);
            if (!new_buffer) {
                fprintf(stderr, "xsh: allocation error\n");
                free(buffer); // Free old buffer before exiting
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
        // Ensure buffer is null-terminated after character addition (important for tab completion logic)
        buffer[position] = '\0'; 
    }

#elif __linux__ // POSIX systems (specifically targeting Linux for termios)
    static struct termios old_tio, new_tio;
    int is_tty = isatty(STDIN_FILENO);
    int cursor_pos = 0; // Current cursor position in the buffer
    
    // History navigation variables
    static int history_index = -1; // -1 means no history navigation active
    static char *original_buffer = NULL; // Store original input when navigating history

    if (is_tty) {
        tcgetattr(STDIN_FILENO, &old_tio); // Get current terminal attributes
        new_tio = old_tio;                 // Copy old attributes to new
        new_tio.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        new_tio.c_cc[VMIN] = 1;            // Read one character at a time
        new_tio.c_cc[VTIME] = 0;           // No timeout
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio); // Set new attributes
    }

    while (1) {
        if (is_tty) {
            char ch;
            ssize_t n = read(STDIN_FILENO, &ch, 1); // Read one character
            if (n <= 0) { // Error or EOF
                if (is_tty) tcsetattr(STDIN_FILENO, TCSANOW, &old_tio); // Restore terminal
                buffer[position] = '\0';
                // If EOF and buffer is empty, treat as exit or handle appropriately
                if (n == 0 && position == 0) { free(buffer); return NULL; } 
                return buffer;
            }
            c = ch;
        } else { // Not a TTY, use getchar for basic line reading
            c = getchar();
            if (c == EOF) {
                buffer[position] = '\0';
                if (position == 0) { free(buffer); return NULL; } // EOF on empty line
                return buffer;
            }
        }

        // Handle arrow keys (escape sequences on Linux)
        if (c == ESC_SEQUENCE && is_tty) {
            char seq1, seq2;
            if (read(STDIN_FILENO, &seq1, 1) == 1 && seq1 == '[' && read(STDIN_FILENO, &seq2, 1) == 1) {
                if (seq2 == ARROW_UP) {
                    // Navigate up in history
                    if (history_count > 0) {
                        if (history_index == -1) {
                            // First time navigating, save current input
                            if (original_buffer) free(original_buffer);
                            original_buffer = strdup(buffer);
                            history_index = history_count - 1;
                        } else if (history_index > 0) {
                            history_index--;
                        }
                        
                        if (history_index >= 0 && history_index < history_count && history[history_index]) {
                            // Clear current line
                            printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                            
                            // Copy history entry to buffer
                            strncpy(buffer, history[history_index], bufsize - 1);
                            buffer[bufsize - 1] = '\0';
                            position = strlen(buffer);
                            cursor_pos = position;
                            
                            // Display new command
                            printf("%s%s", build_prompt(), buffer);
                            fflush(stdout);
                        }
                    }
                    continue;
                }
                
                if (seq2 == ARROW_DOWN) {
                    // Navigate down in history
                    if (history_index != -1) {
                        if (history_index < history_count - 1) {
                            history_index++;
                            
                            if (history[history_index]) {
                                // Clear current line
                                printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                                
                                // Copy history entry to buffer
                                strncpy(buffer, history[history_index], bufsize - 1);
                                buffer[bufsize - 1] = '\0';
                                position = strlen(buffer);
                                cursor_pos = position;
                                
                                // Display new command
                                printf("%s%s", build_prompt(), buffer);
                                fflush(stdout);
                            }
                        } else {
                            // Reached end of history, restore original input
                            if (original_buffer) {
                                // Clear current line
                                printf("\r%*s\r", (int)(strlen(build_prompt()) + position), "");
                                
                                strncpy(buffer, original_buffer, bufsize - 1);
                                buffer[bufsize - 1] = '\0';
                                position = strlen(buffer);
                                cursor_pos = position;
                                
                                // Display original input
                                printf("%s%s", build_prompt(), buffer);
                                fflush(stdout);
                                
                                free(original_buffer);
                                original_buffer = NULL;
                            }
                            history_index = -1;
                        }
                    }
                    continue;
                }
                
                if (seq2 == ARROW_LEFT) {
                    // Move cursor left
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        printf("\b"); // Move cursor back one position
                        fflush(stdout);
                    }
                    continue;
                }
                
                if (seq2 == ARROW_RIGHT) {
                    // Move cursor right
                    if (cursor_pos < position) {
                        cursor_pos++;
                        printf("%c", buffer[cursor_pos - 1]); // Print the character at new position
                        fflush(stdout);
                    }
                    continue;
                }
            }
            // If not a valid arrow sequence, continue processing
        }
        
        // Reset history navigation when user types something
        if (c != ESC_SEQUENCE && history_index != -1) {
            history_index = -1;
            if (original_buffer) {
                free(original_buffer);
                original_buffer = NULL;
            }
        }

        if (c == TAB_KEY && is_tty) {
            buffer[position] = '\0';
            
            // Find the word at cursor position for completion
            char completion_word[XSH_MAXLINE];
            int word_start = cursor_pos;
            int word_end = cursor_pos;
            
            // Find the start of the current word (go backwards from cursor)
            while (word_start > 0 && buffer[word_start - 1] != ' ' && buffer[word_start - 1] != '\t') {
                word_start--;
            }
            
            // Find the end of the current word (go forwards from cursor)
            while (word_end < position && buffer[word_end] != ' ' && buffer[word_end] != '\t') {
                word_end++;
            }
            
            // Extract the word to complete
            int word_len = word_end - word_start;
            if (word_len < XSH_MAXLINE - 1) {
                strncpy(completion_word, buffer + word_start, word_len);
                completion_word[word_len] = '\0';
            } else {
                completion_word[0] = '\0';
            }
            
            char* completion = complete_command(completion_word);
            if (completion) {
                // Replace the current word with the completion
                int completion_len = strlen(completion);
                int new_position = position - word_len + completion_len;
                int new_cursor_pos = cursor_pos - (cursor_pos - word_start) + completion_len;
                
                // Shift the rest of the buffer
                if (completion_len != word_len) {
                    memmove(buffer + word_start + completion_len, 
                           buffer + word_end, 
                           position - word_end + 1); // +1 for null terminator
                }
                
                // Insert the completion
                memcpy(buffer + word_start, completion, completion_len);
                position = new_position;
                cursor_pos = new_cursor_pos;
                
                // Redraw the entire line
                printf("\r%s%s", build_prompt(), buffer);
                
                // Move cursor to correct position
                for (int i = position; i > cursor_pos; i--) {
                    printf("\b");
                }
                
                fflush(stdout);
                free(completion);
            } else {
                int match_count = 0;
                char** matches = find_matches(completion_word, &match_count);
                if (match_count > 0) {
                    printf("\n"); // Newline before showing matches
                    display_matches(matches, match_count);
                    printf("\n%s%s", build_prompt(), buffer); // Redisplay prompt and current input
                    
                    // Move cursor to correct position
                    for (int i = position; i > cursor_pos; i--) {
                        printf("\b");
                    }
                    
                    fflush(stdout);
                    for (int i = 0; i < match_count; i++) free(matches[i]);
                    free(matches);
                } else {
                     putchar('\a'); // Bell for no completion
                     fflush(stdout);
                }
            }
            continue;
        } else if (c == '\n') {
            if (is_tty) {
                putchar('\n');
                fflush(stdout);
                tcsetattr(STDIN_FILENO, TCSANOW, &old_tio); // Restore terminal
            }
            buffer[position] = '\0';
            
            // Clean up history navigation state
            if (original_buffer) {
                free(original_buffer);
                original_buffer = NULL;
            }
            history_index = -1;
            
            return buffer;
        } else if ((c == 127 || c == '\b') && cursor_pos > 0 && is_tty) { // Backspace (127 is DEL)
            // Move all characters from cursor_pos to the left
            for (int i = cursor_pos - 1; i < position - 1; i++) {
                buffer[i] = buffer[i + 1];
            }
            position--;
            cursor_pos--;
            
            // Redraw the line from cursor position
            printf("\b");
            for (int i = cursor_pos; i < position; i++) {
                putchar(buffer[i]);
            }
            putchar(' '); // Clear the last character
            
            // Move cursor back to correct position
            for (int i = position; i >= cursor_pos; i--) {
                printf("\b");
            }
            
            fflush(stdout);
            buffer[position] = '\0';
        } else if (isprint(c)) {
            // Insert character at cursor position
            if (cursor_pos < position) {
                // Shift characters to the right
                for (int i = position; i > cursor_pos; i--) {
                    buffer[i] = buffer[i - 1];
                }
            }
            
            buffer[cursor_pos] = c;
            position++;
            cursor_pos++;
            
            if (is_tty) {
                // Redraw from cursor position to end
                for (int i = cursor_pos - 1; i < position; i++) {
                    putchar(buffer[i]);
                }
                
                // Move cursor back to correct position
                for (int i = position; i > cursor_pos; i--) {
                    printf("\b");
                }
                
                fflush(stdout);
            }
        } else if (c == EOF && !is_tty) { // Handle EOF for non-tty case if getchar() returned it
             buffer[position] = '\0';
             if (position == 0) { free(buffer); return NULL; }
             return buffer;
        }


        // Common buffer resizing logic
        if (position >= bufsize - 1) {
            bufsize += XSH_RL_BUFSIZE;
            char *new_buffer = realloc(buffer, sizeof(char) * bufsize);
            if (!new_buffer) {
                fprintf(stderr, "xsh: allocation error\n");
                if (is_tty) tcsetattr(STDIN_FILENO, TCSANOW, &old_tio); // Restore on error
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
        buffer[position] = '\0'; // Keep buffer null-terminated
    }
    // Should not be reached if is_tty, but as a fallback
    if (is_tty) tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
#else  // Other POSIX or fallback (original simple getchar loop)
    // This part is reached if not _WIN32 and not __linux__
    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            if (c == EOF && position == 0) { free(buffer); return NULL; }
            return buffer;
        }
        buffer[position] = c;
        position++;

        if (position >= bufsize - 1) {
            bufsize += XSH_RL_BUFSIZE;
            char *new_buffer = realloc(buffer, bufsize); // No sizeof(char) needed for realloc with bytes
            if (!new_buffer) {
                fprintf(stderr, "xsh: allocation error\n");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
        buffer[position] = '\0';
    }
#endif
    return buffer; // Should be unreachable if logic is correct
}

char** find_matches(const char* partial, int* match_count) {
    *match_count = 0;
    
    // Use enhanced smart completion system
    char** smart_matches = get_smart_completions(partial, match_count);
    if (smart_matches && *match_count > 0) {
        return smart_matches;
    }
    
    // Enhanced file completion system
    int bufsize = XSH_TOK_BUFSIZE;
    char** matches = malloc(bufsize * sizeof(char*));
    size_t partial_len = strlen(partial);
    
    if (!matches) {
        fprintf(stderr, "xsh: allocation error in find_matches\n");
        exit(EXIT_FAILURE);
    }
    
    *match_count = 0;
    
    // Determine if we're completing a command or a file/path
    // Since we're now called with just the word being completed,
    // we need to be smarter about this detection
    int is_command_completion = 1;
    
    // Check if this looks like a file path
    if (strchr(partial, '.') || strchr(partial, '/') || strchr(partial, '\\') || 
        partial[0] == '.' || partial[0] == '~') {
        is_command_completion = 0;
    }
    
    // Also check if partial starts with common file prefixes
    if (strncmp(partial, "./", 2) == 0 || strncmp(partial, "../", 3) == 0 ||
        strncmp(partial, ".\\", 2) == 0 || strncmp(partial, "..\\", 3) == 0) {
        is_command_completion = 0;
    }
    
    // If the partial is empty, prefer file completion over command completion
    // This helps when completing arguments after commands
    if (partial_len == 0) {
        is_command_completion = 0;
    }
    
    // Always try file completion first, then commands
    // Enhanced file/directory completion
    DIR* dir;
    struct dirent* entry;
    char path[XSH_MAXLINE] = "."; // Default to current directory
    char prefix[XSH_MAXLINE] = ""; // What to search for in directory
    char full_partial[XSH_MAXLINE];
    
    // Copy partial to work with it
    strncpy(full_partial, partial, XSH_MAXLINE - 1);
    full_partial[XSH_MAXLINE - 1] = '\0';
    
    // Handle different path scenarios
    const char* last_slash = strrchr(full_partial, '/');
    #ifdef _WIN32
    const char* last_backslash = strrchr(full_partial, '\\');
    if (!last_slash || (last_backslash && last_backslash > last_slash)) {
        last_slash = last_backslash;
    }
    #endif
    
    if (last_slash) {
        // Path contains directory separator
        size_t path_len = last_slash - full_partial;
        if (path_len < XSH_MAXLINE - 1) {
            strncpy(path, full_partial, path_len);
            path[path_len] = '\0';
        }
        if (strlen(last_slash + 1) < XSH_MAXLINE - 1) {
            strcpy(prefix, last_slash + 1);
        }
    } else {
        // No directory separator, search in current directory
        if (strlen(full_partial) < XSH_MAXLINE - 1) {
            strcpy(prefix, full_partial);
        }
    }
    
    // Handle special cases for path
    if (strlen(path) == 0 || strcmp(path, "") == 0) {
        strcpy(path, ".");
    }
    
    // Handle home directory expansion
    if (path[0] == '~') {
        char expanded_path[XSH_MAXLINE];
        const char* home = getenv("HOME");
        #ifdef _WIN32
        if (!home) home = getenv("USERPROFILE");
        #endif
        if (home) {
            if (strlen(path) == 1) {
                // Just "~"
                strncpy(expanded_path, home, XSH_MAXLINE - 1);
            } else {
                // "~/something"
                snprintf(expanded_path, XSH_MAXLINE, "%s%s", home, path + 1);
            }
            expanded_path[XSH_MAXLINE - 1] = '\0';
            strcpy(path, expanded_path);
        }
    }
    
    dir = opendir(path);
    if (dir) {
        size_t prefix_len = strlen(prefix);
        
        while ((entry = readdir(dir)) != NULL) {
            // Skip "." and ".." unless explicitly requested
            if ((strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) && 
                prefix_len == 0) {
                continue;
            }
            
            // Skip hidden files unless explicitly requested
            if (entry->d_name[0] == '.' && prefix_len > 0 && prefix[0] != '.') {
                continue;
            }
            
            // Check if the entry matches our prefix
            if (prefix_len == 0 || strncmp(prefix, entry->d_name, prefix_len) == 0) {
                char full_path[XSH_MAXLINE];
                
                // Build the full completion path
                if (last_slash) {
                    snprintf(full_path, XSH_MAXLINE, "%.*s%s", 
                             (int)(last_slash - full_partial + 1), full_partial, entry->d_name);
                } else {
                    strncpy(full_path, entry->d_name, XSH_MAXLINE - 1);
                    full_path[XSH_MAXLINE - 1] = '\0';
                }
                
                // Check if it's a directory and add trailing slash
                char actual_path[XSH_MAXLINE];
                if (strcmp(path, ".") == 0) {
                    strcpy(actual_path, entry->d_name);
                } else {
                    snprintf(actual_path, XSH_MAXLINE, "%s/%s", path, entry->d_name);
                }
                
                struct stat statbuf;
                if (stat(actual_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                    // It's a directory, add trailing slash
                    size_t len = strlen(full_path);
                    if (len < XSH_MAXLINE - 2) {
                        #ifdef _WIN32
                        strcat(full_path, "\\");
                        #else
                        strcat(full_path, "/");
                        #endif
                    }
                }
                
                // Avoid duplicates
                int duplicate = 0;
                for (int j = 0; j < *match_count; j++) {
                    if (strcmp(matches[j], full_path) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                
                if (!duplicate) {
                    matches[*match_count] = strdup(full_path);
                    if (!matches[*match_count]) { 
                        fprintf(stderr, "xsh: strdup error\n"); 
                        exit(EXIT_FAILURE); 
                    }
                    (*match_count)++;
                    
                    if (*match_count >= bufsize) {
                        bufsize += XSH_TOK_BUFSIZE;
                        matches = realloc(matches, bufsize * sizeof(char*));
                        if (!matches) {
                            fprintf(stderr, "xsh: allocation error in find_matches\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
        closedir(dir);
    }
    
    // Try built-in commands only if it looks like command completion and we have few file matches
    if (is_command_completion && *match_count < 10) {
        for (int i = 0; i < xsh_num_builtins(); i++) {
            if (strncmp(partial, builtin_str[i], partial_len) == 0) {
                // Avoid duplicates
                int duplicate = 0;
                for (int j = 0; j < *match_count; j++) {
                    if (strcmp(matches[j], builtin_str[i]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                
                if (!duplicate) {
                    matches[*match_count] = strdup(builtin_str[i]);
                    if (!matches[*match_count]) { 
                        fprintf(stderr, "xsh: strdup error\n"); 
                        exit(EXIT_FAILURE); 
                    }
                    (*match_count)++;
                    
                    if (*match_count >= bufsize) {
                        bufsize += XSH_TOK_BUFSIZE;
                        matches = realloc(matches, bufsize * sizeof(char*));
                        if (!matches) {
                            fprintf(stderr, "xsh: allocation error in find_matches\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
    }
    
    // Add command history as fallback (only if we have few matches and it looks like a command)
    if (*match_count < 5 && is_command_completion) {
        for (int i = 0; i < history_count; i++) {
            if (strncmp(partial, history[i], partial_len) == 0) {
                // Avoid duplicating matches already found
                int duplicate = 0;
                for (int j = 0; j < *match_count; j++) {
                    if (strcmp(matches[j], history[i]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (duplicate) continue;

                matches[*match_count] = strdup(history[i]);
                if (!matches[*match_count]) { 
                    fprintf(stderr, "xsh: strdup error\n"); 
                    exit(EXIT_FAILURE); 
                }
                (*match_count)++;
                
                if (*match_count >= bufsize) {
                    bufsize += XSH_TOK_BUFSIZE;
                    matches = realloc(matches, bufsize * sizeof(char*));
                    if (!matches) {
                        fprintf(stderr, "xsh: allocation error in find_matches\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    
    return matches;
}

void display_matches(char** matches, int match_count) {
    if (match_count == 0) return;
    
    printf("\nPossible completions:\n");
    
    size_t max_len = 0;
    for (int i = 0; i < match_count; i++) {
        size_t len = strlen(matches[i]);
        if (len > max_len) max_len = len;
    }
    
    int cols = 80 / (max_len + 2); // +2 for spacing
    if (cols == 0) cols = 1;
    
    for (int i = 0; i < match_count; i++) {
        printf("%-*s", (int)(max_len + 2), matches[i]);
        if ((i + 1) % cols == 0 || i == match_count - 1) {
            printf("\n");
        }
    }
}

char* complete_command(const char* partial) {
    int match_count = 0;
    char** matches = find_matches(partial, &match_count);
    char* result = NULL;

    if (match_count == 0) {
        free(matches);
        return NULL;
    }
    
    if (match_count == 1) {
        result = strdup(matches[0]);
        if (!result) { fprintf(stderr, "xsh: strdup error\n"); exit(EXIT_FAILURE); }
    } else {
        // Multiple matches, find common prefix
        size_t common_len = strlen(matches[0]);
        for (int i = 1; i < match_count; i++) {
            size_t j = 0;
            while(j < common_len && j < strlen(matches[i]) && matches[0][j] == matches[i][j]) {
                j++;
            }
            common_len = j; // Update common_len to the new shortest common part
        }
        
        if (common_len > strlen(partial)) {
            result = malloc(common_len + 1);
            if (!result) {
                fprintf(stderr, "xsh: allocation error in complete_command\n");
                exit(EXIT_FAILURE);
            }
            strncpy(result, matches[0], common_len);
            result[common_len] = '\0';
        }
    }
    
    // Clean up matches
    for (int i = 0; i < match_count; i++) {
        free(matches[i]);
    }
    free(matches);
    
    return result;
}

char **xsh_split_line(char *line) {
    int bufsize = XSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "xsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, XSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += XSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "xsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, XSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}
