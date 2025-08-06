#include "execute.h"
#include "builtins.h"
#include "xsh.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#define close(fd) _close(fd)
#define dup2(old, new) _dup2(old, new)
#define fork() -1  // Not supported on Windows
#define waitpid(pid, status, options) -1
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2L
#endif
#else
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#endif

// Global variables to save original file descriptors
static int original_stdin = -1;
static int original_stdout = -1;
static int original_stderr = -1;

// Global variable to track last command exit status
int last_command_exit_status = 0;

// Create a new command structure
command_t *create_command(void) {
    command_t *cmd = malloc(sizeof(command_t));
    if (!cmd) {
        fprintf(stderr, "xsh: allocation error for command\n");
        return NULL;
    }
    
    cmd->args = NULL;
    cmd->input_redir.type = REDIR_NONE;
    cmd->input_redir.filename = NULL;
    cmd->output_redir.type = REDIR_NONE;
    cmd->output_redir.filename = NULL;
    cmd->error_redir.type = REDIR_NONE;
    cmd->error_redir.filename = NULL;
    cmd->operator = CMD_SIMPLE;
    cmd->next = NULL;
    
    return cmd;
}

// Free a command structure
void free_command(command_t *cmd) {
    if (!cmd) return;
    
    if (cmd->args) {
        for (int i = 0; cmd->args[i] != NULL; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }
    
    if (cmd->input_redir.filename) {
        free(cmd->input_redir.filename);
    }
    if (cmd->output_redir.filename) {
        free(cmd->output_redir.filename);
    }
    if (cmd->error_redir.filename) {
        free(cmd->error_redir.filename);
    }
    
    free(cmd);
}

// Free a pipeline structure
void free_pipeline(pipeline_t *pipeline) {
    if (!pipeline) return;
    
    command_t *cmd = pipeline->commands;
    while (cmd) {
        command_t *next = cmd->next;
        free_command(cmd);
        cmd = next;
    }
    
    free(pipeline);
}

// Check if a string contains shell operators
int contains_operators(const char *str) {
    return (strstr(str, "|") || strstr(str, "&&") || strstr(str, "||") || 
            strstr(str, ">") || strstr(str, "<") || strstr(str, ">>") || 
            strstr(str, "2>") || strstr(str, ";"));
}

// Parse redirection operators and update command
int parse_redirection(command_t *cmd, char *token, char *next_token) {
    if (strcmp(token, "<") == 0) {
        if (!next_token) {
            fprintf(stderr, "xsh: syntax error: expected filename after '<'\n");
            return -1;
        }
        cmd->input_redir.type = REDIR_IN;
        cmd->input_redir.filename = strdup(next_token);
        return 1; // Consumed next token
    } else if (strcmp(token, ">") == 0) {
        if (!next_token) {
            fprintf(stderr, "xsh: syntax error: expected filename after '>'\n");
            return -1;
        }
        cmd->output_redir.type = REDIR_OUT;
        cmd->output_redir.filename = strdup(next_token);
        return 1;
    } else if (strcmp(token, ">>") == 0) {
        if (!next_token) {
            fprintf(stderr, "xsh: syntax error: expected filename after '>>'\n");
            return -1;
        }
        cmd->output_redir.type = REDIR_APPEND;
        cmd->output_redir.filename = strdup(next_token);
        return 1;
    } else if (strcmp(token, "2>") == 0) {
        if (!next_token) {
            fprintf(stderr, "xsh: syntax error: expected filename after '2>'\n");
            return -1;
        }
        cmd->error_redir.type = REDIR_ERR;
        cmd->error_redir.filename = strdup(next_token);
        return 1;
    }
    return 0; // Not a redirection operator
}

// Enhanced tokenization function that handles operators
char **tokenize_with_operators(char *line, int *token_count) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    
    if (!tokens) {
        fprintf(stderr, "xsh: allocation error\n");
        return NULL;
    }
    
    char *line_copy = strdup(line);
    char *ptr = line_copy;
    
    while (*ptr) {
        // Skip whitespace
        while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
            ptr++;
        }
        
        if (!*ptr) break;
        
        char *start = ptr;
        
        // Handle quoted strings
        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr++;
            start = ptr;
            while (*ptr && *ptr != quote) ptr++;
            if (*ptr == quote) {
                *ptr = '\0';
                tokens[position] = strdup(start);
                ptr++;
            } else {
                fprintf(stderr, "xsh: unterminated quote\n");
                free(tokens);
                free(line_copy);
                return NULL;
            }
        }
        // Handle multi-character operators
        else if (strncmp(ptr, "&&", 2) == 0) {
            tokens[position] = strdup("&&");
            ptr += 2;
        }
        else if (strncmp(ptr, "||", 2) == 0) {
            tokens[position] = strdup("||");
            ptr += 2;
        }
        else if (strncmp(ptr, ">>", 2) == 0) {
            tokens[position] = strdup(">>");
            ptr += 2;
        }
        else if (strncmp(ptr, "2>", 2) == 0) {
            tokens[position] = strdup("2>");
            ptr += 2;
        }
        // Handle single-character operators
        else if (*ptr == '|' || *ptr == '<' || *ptr == '>' || *ptr == ';') {
            char op[2] = {*ptr, '\0'};
            tokens[position] = strdup(op);
            ptr++;
        }
        // Handle regular words
        else {
            start = ptr;
            while (*ptr && !isspace(*ptr) && *ptr != '|' && *ptr != '<' && 
                   *ptr != '>' && *ptr != ';' && strncmp(ptr, "&&", 2) != 0 && 
                   strncmp(ptr, "||", 2) != 0 && strncmp(ptr, ">>", 2) != 0 && 
                   strncmp(ptr, "2>", 2) != 0) {
                ptr++;
            }
            
            if (ptr > start) {
                int len = ptr - start;
                char *word = malloc(len + 1);
                strncpy(word, start, len);
                word[len] = '\0';
                tokens[position] = word;
            } else {
                continue;
            }
        }
        
        position++;
        
        // Resize if needed
        if (position >= bufsize) {
            bufsize *= 2;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "xsh: allocation error\n");
                free(line_copy);
                return NULL;
            }
        }
    }
    
    tokens[position] = NULL;
    *token_count = position;
    free(line_copy);
    return tokens;
}

