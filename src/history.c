#include "history.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define PATH_SEPARATOR "\\"

// Windows doesn't have strndup, so provide our own implementation
char* strndup(const char* s, size_t n) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (n < len) len = n;
    char* result = malloc(len + 1);
    if (result) {
        memcpy(result, s, len);
        result[len] = '\0';
    }
    return result;
}

#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

// Define history array and count here
char *history[XSH_HISTORY_SIZE];
int history_count = 0;

// Enhanced history system
history_entry_t *enhanced_history = NULL;
int enhanced_history_count = 0;
int enhanced_history_capacity = 0;

// Command statistics for smart completion
command_stat_t *command_stats = NULL;
int command_stats_count = 0;
int command_stats_capacity = 0;

// Command pattern tracking for context-aware completion
command_pattern_t *command_patterns = NULL;
int command_patterns_count = 0;
int command_patterns_capacity = 0;

// Recent command context (for pattern learning)
static char *recent_commands[MAX_COMMAND_CONTEXT];
static int recent_commands_count = 0;

// Get the path to the history file
char* get_history_file_path(void) {
    static char history_path[1024];
    
#ifdef _WIN32
    char *home = getenv("USERPROFILE");
    if (!home) home = getenv("HOME");
    if (!home) {
        // Fallback to current directory
        strcpy(history_path, HISTORY_FILE_NAME);
        return history_path;
    }
    snprintf(history_path, sizeof(history_path), "%s%s%s", home, PATH_SEPARATOR, HISTORY_FILE_NAME);
#else
    char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : ".";
    }
    snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE_NAME);
#endif
    
    return history_path;
}

// Get the path to the history metadata file
char* get_history_metadata_file_path(void) {
    static char metadata_path[1024];
    
#ifdef _WIN32
    char *home = getenv("USERPROFILE");
    if (!home) home = getenv("HOME");
    if (!home) {
        // Fallback to current directory
        strcpy(metadata_path, HISTORY_METADATA_FILE);
        return metadata_path;
    }
    snprintf(metadata_path, sizeof(metadata_path), "%s%s%s", home, PATH_SEPARATOR, HISTORY_METADATA_FILE);
#else
    char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : ".";
    }
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", home, HISTORY_METADATA_FILE);
#endif
    
    return metadata_path;
}

// Add command to basic history array
void add_to_history(const char *line) {
    if (!line || strlen(line) == 0) return;
    
    // Add to basic history array
    if (history_count < XSH_HISTORY_SIZE) {
        history[history_count] = strdup(line);
        if (history[history_count]) {
            history_count++;
            
            // Also update command statistics
            update_command_stats(line);
        }
    } else {
        // Shift history array to make room for new entry
        if (history[0]) free(history[0]);
        for (int i = 0; i < XSH_HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[XSH_HISTORY_SIZE - 1] = strdup(line);
        if (history[XSH_HISTORY_SIZE - 1]) {
            update_command_stats(line);
        }
    }
}

// Display command history
void display_history(void) {
    printf("=== Command History ===\n");
    if (history_count == 0) {
        printf("No commands in history.\n");
        return;
    }
    
    for (int i = 0; i < history_count; i++) {
        if (history[i]) {
            printf("%4d  %s\n", i + 1, history[i]);
        }
    }
    printf("\nTotal commands: %d\n", history_count);
}

// Initialize the history system
int init_history_system(void) {
    // Initialize command stats array
    command_stats_capacity = 100;
    command_stats = malloc(sizeof(command_stat_t) * command_stats_capacity);
    if (!command_stats) {
        fprintf(stderr, "xsh: Failed to allocate memory for command stats\n");
        return -1;
    }
    command_stats_count = 0;
    
    // Initialize enhanced history array
    enhanced_history_capacity = 1000;
    enhanced_history = malloc(sizeof(history_entry_t) * enhanced_history_capacity);
    if (!enhanced_history) {
        fprintf(stderr, "xsh: Failed to allocate memory for enhanced history\n");
        return -1;
    }
    enhanced_history_count = 0;
    
    // Initialize command patterns array
    command_patterns_capacity = 200;
    command_patterns = malloc(sizeof(command_pattern_t) * command_patterns_capacity);
    if (!command_patterns) {
        fprintf(stderr, "xsh: Failed to allocate memory for command patterns\n");
        return -1;
    }
    command_patterns_count = 0;
    
    // Initialize recent commands context
    for (int i = 0; i < MAX_COMMAND_CONTEXT; i++) {
        recent_commands[i] = NULL;
    }
    recent_commands_count = 0;
    
    // Load history from files
    if (load_history_from_file() < 0) {
        fprintf(stderr, "xsh: Warning - Could not load history file\n");
    }
    
    if (load_enhanced_history() < 0) {
        fprintf(stderr, "xsh: Warning - Could not load enhanced history file\n");
    }
    
    return 0;
}

// Load history from file
int load_history_from_file(void) {
    char *history_file = get_history_file_path();
    FILE *file = fopen(history_file, "r");
    
    if (!file) {
        // File doesn't exist yet, this is okay for first run
        return 0;
    }
    
    char line[XSH_MAXLINE];
    while (fgets(line, sizeof(line), file) && history_count < XSH_HISTORY_SIZE) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strlen(line) > 0) {
            history[history_count] = strdup(line);
            if (history[history_count]) {
                // Update command statistics
                update_command_stats(line);
                history_count++;
            }
        }
    }
    
    fclose(file);
    
    // Calculate initial scores for loaded commands
    calculate_command_scores();
    
    return history_count;
}

