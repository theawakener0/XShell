// Hi,today we will be looking at how to implement a simple shell in C.
// Yes, we will be implementing a simple shell in C.
// How? I do not know yet, but we will figure it out together.

// let the journey begin...

#include "xsh.h"
#include "input.h"
#include "builtins.h"
#include "execute.h"
#include "history.h"
#include "utils.h" // For print_slow, build_prompt
#include "config.h" // For configuration management
#include <time.h> // For clock timing

#ifdef _WIN32
#include <windows.h> // For enabling ANSI escape codes
#include <direct.h> // For _getcwd
#else
#include <unistd.h> // For getcwd
#endif

void xsh_banner(void) {
    // Get terminal width (default to 80 if we can't determine it)
    int term_width = 80;
    
    // ASCII Art Logo with proper centering
    const char *logo[] = {
        " __  ______  _          _ _ ",
        " \\ \\/ / ___|| |__   ___| | |",
        "  \\  /\\___ \\| '_ \\ / _ \\ | |",
        "  /  \\ ___) | | | |  __/ | |",
        " /_/\\_\\____/|_| |_|\\___|_|_|"
    };
    
    printf("\n");
    
    // Center and print the logo
    for (int i = 0; i < 5; i++) {
        int padding = (term_width - strlen(logo[i])) / 2;
        printf("%*s%s\n", padding, "", logo[i]);
    }
    
    printf("\n");
    
    // Center the header line
    const char *header = "====================================================================";
    int header_padding = (term_width - strlen(header)) / 2;
    printf("%*s%s\n", header_padding, "", header);
    
    // Center the title
    const char *title = "XShell - A Powerful Shell Implementation in C";
    int title_padding = (term_width - strlen(title)) / 2;
    printf("%*s%s\n", title_padding, "", title);
    
    // Center the footer line
    printf("%*s%s\n\n", header_padding, "", header);
    
    // Center each feature line
    const char *features[] = {
        "FEATURES:",
        "- Command history tracking and recall",
        "- Built-in commands (help, cd, exit, history)",
        "- External command execution",
        "- Dynamic prompt with system information",
        "",
        "Type 'help' to see all available commands"
    };
    
    for (int i = 0; i < 7; i++) {
        int feat_padding = (term_width - strlen(features[i])) / 2;
        printf("%*s%s\n", feat_padding, "", features[i]);
    }
    
    // Center the closing line
    printf("\n%*s%s\n\n", header_padding, "", header);
}

// Main shell loop
void xsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        char *prompt = build_prompt(); // Get dynamic prompt
        printf("%s", prompt);
        fflush(stdout); // Ensure prompt is displayed immediately
        line = xsh_read_line();
        add_to_history(line); // Add command to history
        
        // Get current working directory for enhanced history
        char current_dir[1024];
#ifdef _WIN32
        _getcwd(current_dir, sizeof(current_dir));
#else
        getcwd(current_dir, sizeof(current_dir));
#endif
        
        // Record start time for execution timing
        clock_t start_time = clock();
        
        // Check if line contains operators before splitting
        if (contains_operators(line)) {
            // Use advanced parsing for complex commands
            int token_count;
            char **tokens = tokenize_with_operators(line, &token_count);
            if (tokens) {
                pipeline_t *pipeline = parse_command_line(tokens, token_count);
                if (pipeline) {
                    int exit_status = execute_pipeline(pipeline);
                    
                    // Calculate execution time
                    clock_t end_time = clock();
                    long execution_time_ms = ((end_time - start_time) * 1000) / CLOCKS_PER_SEC;
                    
                    // Add to enhanced history with execution data
                    add_to_enhanced_history(line, current_dir, exit_status, execution_time_ms);
                    
                    // Check if any command in the pipeline was 'exit'
                    command_t *cmd = pipeline->commands;
                    int should_exit = 0;
                    while (cmd) {
                        if (cmd->args && cmd->args[0] && strcmp(cmd->args[0], "exit") == 0) {
                            should_exit = 1;
                            break;
                        }
                        cmd = cmd->next;
                    }
                    free_pipeline(pipeline);
                    status = should_exit ? 0 : 1; // 0 = exit shell, 1 = continue
                } else {
                    // Parse error
                    add_to_enhanced_history(line, current_dir, -1, 0);
                    status = 1; // Continue shell loop on parse error
                }
                
                // Free tokens
                for (int i = 0; i < token_count; i++) {
                    free(tokens[i]);
                }
                free(tokens);
            } else {
                // Tokenization error
                add_to_enhanced_history(line, current_dir, -1, 0);
                status = 1; // Continue shell loop on tokenization error
            }
        } else {
            // Simple command - use traditional parsing
            args = xsh_split_line(line);
            int exit_status = xsh_execute(args); // xsh_execute will handle builtins or launch
            
            // Calculate execution time
            clock_t end_time = clock();
            long execution_time_ms = ((end_time - start_time) * 1000) / CLOCKS_PER_SEC;
            
            // Add to enhanced history with execution data
            add_to_enhanced_history(line, current_dir, (exit_status == 1) ? 0 : exit_status, execution_time_ms);
            
            status = exit_status;
            free(args);
        }

        free(line);
    } while (status);
}

// Execute command: checks for built-ins first, then launches external command
int xsh_execute(char **args) {
    if (args[0] == NULL) {
        // An empty command was entered.
        return 1; // Continue shell loop
    }

    // Simple command - check for built-ins first
    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    // External command
    return xsh_launch(args);
}

int main(int argc, char **argv) {
#ifdef _WIN32
    // Enable virtual terminal processing for ANSI escape codes on Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
    // Load configuration files
    if (config_load_all_files() != 0) {
        fprintf(stderr, "Warning: Some configuration files could not be loaded, using defaults\n");
    }
    
    // Initialize enhanced history system
    if (init_history_system() != 0) {
        fprintf(stderr, "Warning: Failed to initialize history system\n");
    }
    
    // Display startup banner if enabled
    int startup_banner = config_get_bool(&xshell_config, "startup_banner", 1);
    if (startup_banner) {
        xsh_banner();
    }

    // Run command loop.
    xsh_loop();

    // Perform any shutdown/cleanup.
    // Cleanup enhanced history system
    cleanup_history_system();
    
    // Free configuration memory
    config_free(&xshell_config);
    config_free(&xcodex_config);

    return EXIT_SUCCESS;
}