// Parse command line into pipeline structure
pipeline_t *parse_command_line(char **tokens, int token_count) {
    if (!tokens || token_count == 0) return NULL;
    
    pipeline_t *pipeline = malloc(sizeof(pipeline_t));
    if (!pipeline) {
        fprintf(stderr, "xsh: allocation error for pipeline\n");
        return NULL;
    }
    
    pipeline->commands = NULL;
    command_t *current_cmd = NULL;
    command_t *last_cmd = NULL;
    
    int i = 0;
    while (i < token_count) {
        // Create new command
        current_cmd = create_command();
        if (!current_cmd) {
            free_pipeline(pipeline);
            return NULL;
        }
        
        // Link to pipeline
        if (!pipeline->commands) {
            pipeline->commands = current_cmd;
        } else {
            last_cmd->next = current_cmd;
        }
        
        // Parse arguments for current command
        int arg_count = 0;
        int arg_capacity = 16;
        current_cmd->args = malloc(arg_capacity * sizeof(char*));
        
        if (!current_cmd->args) {
            fprintf(stderr, "xsh: allocation error for args\n");
            free_pipeline(pipeline);
            return NULL;
        }
        
        // Parse tokens for this command until we hit an operator
        while (i < token_count) {
            char *token = tokens[i];
            char *next_token = (i + 1 < token_count) ? tokens[i + 1] : NULL;
            
            // Check for operators
            if (strcmp(token, "|") == 0) {
                current_cmd->operator = CMD_PIPE;
                i++;
                break;
            } else if (strcmp(token, "&&") == 0) {
                current_cmd->operator = CMD_AND;
                i++;
                break;
            } else if (strcmp(token, "||") == 0) {
                current_cmd->operator = CMD_OR;
                i++;
                break;
            } else if (strcmp(token, ";") == 0) {
                current_cmd->operator = CMD_SEMICOLON;
                i++;
                break;
            }
            // Check for redirections
            else {
                int redir_result = parse_redirection(current_cmd, token, next_token);
                if (redir_result == -1) {
                    free_pipeline(pipeline);
                    return NULL;
                } else if (redir_result == 1) {
                    // Consumed redirection operator and filename
                    i += 2;
                    continue;
                }
                
                // Regular argument
                if (arg_count >= arg_capacity - 1) {
                    arg_capacity *= 2;
                    current_cmd->args = realloc(current_cmd->args, arg_capacity * sizeof(char*));
                    if (!current_cmd->args) {
                        fprintf(stderr, "xsh: allocation error for args realloc\n");
                        free_pipeline(pipeline);
                        return NULL;
                    }
                }
                
                current_cmd->args[arg_count] = strdup(token);
                arg_count++;
                i++;
            }
        }
        
        // Null-terminate args array
        current_cmd->args[arg_count] = NULL;
        
        // If no operator was found, this is the last command
        if (i >= token_count) {
            current_cmd->operator = CMD_SIMPLE;
        }
        
        last_cmd = current_cmd;
        
        // Break if we've processed all tokens
        if (i >= token_count) break;
    }
    
    return pipeline;
}