// Save history to file
int save_history_to_file(void) {
    char *history_file = get_history_file_path();
    FILE *file = fopen(history_file, "w");
    
    if (!file) {
        fprintf(stderr, "xsh: Cannot write to history file %s\n", history_file);
        return -1;
    }
    
    // Write all history entries
    for (int i = 0; i < history_count; i++) {
        if (history[i]) {
            fprintf(file, "%s\n", history[i]);
        }
    }
    
    fclose(file);
    return 0;
}

// Load enhanced history with metadata from JSON file
int load_enhanced_history(void) {
    char *metadata_file = get_history_metadata_file_path();
    FILE *file = fopen(metadata_file, "r");
    
    if (!file) {
        // File doesn't exist yet, this is okay for first run
        return 0;
    }
    
    // Simple JSON parsing for enhanced history
    // In a production system, you'd want to use a proper JSON library
    char line[XSH_MAXLINE];
    while (fgets(line, sizeof(line), file) && enhanced_history_count < enhanced_history_capacity) {
        // Skip JSON structure lines
        if (strstr(line, "\"command\":") != NULL) {
            // Extract command (simple parsing)
            char *start = strchr(line, '"');
            if (start) {
                start = strchr(start + 1, '"');
                if (start) {
                    start += 3; // Skip ": "
                    char *end = strchr(start, '"');
                    if (end) {
                        size_t len = end - start;
                        if (len > 0 && len < 256) {
                            enhanced_history[enhanced_history_count].command = strndup(start, len);
                            enhanced_history[enhanced_history_count].timestamp = time(NULL);
                            enhanced_history[enhanced_history_count].cwd = strdup(".");
                            enhanced_history[enhanced_history_count].exit_code = 0;
                            enhanced_history[enhanced_history_count].execution_time_ms = 0;
                            enhanced_history[enhanced_history_count].context_count = 0;
                            enhanced_history_count++;
                        }
                    }
                }
            }
        }
    }
    
    fclose(file);
    return enhanced_history_count;
}

// Save enhanced history to JSON file
int save_enhanced_history(void) {
    char *metadata_file = get_history_metadata_file_path();
    FILE *file = fopen(metadata_file, "w");
    
    if (!file) {
        fprintf(stderr, "xsh: Cannot write to enhanced history file %s\n", metadata_file);
        return -1;
    }
    
    fprintf(file, "{\n");
    fprintf(file, "  \"version\": \"1.0\",\n");
    fprintf(file, "  \"created\": %ld,\n", time(NULL));
    fprintf(file, "  \"history\": [\n");
    
    for (int i = 0; i < enhanced_history_count; i++) {
        if (enhanced_history[i].command) {
            fprintf(file, "    {\n");
            fprintf(file, "      \"command\": \"%s\",\n", enhanced_history[i].command);
            fprintf(file, "      \"timestamp\": %ld,\n", enhanced_history[i].timestamp);
            fprintf(file, "      \"cwd\": \"%s\",\n", enhanced_history[i].cwd ? enhanced_history[i].cwd : ".");
            fprintf(file, "      \"exit_code\": %d,\n", enhanced_history[i].exit_code);
            fprintf(file, "      \"execution_time_ms\": %ld,\n", enhanced_history[i].execution_time_ms);
            fprintf(file, "      \"context\": [");
            for (int j = 0; j < enhanced_history[i].context_count; j++) {
                if (enhanced_history[i].context[j]) {
                    fprintf(file, "\"%s\"", enhanced_history[i].context[j]);
                    if (j < enhanced_history[i].context_count - 1) fprintf(file, ", ");
                }
            }
            fprintf(file, "]\n");
            fprintf(file, "    }%s\n", (i < enhanced_history_count - 1) ? "," : "");
        }
    }
    
    fprintf(file, "  ],\n");
    fprintf(file, "  \"command_stats\": [\n");
    
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command) {
            fprintf(file, "    {\n");
            fprintf(file, "      \"command\": \"%s\",\n", command_stats[i].command);
            fprintf(file, "      \"frequency\": %d,\n", command_stats[i].frequency);
            fprintf(file, "      \"last_used\": %ld,\n", command_stats[i].last_used);
            fprintf(file, "      \"first_used\": %ld,\n", command_stats[i].first_used);
            fprintf(file, "      \"score\": %.2f,\n", command_stats[i].score);
            fprintf(file, "      \"success_rate\": %.2f,\n", command_stats[i].success_rate);
            fprintf(file, "      \"avg_execution_time\": %ld,\n", command_stats[i].avg_execution_time);
            fprintf(file, "      \"total_executions\": %d,\n", command_stats[i].total_executions);
            fprintf(file, "      \"successful_executions\": %d\n", command_stats[i].successful_executions);
            fprintf(file, "    }%s\n", (i < command_stats_count - 1) ? "," : "");
        }
    }
    
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");
    
    fclose(file);
    return 0;
}

