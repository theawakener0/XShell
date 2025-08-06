#ifndef HISTORY_H
#define HISTORY_H

#include "xsh.h" // For XSH_HISTORY_SIZE
#include <time.h>

#define HISTORY_FILE_NAME ".xshell_history"
#define HISTORY_METADATA_FILE ".xshell_history_meta.json"
#define MAX_HISTORY_FILE_SIZE 10000
#define COMMAND_FREQUENCY_THRESHOLD 3
#define MAX_COMMAND_CONTEXT 5
#define MAX_COMPLETION_SUGGESTIONS 20

// Enhanced command history entry with metadata
typedef struct {
    char *command;
    time_t timestamp;
    char *cwd;  // Current working directory when command was executed
    int exit_code;  // Exit code of the command
    long execution_time_ms;  // How long the command took to execute
    char *context[MAX_COMMAND_CONTEXT];  // Previous commands (for context-aware completion)
    int context_count;
} history_entry_t;

// Enhanced command frequency tracking structure
typedef struct {
    char *command;
    int frequency;
    time_t last_used;
    time_t first_used;
    double score; // Weighted score based on frequency and recency
    double success_rate; // Percentage of successful executions
    long avg_execution_time; // Average execution time
    char **common_contexts; // Common previous commands
    int context_count;
    int total_executions;
    int successful_executions;
} command_stat_t;

// Command sequence pattern for context-aware completion
typedef struct {
    char *sequence[MAX_COMMAND_CONTEXT];
    int sequence_length;
    char *next_command;
    int frequency;
    time_t last_used;
    double confidence; // Confidence score for the pattern
} command_pattern_t;

// External declarations for history variables and functions
// Definitions will be in history.c
extern char *history[XSH_HISTORY_SIZE];
extern int history_count;
extern history_entry_t *enhanced_history;
extern int enhanced_history_count;
extern int enhanced_history_capacity;
extern command_stat_t *command_stats;
extern int command_stats_count;
extern int command_stats_capacity;
extern command_pattern_t *command_patterns;
extern int command_patterns_count;
extern int command_patterns_capacity;

// Function prototypes for history.c
void add_to_history(const char *line);
void add_to_enhanced_history(const char *command, const char *cwd, int exit_code, long execution_time_ms);
void display_history(void);
int init_history_system(void);
void cleanup_history_system(void);
int load_history_from_file(void);
int save_history_to_file(void);
int load_enhanced_history(void);
int save_enhanced_history(void);
char* get_history_file_path(void);
char* get_history_metadata_file_path(void);

// Smart completion functions
void update_command_stats(const char *command);
void update_command_patterns(const char *command);
void calculate_command_scores(void);
char** get_smart_completions(const char *partial, int *match_count);
char** get_context_aware_completions(const char *partial, int *match_count);
char** get_adaptive_completions(const char *partial, int *match_count);

// History analytics and learning
void display_command_stats(const char *command);
char** get_frequent_commands(int *count);
char** get_recent_commands(int *count);
char** get_successful_commands(const char *partial, int *match_count);
char** get_directory_based_suggestions(const char *partial, int *match_count);
double calculate_command_relevance_score(const char *command, const char *partial);
void learn_from_command_execution(const char *command, int success, long execution_time);
void analyze_command_patterns(void);

// Performance and usage analytics
void display_performance_analytics(void);
void display_usage_trends(void);
char** get_time_based_suggestions(int hour_of_day, int *match_count);

#endif // HISTORY_H