// Setup file redirections for a command
int setup_redirections(command_t *cmd) {
    // Save original file descriptors
    if (original_stdin == -1) original_stdin = dup(STDIN_FILENO);
    if (original_stdout == -1) original_stdout = dup(STDOUT_FILENO);
    if (original_stderr == -1) original_stderr = dup(STDERR_FILENO);
    
    // Handle input redirection
    if (cmd->input_redir.type == REDIR_IN) {
        int fd = open(cmd->input_redir.filename, O_RDONLY);
        if (fd == -1) {
            perror("xsh: input redirection");
            return -1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // Handle output redirection
    if (cmd->output_redir.type == REDIR_OUT) {
        int fd = open(cmd->output_redir.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("xsh: output redirection");
            return -1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    } else if (cmd->output_redir.type == REDIR_APPEND) {
        int fd = open(cmd->output_redir.filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            perror("xsh: append redirection");
            return -1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    // Handle error redirection
    if (cmd->error_redir.type == REDIR_ERR) {
        int fd = open(cmd->error_redir.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("xsh: error redirection");
            return -1;
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    
    return 0;
}

// Restore original file descriptors
void restore_redirections(void) {
    if (original_stdin != -1) {
        dup2(original_stdin, STDIN_FILENO);
    }
    if (original_stdout != -1) {
        dup2(original_stdout, STDOUT_FILENO);
    }
    if (original_stderr != -1) {
        dup2(original_stderr, STDERR_FILENO);
    }
}

#ifdef _WIN32
// Windows-specific helper functions

// Build command line string for Windows CreateProcess
char *build_command_line_windows(char **args) {
    if (!args || !args[0]) return NULL;
    
    size_t total_len = 0;
    for (int i = 0; args[i]; i++) {
        total_len += strlen(args[i]) + 3; // +3 for quotes and space
    }
    total_len += 1; // null terminator
    
    char *cmd_line = malloc(total_len);
    if (!cmd_line) return NULL;
    
    cmd_line[0] = '\0';
    for (int i = 0; args[i]; i++) {
        if (i > 0) strcat(cmd_line, " ");
        
        // Quote arguments that contain spaces
        if (strchr(args[i], ' ')) {
            strcat(cmd_line, "\"");
            strcat(cmd_line, args[i]);
            strcat(cmd_line, "\"");
        } else {
            strcat(cmd_line, args[i]);
        }
    }
    
    return cmd_line;
}

// Find executable in PATH or current directory (Windows)
char *find_executable_windows(const char *program) {
    if (!program) return NULL;
    
    // If program contains path separators, use as is
    if (strchr(program, '\\') || strchr(program, '/')) {
        return strdup(program);
    }
    
    // Try adding .exe extension if not present
    char *exe_name = malloc(strlen(program) + 5);
    strcpy(exe_name, program);
    if (!strstr(program, ".exe")) {
        strcat(exe_name, ".exe");
    }
    
    // Check if file exists in current directory
    if (GetFileAttributesA(exe_name) != INVALID_FILE_ATTRIBUTES) {
        return exe_name;
    }
    
    // Search in PATH
    char *path_env = getenv("PATH");
    if (path_env) {
        char *path_copy = strdup(path_env);
        char *path_dir = strtok(path_copy, ";");
        
        while (path_dir) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s\\%s", path_dir, exe_name);
            
            if (GetFileAttributesA(full_path) != INVALID_FILE_ATTRIBUTES) {
                free(path_copy);
                free(exe_name);
                return strdup(full_path);
            }
            
            path_dir = strtok(NULL, ";");
        }
        free(path_copy);
    }
    
    free(exe_name);
    return strdup(program); // Return original if not found
}

// Execute piped commands on Windows
int execute_piped_commands_windows(command_t *commands) {
    if (!commands) return -1;
    
    // Count commands in pipeline
    int cmd_count = 0;
    command_t *cmd = commands;
    while (cmd) {
        cmd_count++;
        cmd = cmd->next;
    }
    
    if (cmd_count == 1) {
        // Single command, no pipes needed
        return execute_single_command_windows(commands);
    }
    
    // Create pipes for communication
    HANDLE *read_handles = malloc(cmd_count * sizeof(HANDLE));
    HANDLE *write_handles = malloc(cmd_count * sizeof(HANDLE));
    PROCESS_INFORMATION *processes = malloc(cmd_count * sizeof(PROCESS_INFORMATION));
    
    if (!read_handles || !write_handles || !processes) {
        free(read_handles);
        free(write_handles);
        free(processes);
        return -1;
    }
    
    // Initialize handles
    for (int i = 0; i < cmd_count; i++) {
        read_handles[i] = INVALID_HANDLE_VALUE;
        write_handles[i] = INVALID_HANDLE_VALUE;
        memset(&processes[i], 0, sizeof(PROCESS_INFORMATION));
    }
    
    // Create pipes between commands
    for (int i = 0; i < cmd_count - 1; i++) {
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        if (!CreatePipe(&read_handles[i + 1], &write_handles[i], &sa, 0)) {
            fprintf(stderr, "xsh: failed to create pipe\n");
            // Cleanup and return
            for (int j = 0; j <= i; j++) {
                if (read_handles[j] != INVALID_HANDLE_VALUE) CloseHandle(read_handles[j]);
                if (write_handles[j] != INVALID_HANDLE_VALUE) CloseHandle(write_handles[j]);
            }
            free(read_handles);
            free(write_handles);
            free(processes);
            return -1;
        }
    }
    
    // Start each process
    cmd = commands;
    for (int i = 0; i < cmd_count; i++) {
        STARTUPINFOA si;
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        
        // Set up standard handles for this process
        si.hStdInput = (i == 0) ? GetStdHandle(STD_INPUT_HANDLE) : read_handles[i];
        si.hStdOutput = (i == cmd_count - 1) ? GetStdHandle(STD_OUTPUT_HANDLE) : write_handles[i];
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        char *cmd_line = build_command_line_windows(cmd->args);
        char *executable = find_executable_windows(cmd->args[0]);
        
        // Check if it's a built-in command first
        if (!CreateProcessA(executable, cmd_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &processes[i])) {
            // If CreateProcess fails, check if it's a built-in command
            if (xsh_builtin_exists(cmd->args[0])) {
                fprintf(stderr, "xsh: built-in command '%s' cannot be used in pipelines on Windows\n", cmd->args[0]);
            } else {
                fprintf(stderr, "xsh: failed to create process for '%s'\n", cmd->args[0]);
            }
            free(cmd_line);
            free(executable);
            
            // Cleanup
            for (int j = 0; j < cmd_count; j++) {
                if (read_handles[j] != INVALID_HANDLE_VALUE) CloseHandle(read_handles[j]);
                if (write_handles[j] != INVALID_HANDLE_VALUE) CloseHandle(write_handles[j]);
                if (processes[j].hProcess) {
                    TerminateProcess(processes[j].hProcess, 1);
                    CloseHandle(processes[j].hProcess);
                    CloseHandle(processes[j].hThread);
                }
            }
            free(read_handles);
            free(write_handles);
            free(processes);
            return -1;
        }
        
        free(cmd_line);
        free(executable);
        cmd = cmd->next;
    }
    
    // Close pipe handles that are no longer needed
    for (int i = 0; i < cmd_count - 1; i++) {
        CloseHandle(read_handles[i + 1]);
        CloseHandle(write_handles[i]);
    }
    
    // Wait for all processes to complete
    HANDLE *process_handles = malloc(cmd_count * sizeof(HANDLE));
    for (int i = 0; i < cmd_count; i++) {
        process_handles[i] = processes[i].hProcess;
    }
    
    WaitForMultipleObjects(cmd_count, process_handles, TRUE, INFINITE);
    
    // Get exit code from last process
    DWORD exit_code = 0;
    GetExitCodeProcess(processes[cmd_count - 1].hProcess, &exit_code);
    last_command_exit_status = (int)exit_code;
    
    // Cleanup
    for (int i = 0; i < cmd_count; i++) {
        CloseHandle(processes[i].hProcess);
        CloseHandle(processes[i].hThread);
    }
    
    free(read_handles);
    free(write_handles);
    free(processes);
    free(process_handles);
    
    return exit_code;
}

// Execute single command on Windows
int execute_single_command_windows(command_t *cmd) {
    if (!cmd || !cmd->args || !cmd->args[0]) return -1;
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    
    // Setup redirections if needed
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    
    // Handle redirections
    HANDLE input_file = INVALID_HANDLE_VALUE;
    HANDLE output_file = INVALID_HANDLE_VALUE;
    HANDLE error_file = INVALID_HANDLE_VALUE;
    
    if (cmd->input_redir.type == REDIR_IN) {
        input_file = CreateFileA(cmd->input_redir.filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (input_file != INVALID_HANDLE_VALUE) {
            si.hStdInput = input_file;
        }
    }
    
    if (cmd->output_redir.type == REDIR_OUT) {
        output_file = CreateFileA(cmd->output_redir.filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (output_file != INVALID_HANDLE_VALUE) {
            si.hStdOutput = output_file;
        }
    } else if (cmd->output_redir.type == REDIR_APPEND) {
        output_file = CreateFileA(cmd->output_redir.filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (output_file != INVALID_HANDLE_VALUE) {
            SetFilePointer(output_file, 0, NULL, FILE_END);
            si.hStdOutput = output_file;
        }
    }
    
    if (cmd->error_redir.type == REDIR_ERR) {
        error_file = CreateFileA(cmd->error_redir.filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (error_file != INVALID_HANDLE_VALUE) {
            si.hStdError = error_file;
        }
    }
    
    char *cmd_line = build_command_line_windows(cmd->args);
    char *executable = find_executable_windows(cmd->args[0]);
    
    BOOL success = CreateProcessA(executable, cmd_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    
    free(cmd_line);
    free(executable);
    
    if (!success) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            fprintf(stderr, "xsh: command not found: %s\n", cmd->args[0]);
        } else {
            fprintf(stderr, "xsh: failed to execute command: %s\n", cmd->args[0]);
        }
        
        // Close file handles
        if (input_file != INVALID_HANDLE_VALUE) CloseHandle(input_file);
        if (output_file != INVALID_HANDLE_VALUE) CloseHandle(output_file);
        if (error_file != INVALID_HANDLE_VALUE) CloseHandle(error_file);
        
        return -1;
    }
    
    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    // Get exit code
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    last_command_exit_status = (int)exit_code;
    
    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    if (input_file != INVALID_HANDLE_VALUE) CloseHandle(input_file);
    if (output_file != INVALID_HANDLE_VALUE) CloseHandle(output_file);
    if (error_file != INVALID_HANDLE_VALUE) CloseHandle(error_file);
    
    return exit_code;
}

#endif // _WIN32

#ifndef _WIN32
// POSIX-specific execution functions

// Execute a single command with POSIX
int execute_single_command_posix(command_t *cmd) {
    if (!cmd || !cmd->args || !cmd->args[0]) return -1;
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (setup_redirections(cmd) == -1) {
            exit(1);
        }
        
        if (execvp(cmd->args[0], cmd->args) == -1) {
            perror("xsh");
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        restore_redirections();
        
        if (WIFEXITED(status)) {
            last_command_exit_status = WEXITSTATUS(status);
            return WEXITSTATUS(status);
        } else {
            last_command_exit_status = 1;
            return 1;
        }
    } else {
        perror("xsh: fork");
        return -1;
    }
    
    return 0;
}

// Execute piped commands with POSIX
int execute_piped_commands_posix(command_t *commands) {
    if (!commands) return -1;
    
    // Count commands
    int cmd_count = 0;
    command_t *cmd = commands;
    while (cmd) {
        cmd_count++;
        cmd = cmd->next;
    }
    
    if (cmd_count == 1) {
        return execute_single_command_posix(commands);
    }
    
    // Create pipes
    int pipes[cmd_count - 1][2];
    for (int i = 0; i < cmd_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("xsh: pipe");
            return -1;
        }
    }
    
    // Execute commands
    pid_t pids[cmd_count];
    cmd = commands;
    
    for (int i = 0; i < cmd_count; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Setup pipes
            if (i > 0) {
                // Not first command, read from previous pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            if (i < cmd_count - 1) {
                // Not last command, write to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Setup redirections for this command
            if (i == 0 && cmd->input_redir.type != REDIR_NONE) {
                // First command input redirection
                if (cmd->input_redir.type == REDIR_IN) {
                    int fd = open(cmd->input_redir.filename, O_RDONLY);
                    if (fd == -1) {
                        perror("xsh: input redirection");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            }
            
            if (i == cmd_count - 1) {
                // Last command output redirection
                if (cmd->output_redir.type == REDIR_OUT) {
                    int fd = open(cmd->output_redir.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("xsh: output redirection");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                } else if (cmd->output_redir.type == REDIR_APPEND) {
                    int fd = open(cmd->output_redir.filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (fd == -1) {
                        perror("xsh: append redirection");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }
            
            // Error redirection for any command
            if (cmd->error_redir.type == REDIR_ERR) {
                int fd = open(cmd->error_redir.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("xsh: error redirection");
                    exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            
            // Execute command
            if (xsh_builtin_exists(cmd->args[0])) {
                // Built-in command in pipeline
                int result = xsh_execute_builtin(cmd->args);
                // Convert shell return value to exit code
                exit(result == 0 ? 0 : 0); // Built-in commands return 0 for success when they don't exit shell
            } else {
                // External command
                if (execvp(cmd->args[0], cmd->args) == -1) {
                    perror("xsh");
                    exit(1);
                }
            }
        } else if (pids[i] < 0) {
            perror("xsh: fork");
            return -1;
        }
        
        cmd = cmd->next;
    }
    
    // Parent process: close all pipe file descriptors
    for (int i = 0; i < cmd_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children
    int last_status = 0;
    for (int i = 0; i < cmd_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (i == cmd_count - 1) { // Last command determines overall exit status
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else {
                last_status = 1;
            }
        }
    }
    
    last_command_exit_status = last_status;
    return last_status;
}

#endif // !_WIN32

// Main pipeline execution function
int execute_pipeline(pipeline_t *pipeline) {
    if (!pipeline || !pipeline->commands) return -1;
    
    command_t *cmd = pipeline->commands;
    int overall_status = 0;
    
    while (cmd) {
        // Check if this is a pipeline (has pipe operator)
        if (cmd->operator == CMD_PIPE) {
            // Find the end of the pipeline
            command_t *pipe_start = cmd;
            command_t *pipe_end = cmd;
            while (pipe_end->next && pipe_end->operator == CMD_PIPE) {
                pipe_end = pipe_end->next;
            }
            
            // Execute the piped commands
#ifdef _WIN32
            overall_status = execute_piped_commands_windows(pipe_start);
#else
            overall_status = execute_piped_commands_posix(pipe_start);
#endif
            
            // Move to the command after the pipeline
            cmd = pipe_end->next;
        } else {
            // Single command or start of new pipeline
            
            // Check if it's a built-in command first
            if (cmd->args && cmd->args[0] && xsh_builtin_exists(cmd->args[0])) {
                // Setup redirections for built-in
                int saved_stdin = -1, saved_stdout = -1, saved_stderr = -1;
                
                // Save original descriptors
                saved_stdin = dup(STDIN_FILENO);
                saved_stdout = dup(STDOUT_FILENO);
                saved_stderr = dup(STDERR_FILENO);
                
                if (setup_redirections(cmd) == 0) {
                    int builtin_result = xsh_execute_builtin(cmd->args);
                    // Convert built-in return value (0 = exit, 1 = continue) to exit status
                    overall_status = (builtin_result == 0) ? 0 : 0; // Built-in success
                } else {
                    overall_status = 1; // Setup error
                }
                
                // Restore redirections
                if (saved_stdin != -1) {
                    dup2(saved_stdin, STDIN_FILENO);
                    close(saved_stdin);
                }
                if (saved_stdout != -1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);
                }
                if (saved_stderr != -1) {
                    dup2(saved_stderr, STDERR_FILENO);
                    close(saved_stderr);
                }
            } else {
                // External command
#ifdef _WIN32
                overall_status = execute_single_command_windows(cmd);
#else
                overall_status = execute_single_command_posix(cmd);
#endif
            }
            
            // Handle command operators
            if (cmd->operator == CMD_AND && overall_status != 0) {
                // Command failed, skip until next non-AND command
                while (cmd->next && cmd->next->operator == CMD_AND) {
                    cmd = cmd->next;
                }
                if (cmd->next) cmd = cmd->next;
            } else if (cmd->operator == CMD_OR && overall_status == 0) {
                // Command succeeded, skip until next non-OR command
                while (cmd->next && cmd->next->operator == CMD_OR) {
                    cmd = cmd->next;
                }
                if (cmd->next) cmd = cmd->next;
            } else {
                cmd = cmd->next;
            }
        }
        
        last_command_exit_status = overall_status;
        
        // For semicolon, continue regardless of status
        // For && and ||, the logic is handled above
    }
    
    return overall_status;
}

// Main execution function - updated to handle advanced features
int xsh_launch(char **args) {
    if (!args || !args[0]) return 1;
    
    // Reconstruct the command line to check for operators
    size_t total_len = 0;
    for (int i = 0; args[i]; i++) {
        total_len += strlen(args[i]) + 1;
    }
    
    char *line = malloc(total_len);
    if (!line) return 1;
    
    strcpy(line, args[0]);
    for (int i = 1; args[i]; i++) {
        strcat(line, " ");
        strcat(line, args[i]);
    }
    
    // Check if the line contains operators
    if (!contains_operators(line)) {
        free(line);
        // Simple command, use original logic for built-ins
        if (xsh_builtin_exists(args[0])) {
            return xsh_execute_builtin(args);
        }
        
        // External command
#ifdef _WIN32
        command_t *cmd = create_command();
        if (!cmd) return 1;
        
        // Copy args
        int arg_count = 0;
        while (args[arg_count]) arg_count++;
        
        cmd->args = malloc((arg_count + 1) * sizeof(char*));
        if (!cmd->args) {
            free_command(cmd);
            return 1;
        }
        
        for (int i = 0; i < arg_count; i++) {
            cmd->args[i] = strdup(args[i]);
        }
        cmd->args[arg_count] = NULL;
        
        int result = execute_single_command_windows(cmd);
        free_command(cmd);
        return (result == 0) ? 1 : 1; // Continue shell loop
#else
        // POSIX simple execution
        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("xsh");
            }
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                last_command_exit_status = WEXITSTATUS(status);
            }
        } else {
            perror("xsh: fork");
        }
        return 1; // Continue shell loop
#endif
    }
    
    // Complex command with operators
    int token_count;
    char **tokens = tokenize_with_operators(line, &token_count);
    free(line);
    
    if (!tokens) return 1;
    
    pipeline_t *pipeline = parse_command_line(tokens, token_count);
    
    // Free tokens
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    if (!pipeline) return 1;
    
    int result = execute_pipeline(pipeline);
    free_pipeline(pipeline);
    
    return 1; // Continue shell loop
}