// Add command to enhanced history with metadata
void add_to_enhanced_history(const char *command, const char *cwd, int exit_code, long execution_time_ms) {
    if (!command || strlen(command) == 0) return;
    
    // Expand array if needed
    if (enhanced_history_count >= enhanced_history_capacity) {
        enhanced_history_capacity *= 2;
        history_entry_t *new_history = realloc(enhanced_history, 
                                             sizeof(history_entry_t) * enhanced_history_capacity);
        if (!new_history) {
            fprintf(stderr, "xsh: Failed to expand enhanced history array\n");
            return;
        }
        enhanced_history = new_history;
    }
    
    // Add new entry
    enhanced_history[enhanced_history_count].command = strdup(command);
    enhanced_history[enhanced_history_count].timestamp = time(NULL);
    enhanced_history[enhanced_history_count].cwd = cwd ? strdup(cwd) : strdup(".");
    enhanced_history[enhanced_history_count].exit_code = exit_code;
    enhanced_history[enhanced_history_count].execution_time_ms = execution_time_ms;
    enhanced_history[enhanced_history_count].context_count = 0;
    
    // Copy recent command context
    for (int i = 0; i < recent_commands_count && i < MAX_COMMAND_CONTEXT; i++) {
        if (recent_commands[i]) {
            enhanced_history[enhanced_history_count].context[i] = strdup(recent_commands[i]);
            enhanced_history[enhanced_history_count].context_count++;
        }
    }
    
    enhanced_history_count++;
    
    // Update recent commands context
    if (recent_commands_count >= MAX_COMMAND_CONTEXT) {
        // Shift array
        if (recent_commands[0]) free(recent_commands[0]);
        for (int i = 0; i < MAX_COMMAND_CONTEXT - 1; i++) {
            recent_commands[i] = recent_commands[i + 1];
        }
        recent_commands[MAX_COMMAND_CONTEXT - 1] = strdup(command);
    } else {
        recent_commands[recent_commands_count] = strdup(command);
        recent_commands_count++;
    }
    
    // Update command patterns
    update_command_patterns(command);
    
    // Learn from execution
    learn_from_command_execution(command, (exit_code == 0), execution_time_ms);
    
    // Periodically save enhanced history
    if (enhanced_history_count % 20 == 0) {
        save_enhanced_history();
    }
}

// Cleanup the history system
void cleanup_history_system(void) {
    // Save history to files before cleanup
    save_history_to_file();
    save_enhanced_history();
    
    // Free history array
    for (int i = 0; i < history_count; i++) {
        if (history[i]) {
            free(history[i]);
            history[i] = NULL;
        }
    }
    
    // Free enhanced history
    if (enhanced_history) {
        for (int i = 0; i < enhanced_history_count; i++) {
            if (enhanced_history[i].command) free(enhanced_history[i].command);
            if (enhanced_history[i].cwd) free(enhanced_history[i].cwd);
            for (int j = 0; j < enhanced_history[i].context_count; j++) {
                if (enhanced_history[i].context[j]) free(enhanced_history[i].context[j]);
            }
        }
        free(enhanced_history);
        enhanced_history = NULL;
    }
    
    // Free command stats
    if (command_stats) {
        for (int i = 0; i < command_stats_count; i++) {
            if (command_stats[i].command) {
                free(command_stats[i].command);
            }
            if (command_stats[i].common_contexts) {
                for (int j = 0; j < command_stats[i].context_count; j++) {
                    if (command_stats[i].common_contexts[j]) {
                        free(command_stats[i].common_contexts[j]);
                    }
                }
                free(command_stats[i].common_contexts);
            }
        }
        free(command_stats);
        command_stats = NULL;
    }
    
    // Free command patterns
    if (command_patterns) {
        for (int i = 0; i < command_patterns_count; i++) {
            for (int j = 0; j < command_patterns[i].sequence_length; j++) {
                if (command_patterns[i].sequence[j]) {
                    free(command_patterns[i].sequence[j]);
                }
            }
            if (command_patterns[i].next_command) {
                free(command_patterns[i].next_command);
            }
        }
        free(command_patterns);
        command_patterns = NULL;
    }
    
    // Free recent commands context
    for (int i = 0; i < recent_commands_count; i++) {
        if (recent_commands[i]) {
            free(recent_commands[i]);
            recent_commands[i] = NULL;
        }
    }
    
    history_count = 0;
    enhanced_history_count = 0;
    command_stats_count = 0;
    command_patterns_count = 0;
    recent_commands_count = 0;
    enhanced_history_capacity = 0;
    command_stats_capacity = 0;
    command_patterns_capacity = 0;
}

// Enhanced update command statistics for smart completion
void update_command_stats(const char *command) {
    if (!command || strlen(command) == 0) return;
    
    // Extract just the command name (first word)
    char cmd_name[256];
    const char *space = strchr(command, ' ');
    if (space) {
        size_t cmd_len = space - command;
        if (cmd_len >= sizeof(cmd_name)) cmd_len = sizeof(cmd_name) - 1;
        strncpy(cmd_name, command, cmd_len);
        cmd_name[cmd_len] = '\0';
    } else {
        strncpy(cmd_name, command, sizeof(cmd_name) - 1);
        cmd_name[sizeof(cmd_name) - 1] = '\0';
    }
    
    // Find existing command or create new entry
    int found = -1;
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command && strcmp(command_stats[i].command, cmd_name) == 0) {
            found = i;
            break;
        }
    }
    
    if (found >= 0) {
        // Update existing command
        command_stats[found].frequency++;
        command_stats[found].last_used = time(NULL);
        command_stats[found].total_executions++;
    } else {
        // Add new command
        if (command_stats_count >= command_stats_capacity) {
            // Expand array
            command_stats_capacity *= 2;
            command_stat_t *new_stats = realloc(command_stats, 
                                              sizeof(command_stat_t) * command_stats_capacity);
            if (!new_stats) {
                fprintf(stderr, "xsh: Failed to expand command stats array\n");
                return;
            }
            command_stats = new_stats;
        }
        
        command_stats[command_stats_count].command = strdup(cmd_name);
        command_stats[command_stats_count].frequency = 1;
        command_stats[command_stats_count].last_used = time(NULL);
        command_stats[command_stats_count].first_used = time(NULL);
        command_stats[command_stats_count].score = 0.0;
        command_stats[command_stats_count].success_rate = 1.0;
        command_stats[command_stats_count].avg_execution_time = 0;
        command_stats[command_stats_count].common_contexts = NULL;
        command_stats[command_stats_count].context_count = 0;
        command_stats[command_stats_count].total_executions = 1;
        command_stats[command_stats_count].successful_executions = 1;
        command_stats_count++;
    }
}

