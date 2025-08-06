#ifndef EXECUTE_H
#define EXECUTE_H

#include "xsh.h" // For common definitions

// Command operator types
typedef enum {
    CMD_SIMPLE,      // Simple command
    CMD_PIPE,        // Command with pipe |
    CMD_AND,         // Command with &&
    CMD_OR,          // Command with ||
    CMD_SEMICOLON    // Command with ;
} cmd_operator_t;

// Redirection types
typedef enum {
    REDIR_NONE,      // No redirection
    REDIR_IN,        // Input redirection <
    REDIR_OUT,       // Output redirection >
    REDIR_APPEND,    // Append redirection >>
    REDIR_ERR        // Error redirection 2>
} redir_type_t;

// Redirection structure
typedef struct {
    redir_type_t type;
    char *filename;
} redirection_t;

// Command structure
typedef struct command {
    char **args;                    // Command arguments
    redirection_t input_redir;      // Input redirection
    redirection_t output_redir;     // Output redirection
    redirection_t error_redir;      // Error redirection
    cmd_operator_t operator;        // Operator following this command
    struct command *next;           // Next command in chain
} command_t;

// Command pipeline structure
typedef struct {
    command_t *commands;            // Linked list of commands
    int command_count;              // Number of commands
} pipeline_t;

// Function prototypes
int xsh_launch(char **args);
pipeline_t *parse_command_line(char **tokens, int token_count);
int execute_pipeline(pipeline_t *pipeline);
void free_pipeline(pipeline_t *pipeline);
command_t *create_command(void);
void free_command(command_t *cmd);
int setup_redirections(command_t *cmd);
void restore_redirections(void);
int execute_simple_command(command_t *cmd);
int execute_piped_commands(command_t *commands, int num_commands);
int contains_operators(const char *str);
char **tokenize_with_operators(char *line, int *token_count);
int parse_redirection(command_t *cmd, char *token, char *next_token);

// Platform-specific functions
#ifdef _WIN32
int execute_piped_commands_windows(command_t *commands);
int execute_single_command_windows(command_t *cmd);
int setup_redirections_windows(command_t *cmd, STARTUPINFO *si);
char *build_command_line_windows(char **args);
char *find_executable_windows(const char *command);
#endif

// Global variable to track command exit status
extern int last_command_exit_status;

#endif // EXECUTE_H
