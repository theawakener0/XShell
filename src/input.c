#include "input.h"
#include "xsh.h" // For build_prompt, history, etc.

// Implementation of input handling functions

char *xsh_read_line(void){
    int bufsize = XSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    // int tab_pressed = 0; // tab_pressed was declared but not used effectively, consider removing or implementing fully

    if (!buffer) {
        fprintf(stderr, "xsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0'; // Initialize buffer to empty string

    while (1) {
#ifdef _WIN32
        c = _getch(); // Use _getch() to get a character without echoing it
        
        if (c == TAB_KEY) {
            // tab_pressed = 1; // Mark tab as pressed

            buffer[position] = '\0'; // Null-terminate the current input

            char* completion = complete_command(buffer);
            if (completion) {
                // Clear current line and print the prompt and completed command
                printf("\r%s%s", build_prompt(), completion);

                strcpy(buffer, completion); // Copy the completed command back to buffer
                position = strlen(buffer); // Update position to the end of the completed command
                free(completion); // Free the allocated memory for completion
            } else {
                int match_count = 0;
                char** matches = find_matches(buffer, &match_count);

                if (match_count > 0) {
                    printf("\n");
                    display_matches(matches, match_count);

                    // Redisplay prompt and current input
                    printf("\n%s%s", build_prompt(), buffer);

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
            return buffer; // Return the input line
        }

        if (c == '\b' && position > 0) { // Backspace
            position--;
            _putch('\b'); // Move cursor back
            _putch(' '); // Overwrite with space
            _putch('\b'); // Move cursor back again
            buffer[position] = '\0'; // Null-terminate at new position
            continue;
        }
        
        // For other printable characters
        if (isprint(c)) {
             _putch(c); // Echo the character
            buffer[position] = c; // Store the character in the buffer
            position++;
        }

#else // POSIX systems
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0'; // Null-terminate the input
            return buffer; // Return the input line
        }
        // Basic POSIX input, tab completion and advanced line editing not implemented here for simplicity
        // This part would need significant work for feature parity with Windows _getch loop
        buffer[position] = c; // Store the character in the buffer
        position++;
#endif

        // Common buffer resizing logic for both platforms
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
}

char** find_matches(const char* partial, int* match_count) {
    int bufsize = XSH_TOK_BUFSIZE;
    char** matches = malloc(bufsize * sizeof(char*));
    size_t partial_len = strlen(partial);
    
    if (!matches) {
        fprintf(stderr, "xsh: allocation error in find_matches\n");
        exit(EXIT_FAILURE);
    }
    
    *match_count = 0;
    
    // First check built-in commands
    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strncmp(partial, builtin_str[i], partial_len) == 0) {
            matches[*match_count] = strdup(builtin_str[i]);
            if (!matches[*match_count]) { fprintf(stderr, "xsh: strdup error\n"); exit(EXIT_FAILURE); }
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
    
    // Check files in current directory if no command matches or if there's a path prefix
    // This part needs dirent.h included via xsh.h
    if (*match_count == 0 || strchr(partial, '/') || strchr(partial, '\\')) {
        DIR* dir;
        struct dirent* entry;
        char path[XSH_MAXLINE] = "."; // Default to current directory
        char prefix[XSH_MAXLINE] = ""; // What to search for in directory
        
        const char* last_slash = strrchr(partial, '/');
        if (!last_slash) last_slash = strrchr(partial, '\\');
        
        if (last_slash) {
            size_t path_len = last_slash - partial;
            if (path_len < XSH_MAXLINE) {
                strncpy(path, partial, path_len);
                path[path_len] = '\0';
            }
            if (strlen(last_slash + 1) < XSH_MAXLINE) {
                strcpy(prefix, last_slash + 1);
            }
        } else {
            if (strlen(partial) < XSH_MAXLINE) {
                strcpy(prefix, partial);
            }
        }
        
        dir = opendir(path);
        if (dir) {
            size_t prefix_len = strlen(prefix);
            while ((entry = readdir(dir)) != NULL) {
                if (strncmp(prefix, entry->d_name, prefix_len) == 0) {
                    char full_path[XSH_MAXLINE];
                    if (last_slash) {
                        snprintf(full_path, XSH_MAXLINE, "%.*s%s", (int)(last_slash - partial + 1), partial, entry->d_name);
                    } else {
                        strncpy(full_path, entry->d_name, XSH_MAXLINE -1);
                        full_path[XSH_MAXLINE -1] = '\0';
                    }
                    
                    matches[*match_count] = strdup(full_path);
                    if (!matches[*match_count]) { fprintf(stderr, "xsh: strdup error\n"); exit(EXIT_FAILURE); }
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
            closedir(dir);
        }
    }
    
    // Check command history
    // Ensure history and history_count are accessible (e.g. extern in xsh.h)
    for (int i = 0; i < history_count; i++) {
        if (strncmp(partial, history[i], partial_len) == 0) {
            // Avoid duplicating matches already found (e.g. from builtins or files)
            int duplicate = 0;
            for (int j=0; j < *match_count; ++j) {
                if (strcmp(matches[j], history[i]) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) continue;

            matches[*match_count] = strdup(history[i]);
            if (!matches[*match_count]) { fprintf(stderr, "xsh: strdup error\n"); exit(EXIT_FAILURE); }
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