// Enhanced calculate weighted scores for commands based on frequency, recency, and success rate
void calculate_command_scores(void) {
    time_t current_time = time(NULL);
    
    for (int i = 0; i < command_stats_count; i++) {
        if (!command_stats[i].command) continue;
        
        double frequency_score = log(command_stats[i].frequency + 1);
        double time_diff = difftime(current_time, command_stats[i].last_used);
        double recency_score = exp(-time_diff / (7 * 24 * 3600)); // Decay over week
        double success_factor = command_stats[i].success_rate;
        double age_factor = difftime(current_time, command_stats[i].first_used) / (30 * 24 * 3600); // Age in months
        
        // Advanced scoring algorithm
        command_stats[i].score = frequency_score * (1.0 + recency_score) * success_factor * (1.0 + age_factor * 0.1);
    }
}

// Update command patterns for context-aware completion
void update_command_patterns(const char *command) {
    if (!command || strlen(command) == 0 || recent_commands_count == 0) return;
    
    // Create pattern from recent commands leading to this command
    for (int seq_len = 1; seq_len <= recent_commands_count && seq_len <= MAX_COMMAND_CONTEXT; seq_len++) {
        // Find existing pattern or create new one
        int found = -1;
        for (int i = 0; i < command_patterns_count; i++) {
            if (command_patterns[i].sequence_length == seq_len &&
                command_patterns[i].next_command &&
                strcmp(command_patterns[i].next_command, command) == 0) {
                
                // Check if sequence matches
                int match = 1;
                for (int j = 0; j < seq_len; j++) {
                    int idx = recent_commands_count - seq_len + j;
                    if (idx < 0 || !recent_commands[idx] || !command_patterns[i].sequence[j] ||
                        strcmp(recent_commands[idx], command_patterns[i].sequence[j]) != 0) {
                        match = 0;
                        break;
                    }
                }
                if (match) {
                    found = i;
                    break;
                }
            }
        }
        
        if (found >= 0) {
            // Update existing pattern
            command_patterns[found].frequency++;
            command_patterns[found].last_used = time(NULL);
        } else {
            // Add new pattern
            if (command_patterns_count >= command_patterns_capacity) {
                command_patterns_capacity *= 2;
                command_pattern_t *new_patterns = realloc(command_patterns,
                                                        sizeof(command_pattern_t) * command_patterns_capacity);
                if (!new_patterns) {
                    fprintf(stderr, "xsh: Failed to expand command patterns array\n");
                    return;
                }
                command_patterns = new_patterns;
            }
            
            command_patterns[command_patterns_count].sequence_length = seq_len;
            for (int j = 0; j < seq_len; j++) {
                int idx = recent_commands_count - seq_len + j;
                if (idx >= 0 && recent_commands[idx]) {
                    command_patterns[command_patterns_count].sequence[j] = strdup(recent_commands[idx]);
                }
            }
            command_patterns[command_patterns_count].next_command = strdup(command);
            command_patterns[command_patterns_count].frequency = 1;
            command_patterns[command_patterns_count].last_used = time(NULL);
            command_patterns_count++;
        }
    }
}

// Learn from command execution results
void learn_from_command_execution(const char *command, int success, long execution_time) {
    if (!command || strlen(command) == 0) return;
    
    // Extract command name
    char cmd_name[256];
    const char *space = strchr(command, ' ');
    if (space) {
        size_t cmd_len = space - command;
        if (cmd_len >= sizeof(cmd_name)) cmd_len = sizeof(cmd_name) - 1;
        strncpy(cmd_name, command, cmd_len);
        cmd_name[cmd_len] = '\0';
    } else {
        strncpy(cmd_name, command, sizeof(cmd_name) - 1);
        cmd_name[sizeof(cmd_name) - 1] = '\0';
    }
    
    // Find command stats
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command && strcmp(command_stats[i].command, cmd_name) == 0) {
            if (success) {
                command_stats[i].successful_executions++;
            }
            command_stats[i].total_executions++;
            command_stats[i].success_rate = (double)command_stats[i].successful_executions / 
                                          command_stats[i].total_executions;
            
            // Update average execution time
            if (execution_time > 0) {
                if (command_stats[i].avg_execution_time == 0) {
                    command_stats[i].avg_execution_time = execution_time;
                } else {
                    command_stats[i].avg_execution_time = 
                        (command_stats[i].avg_execution_time + execution_time) / 2;
                }
            }
            break;
        }
    }
}

// Smart completion engine with multiple strategies
char **get_adaptive_completions(const char *input, int *completion_count) {
    if (!input || !completion_count) return NULL;
    
    *completion_count = 0;
    char **completions = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!completions) return NULL;
    
    // Strategy 1: Exact prefix matching with frequency ranking
    calculate_command_scores();
    for (int i = 0; i < command_stats_count && *completion_count < MAX_COMPLETION_SUGGESTIONS / 2; i++) {
        if (command_stats[i].command && strncmp(command_stats[i].command, input, strlen(input)) == 0) {
            // Check for duplicates
            int duplicate = 0;
            for (int j = 0; j < *completion_count; j++) {
                if (strcmp(completions[j], command_stats[i].command) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                completions[*completion_count] = strdup(command_stats[i].command);
                (*completion_count)++;
            }
        }
    }
    
    // Strategy 2: Pattern-based predictions
    if (*completion_count < MAX_COMPLETION_SUGGESTIONS / 2) {
        for (int i = 0; i < command_patterns_count && *completion_count < MAX_COMPLETION_SUGGESTIONS; i++) {
            if (command_patterns[i].next_command && 
                strncmp(command_patterns[i].next_command, input, strlen(input)) == 0) {
                
                // Check if current context matches pattern
                int context_match = 1;
                if (recent_commands_count >= command_patterns[i].sequence_length) {
                    for (int j = 0; j < command_patterns[i].sequence_length; j++) {
                        int idx = recent_commands_count - command_patterns[i].sequence_length + j;
                        if (idx < 0 || !recent_commands[idx] || !command_patterns[i].sequence[j] ||
                            strcmp(recent_commands[idx], command_patterns[i].sequence[j]) != 0) {
                            context_match = 0;
                            break;
                        }
                    }
                }
                
                if (context_match) {
                    // Check for duplicates
                    int duplicate = 0;
                    for (int j = 0; j < *completion_count; j++) {
                        if (strcmp(completions[j], command_patterns[i].next_command) == 0) {
                            duplicate = 1;
                            break;
                        }
                    }
                    if (!duplicate) {
                        completions[*completion_count] = strdup(command_patterns[i].next_command);
                        (*completion_count)++;
                    }
                }
            }
        }
    }
    
    // Strategy 3: Fill remaining slots with history-based suggestions
    if (*completion_count < MAX_COMPLETION_SUGGESTIONS) {
        for (int i = enhanced_history_count - 1; i >= 0 && *completion_count < MAX_COMPLETION_SUGGESTIONS; i--) {
            if (enhanced_history[i].command && 
                strncmp(enhanced_history[i].command, input, strlen(input)) == 0) {
                
                // Check for duplicates
                int duplicate = 0;
                for (int j = 0; j < *completion_count; j++) {
                    if (strcmp(completions[j], enhanced_history[i].command) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    completions[*completion_count] = strdup(enhanced_history[i].command);
                    (*completion_count)++;
                }
            }
        }
    }
    
    return completions;
}

// Context-aware completion based on recent commands
char **get_context_aware_completions(const char *input, int *completion_count) {
    if (!input || !completion_count) return NULL;
    
    *completion_count = 0;
    char **completions = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!completions) return NULL;
    
    // Look for patterns that match current context
    for (int i = 0; i < command_patterns_count && *completion_count < MAX_COMPLETION_SUGGESTIONS; i++) {
        if (!command_patterns[i].next_command) continue;
        
        // Check if pattern sequence matches recent commands
        int context_match = 1;
        if (recent_commands_count >= command_patterns[i].sequence_length) {
            for (int j = 0; j < command_patterns[i].sequence_length; j++) {
                int idx = recent_commands_count - command_patterns[i].sequence_length + j;
                if (idx < 0 || !recent_commands[idx] || !command_patterns[i].sequence[j] ||
                    strcmp(recent_commands[idx], command_patterns[i].sequence[j]) != 0) {
                    context_match = 0;
                    break;
                }
            }
        } else {
            context_match = 0;
        }
        
        if (context_match && strncmp(command_patterns[i].next_command, input, strlen(input)) == 0) {
            completions[*completion_count] = strdup(command_patterns[i].next_command);
            (*completion_count)++;
        }
    }
    
    return completions;
}

// Directory-based completion suggestions
char **get_directory_based_suggestions(const char *input, int *completion_count) {
    if (!input || !completion_count) return NULL;
    
    *completion_count = 0;
    char **completions = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!completions) return NULL;
    
    // Get current working directory
    char current_dir[1024];
#ifdef _WIN32
    if (_getcwd(current_dir, sizeof(current_dir)) == NULL) {
        free(completions);
        return NULL;
    }
#else
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        free(completions);
        return NULL;
    }
#endif
    
    // Look for commands that were executed in this directory
    for (int i = enhanced_history_count - 1; i >= 0 && *completion_count < MAX_COMPLETION_SUGGESTIONS; i--) {
        if (enhanced_history[i].command && enhanced_history[i].cwd &&
            strcmp(enhanced_history[i].cwd, current_dir) == 0 &&
            strncmp(enhanced_history[i].command, input, strlen(input)) == 0) {
            
            // Check for duplicates
            int duplicate = 0;
            for (int j = 0; j < *completion_count; j++) {
                if (strcmp(completions[j], enhanced_history[i].command) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                completions[*completion_count] = strdup(enhanced_history[i].command);
                (*completion_count)++;
            }
        }
    }
    
    return completions;
}

// Analytics and reporting functions
void display_performance_analytics(void) {
    printf("\n=== XShell Performance Analytics ===\n");
    printf("Total Commands in History: %d\n", enhanced_history_count);
    printf("Unique Commands Tracked: %d\n", command_stats_count);
    printf("Command Patterns Learned: %d\n", command_patterns_count);
    
    // Most frequent commands
    printf("\n--- Top 10 Most Frequent Commands ---\n");
    calculate_command_scores();
    
    // Sort commands by score (frequency * recency * success_rate)
    command_stat_t *sorted_commands = malloc(sizeof(command_stat_t) * command_stats_count);
    if (sorted_commands) {
        memcpy(sorted_commands, command_stats, sizeof(command_stat_t) * command_stats_count);
        
        // Simple bubble sort by score
        for (int i = 0; i < command_stats_count - 1; i++) {
            for (int j = 0; j < command_stats_count - i - 1; j++) {
                if (sorted_commands[j].score < sorted_commands[j + 1].score) {
                    command_stat_t temp = sorted_commands[j];
                    sorted_commands[j] = sorted_commands[j + 1];
                    sorted_commands[j + 1] = temp;
                }
            }
        }
        
        int display_count = (command_stats_count < 10) ? command_stats_count : 10;
        for (int i = 0; i < display_count; i++) {
            if (sorted_commands[i].command) {
                printf("%2d. %-15s (freq: %3d, score: %.2f, success: %.1f%%)\n",
                       i + 1, sorted_commands[i].command, sorted_commands[i].frequency,
                       sorted_commands[i].score, sorted_commands[i].success_rate * 100);
            }
        }
        free(sorted_commands);
    }
    
    // Command patterns
    printf("\n--- Top Command Patterns ---\n");
    for (int i = 0; i < command_patterns_count && i < 5; i++) {
        if (command_patterns[i].next_command) {
            printf("Pattern %d (freq: %d): ", i + 1, command_patterns[i].frequency);
            for (int j = 0; j < command_patterns[i].sequence_length; j++) {
                if (command_patterns[i].sequence[j]) {
                    printf("%s -> ", command_patterns[i].sequence[j]);
                }
            }
            printf("%s\n", command_patterns[i].next_command);
        }
    }
    
    // Recent activity
    printf("\n--- Recent Activity (Last 10 Commands) ---\n");
    int start = (enhanced_history_count > 10) ? enhanced_history_count - 10 : 0;
    for (int i = start; i < enhanced_history_count; i++) {
        if (enhanced_history[i].command) {
            struct tm *tm_info = localtime(&enhanced_history[i].timestamp);
            printf("%2d. [%02d:%02d:%02d] %s\n", 
                   i - start + 1, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
                   enhanced_history[i].command);
        }
    }
    printf("\n");
}

void display_command_stats(const char *command) {
    if (!command) {
        display_performance_analytics();
        return;
    }
    
    // Find specific command stats
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command && strcmp(command_stats[i].command, command) == 0) {
            printf("\n=== Statistics for '%s' ===\n", command);
            printf("Frequency: %d\n", command_stats[i].frequency);
            printf("Success Rate: %.1f%%\n", command_stats[i].success_rate * 100);
            printf("Average Execution Time: %ldms\n", command_stats[i].avg_execution_time);
            
            struct tm *first_tm = localtime(&command_stats[i].first_used);
            struct tm *last_tm = localtime(&command_stats[i].last_used);
            printf("First Used: %04d-%02d-%02d %02d:%02d:%02d\n",
                   first_tm->tm_year + 1900, first_tm->tm_mon + 1, first_tm->tm_mday,
                   first_tm->tm_hour, first_tm->tm_min, first_tm->tm_sec);
            printf("Last Used: %04d-%02d-%02d %02d:%02d:%02d\n",
                   last_tm->tm_year + 1900, last_tm->tm_mon + 1, last_tm->tm_mday,
                   last_tm->tm_hour, last_tm->tm_min, last_tm->tm_sec);
            printf("Score: %.2f\n", command_stats[i].score);
            printf("\n");
            return;
        }
    }
    
    printf("No statistics found for command: %s\n", command);
}

// Clear analytics data (useful for testing or privacy)
void clear_analytics_data(void) {
    // Clear command stats
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command) {
            free(command_stats[i].command);
        }
        if (command_stats[i].common_contexts) {
            for (int j = 0; j < command_stats[i].context_count; j++) {
                free(command_stats[i].common_contexts[j]);
            }
            free(command_stats[i].common_contexts);
        }
    }
    free(command_stats);
    command_stats = NULL;
    command_stats_count = 0;
    command_stats_capacity = 0;
    
    // Clear command patterns
    for (int i = 0; i < command_patterns_count; i++) {
        for (int j = 0; j < command_patterns[i].sequence_length; j++) {
            if (command_patterns[i].sequence[j]) {
                free(command_patterns[i].sequence[j]);
            }
        }
        if (command_patterns[i].next_command) {
            free(command_patterns[i].next_command);
        }
    }
    free(command_patterns);
    command_patterns = NULL;
    command_patterns_count = 0;
    command_patterns_capacity = 0;
    
    // Clear recent commands
    for (int i = 0; i < recent_commands_count; i++) {
        if (recent_commands[i]) {
            free(recent_commands[i]);
        }
    }
    recent_commands_count = 0;
    
    printf("Analytics data cleared.\n");
}

// Smart completion function - integrates all completion strategies
char** get_smart_completions(const char *partial, int *match_count) {
    if (!partial || !match_count) return NULL;
    
    *match_count = 0;
    char **completions = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!completions) return NULL;
    
    // Strategy 1: Use adaptive completions (primary strategy)
    char **adaptive = get_adaptive_completions(partial, match_count);
    if (adaptive && *match_count > 0) {
        for (int i = 0; i < *match_count && i < MAX_COMPLETION_SUGGESTIONS; i++) {
            completions[i] = strdup(adaptive[i]);
        }
        free(adaptive);
    }
    
    // Strategy 2: Add context-aware completions if we have space
    if (*match_count < MAX_COMPLETION_SUGGESTIONS) {
        int context_count = 0;
        char **context_completions = get_context_aware_completions(partial, &context_count);
        if (context_completions) {
            for (int i = 0; i < context_count && *match_count < MAX_COMPLETION_SUGGESTIONS; i++) {
                // Check for duplicates
                int duplicate = 0;
                for (int j = 0; j < *match_count; j++) {
                    if (strcmp(completions[j], context_completions[i]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    completions[*match_count] = strdup(context_completions[i]);
                    (*match_count)++;
                }
            }
            free(context_completions);
        }
    }
    
    // Strategy 3: Add directory-based suggestions
    if (*match_count < MAX_COMPLETION_SUGGESTIONS) {
        int dir_count = 0;
        char **dir_completions = get_directory_based_suggestions(partial, &dir_count);
        if (dir_completions) {
            for (int i = 0; i < dir_count && *match_count < MAX_COMPLETION_SUGGESTIONS; i++) {
                // Check for duplicates
                int duplicate = 0;
                for (int j = 0; j < *match_count; j++) {
                    if (strcmp(completions[j], dir_completions[i]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    completions[*match_count] = strdup(dir_completions[i]);
                    (*match_count)++;
                }
            }
            free(dir_completions);
        }
    }
    
    return completions;
}

// Get frequent commands function
char** get_frequent_commands(int *count) {
    if (!count) return NULL;
    
    *count = 0;
    char **frequent = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!frequent) return NULL;
    
    // Sort commands by frequency
    calculate_command_scores();
    command_stat_t *sorted = malloc(sizeof(command_stat_t) * command_stats_count);
    if (!sorted) {
        free(frequent);
        return NULL;
    }
    
    memcpy(sorted, command_stats, sizeof(command_stat_t) * command_stats_count);
    
    // Simple bubble sort by frequency
    for (int i = 0; i < command_stats_count - 1; i++) {
        for (int j = 0; j < command_stats_count - i - 1; j++) {
            if (sorted[j].frequency < sorted[j + 1].frequency) {
                command_stat_t temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    // Return top commands
    int max_return = (command_stats_count < MAX_COMPLETION_SUGGESTIONS) ? 
                     command_stats_count : MAX_COMPLETION_SUGGESTIONS;
    for (int i = 0; i < max_return; i++) {
        if (sorted[i].command) {
            frequent[*count] = strdup(sorted[i].command);
            (*count)++;
        }
    }
    
    free(sorted);
    return frequent;
}

// Get recent commands function
char** get_recent_commands(int *count) {
    if (!count) return NULL;
    
    *count = 0;
    char **recent = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!recent) return NULL;
    
    // Return recent commands from enhanced history
    int start = (enhanced_history_count > MAX_COMPLETION_SUGGESTIONS) ? 
                enhanced_history_count - MAX_COMPLETION_SUGGESTIONS : 0;
    
    for (int i = enhanced_history_count - 1; i >= start; i--) {
        if (enhanced_history[i].command) {
            recent[*count] = strdup(enhanced_history[i].command);
            (*count)++;
        }
    }
    
    return recent;
}

// Get successful commands function
char** get_successful_commands(const char *partial, int *match_count) {
    if (!partial || !match_count) return NULL;
    
    *match_count = 0;
    char **successful = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!successful) return NULL;
    
    // Find commands with high success rate that match partial
    for (int i = 0; i < command_stats_count && *match_count < MAX_COMPLETION_SUGGESTIONS; i++) {
        if (command_stats[i].command && 
            command_stats[i].success_rate > 0.8 && // 80% success rate threshold
            strncmp(command_stats[i].command, partial, strlen(partial)) == 0) {
            successful[*match_count] = strdup(command_stats[i].command);
            (*match_count)++;
        }
    }
    
    return successful;
}

// Calculate command relevance score
double calculate_command_relevance_score(const char *command, const char *partial) {
    if (!command || !partial) return 0.0;
    
    double score = 0.0;
    
    // Base score for prefix match
    if (strncmp(command, partial, strlen(partial)) == 0) {
        score += 10.0;
    }
    
    // Find command in stats for additional scoring
    for (int i = 0; i < command_stats_count; i++) {
        if (command_stats[i].command && strcmp(command_stats[i].command, command) == 0) {
            // Frequency component (0-5 points)
            score += (command_stats[i].frequency / 100.0) * 5.0;
            
            // Success rate component (0-3 points)
            score += command_stats[i].success_rate * 3.0;
            
            // Recency component (0-2 points)
            time_t now = time(NULL);
            double days_ago = (now - command_stats[i].last_used) / (24.0 * 3600.0);
            score += (days_ago < 1.0) ? 2.0 : (days_ago < 7.0) ? 1.0 : 0.0;
            
            break;
        }
    }
    
    return score;
}

// Analyze command patterns function
void analyze_command_patterns(void) {
    printf("\n=== Command Pattern Analysis ===\n");
    printf("Total patterns identified: %d\n", command_patterns_count);
    
    if (command_patterns_count == 0) {
        printf("No patterns identified yet. Execute more commands to build patterns.\n");
        return;
    }
    
    // Show top patterns
    printf("\n--- Most Frequent Patterns ---\n");
    for (int i = 0; i < command_patterns_count && i < 10; i++) {
        if (command_patterns[i].next_command && command_patterns[i].frequency > 1) {
            printf("Pattern %d (frequency: %d):\n", i + 1, command_patterns[i].frequency);
            printf("  Sequence: ");
            for (int j = 0; j < command_patterns[i].sequence_length; j++) {
                if (command_patterns[i].sequence[j]) {
                    printf("%s", command_patterns[i].sequence[j]);
                    if (j < command_patterns[i].sequence_length - 1) printf(" -> ");
                }
            }
            printf(" -> %s\n", command_patterns[i].next_command);
            printf("  Confidence: %.1f%%\n", command_patterns[i].confidence * 100);
        }
    }
    
    // Pattern statistics
    int active_patterns = 0;
    double avg_confidence = 0.0;
    for (int i = 0; i < command_patterns_count; i++) {
        if (command_patterns[i].frequency > 1) {
            active_patterns++;
            avg_confidence += command_patterns[i].confidence;
        }
    }
    
    if (active_patterns > 0) {
        avg_confidence /= active_patterns;
        printf("\n--- Pattern Statistics ---\n");
        printf("Active patterns (freq > 1): %d\n", active_patterns);
        printf("Average confidence: %.1f%%\n", avg_confidence * 100);
    }
}

// Display usage trends function
void display_usage_trends(void) {
    printf("\n=== Usage Trends Analysis ===\n");
    
    if (enhanced_history_count == 0) {
        printf("No command history available for trend analysis.\n");
        return;
    }
    
    // Analyze commands by hour
    int hourly_counts[24] = {0};
    for (int i = 0; i < enhanced_history_count; i++) {
        struct tm *tm_info = localtime(&enhanced_history[i].timestamp);
        if (tm_info->tm_hour >= 0 && tm_info->tm_hour < 24) {
            hourly_counts[tm_info->tm_hour]++;
        }
    }
    
    printf("\n--- Activity by Hour of Day ---\n");
    int max_count = 0;
    for (int i = 0; i < 24; i++) {
        if (hourly_counts[i] > max_count) max_count = hourly_counts[i];
    }
    
    for (int i = 0; i < 24; i++) {
        printf("%02d:00 ", i);
        int bar_length = max_count > 0 ? (hourly_counts[i] * 50) / max_count : 0;
        for (int j = 0; j < bar_length; j++) printf("█");
        printf(" %d\n", hourly_counts[i]);
    }
    
    // Find peak hours
    int peak_hour = 0;
    for (int i = 1; i < 24; i++) {
        if (hourly_counts[i] > hourly_counts[peak_hour]) {
            peak_hour = i;
        }
    }
    
    printf("\nPeak usage hour: %02d:00 (%d commands)\n", peak_hour, hourly_counts[peak_hour]);
    
    // Recent activity trend
    printf("\n--- Recent Activity Trend ---\n");
    time_t now = time(NULL);
    int recent_days[7] = {0}; // Last 7 days
    
    for (int i = 0; i < enhanced_history_count; i++) {
        double days_ago = (now - enhanced_history[i].timestamp) / (24.0 * 3600.0);
        if (days_ago < 7.0) {
            int day_index = (int)days_ago;
            if (day_index >= 0 && day_index < 7) {
                recent_days[day_index]++;
            }
        }
    }
    
    const char* day_labels[] = {"Today", "Yesterday", "2 days ago", "3 days ago", 
                               "4 days ago", "5 days ago", "6 days ago"};
    
    for (int i = 0; i < 7; i++) {
        printf("%-12s: %d commands\n", day_labels[i], recent_days[i]);
    }
}

// Time-based suggestions (updated signature to match header)
char** get_time_based_suggestions(int hour_of_day, int *match_count) {
    if (!match_count || hour_of_day < 0 || hour_of_day > 23) return NULL;
    
    *match_count = 0;
    char **suggestions = malloc(sizeof(char*) * MAX_COMPLETION_SUGGESTIONS);
    if (!suggestions) return NULL;
    
    // Find commands commonly used at this hour
    for (int i = 0; i < enhanced_history_count && *match_count < MAX_COMPLETION_SUGGESTIONS; i++) {
        struct tm *tm_info = localtime(&enhanced_history[i].timestamp);
        if (tm_info->tm_hour == hour_of_day) {
            // Check for duplicates
            int duplicate = 0;
            for (int j = 0; j < *match_count; j++) {
                if (strcmp(suggestions[j], enhanced_history[i].command) == 0) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate && enhanced_history[i].command) {
                suggestions[*match_count] = strdup(enhanced_history[i].command);
                (*match_count)++;
            }
        }
    }
    
    return suggestions;
}
