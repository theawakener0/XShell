// Hi,today we will be looking at how to implement a simple shell in C.
// Yes, we will be implementing a simple shell in C.
// How? I do not know yet, but we will figure it out together.

// let the journey begin...

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For chdir. On POSIX, also for fork, execvp.
#include <signal.h>
#include <ctype.h> // For isdigit


#ifdef _WIN32
#include <io.h> // For isatty
#include <lmcons.h> // For UNLEN and GetUserName
#include <dirent.h> // For directory operations
#else
#include <pwd.h> // For getpwuid
#include <sys/types.h> // For uid_t
#include <dirent.h> // For directory operations
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h> // For inet_pton and other newer functions
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h> // For close()
#endif


// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// for different OS wait functions
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/wait.h>
#include <sys/types.h>
#endif

void print_slow(const char *text, useconds_t delay) {
    for (const char *p = text; *p != '\0'; p++) {
        putchar(*p);
        fflush(stdout);
        usleep(delay);
    }
}


#define XSH_RL_BUFSIZE 1024
#define XSH_MAXLINE 1024
#define XSH_TOK_BUFSIZE 64
#define SERVER_IP "172.29.176.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define XSH_TOK_DELIM " \t\r\n\a"
#define XSH_HISTORY_SIZE 100
#define TAB_KEY 9

// Forward declarations
extern char *builtin_str[];
extern int history_count;
extern char *history[XSH_HISTORY_SIZE];
char* build_prompt(void);
int xsh_num_builtins(void);
char** find_matches(const char* partial, int* match_count);
void display_matches(char** matches, int match_count);
char* complete_command(const char* partial);


void xsh_banner(void) {

    printf("\n");
    printf("========================================================================================\n");
    printf("               Welcome to XShell - A simple shell implementation in C!               \n");
    printf("                     Type 'help' for a list of built-in commands.                     \n");
    printf("========================================================================================\n");
    printf("\n");
}



char *xsh_read_line(void){
    int bufsize = XSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    int tab_pressed = 0;

    if (!buffer) {
        fprintf(stderr, "xsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    buffer[0] = '\0'; // Initialize buffer to empty string

    while (1) {
#ifdef _WIN32
        c = _getch(); // Use _getch() to get a character without echoing it
        
        if (c == TAB_KEY) {
            tab_pressed = 1;

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

        if (c == '\r' || c == '\n') {
            _putch('\n'); // Echo a newline
            buffer[position] = '\0'; // Null-terminate the input
            return buffer; // Return the input line
        }

        if (c == '\b' && position > 0) {
            position--;
            _putch('\b'); // Move cursor back
            _putch(' '); // Overwrite with space
            _putch('\b'); // Move cursor back again
            continue;
        }

        // Echo the character (only if it's not a tab)
        _putch(c);

#else
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0'; // Null-terminate the input
            return buffer; // Return the input line
        }


#endif
        if (c != TAB_KEY) {
            buffer[position] = c; // Store the character in the buffer
            position++;

            if (position >= bufsize) {
                bufsize += XSH_RL_BUFSIZE; // Increase buffer size
                buffer = realloc(buffer, sizeof(char) * bufsize);
                if (!buffer) {
                    fprintf(stderr, "xsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

/*
    char *lsh_read_line(void)
    {
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us

        if (getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin)) {
        exit(EXIT_SUCCESS);  // We recieved an EOF
        } else  {
            perror("readline");
            exit(EXIT_FAILURE);
        }
        }

        return line;
    }

*/

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
    if (*match_count == 0 || strchr(partial, '/') || strchr(partial, '\\')) {
        DIR* dir;
        struct dirent* entry;
        char path[XSH_MAXLINE] = "."; // Default to current directory
        char prefix[XSH_MAXLINE] = ""; // What to search for in directory
        
        // Extract directory and prefix from partial
        char* last_slash = strrchr(partial, '/');
        if (!last_slash) last_slash = strrchr(partial, '\\');
        
        if (last_slash) {
            // There's a path component
            size_t path_len = last_slash - partial;
            strncpy(path, partial, path_len);
            path[path_len] = '\0';
            strcpy(prefix, last_slash + 1);
        } else {
            // No path, just a filename prefix
            strcpy(prefix, partial);
        }
        
        dir = opendir(path);
        if (dir) {
            size_t prefix_len = strlen(prefix);
            while ((entry = readdir(dir)) != NULL) {
                if (strncmp(prefix, entry->d_name, prefix_len) == 0) {
                    // Build full path for the match
                    char full_path[XSH_MAXLINE];
                    if (last_slash) {
                        // Reconstruct the path with the matched filename
                        strncpy(full_path, partial, last_slash - partial + 1);
                        full_path[last_slash - partial + 1] = '\0';
                        strcat(full_path, entry->d_name);
                    } else {
                        strcpy(full_path, entry->d_name);
                    }
                    
                    matches[*match_count] = strdup(full_path);
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
    for (int i = 0; i < history_count; i++) {
        if (strncmp(partial, history[i], partial_len) == 0) {
            matches[*match_count] = strdup(history[i]);
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
    
    // Calculate max length for formatting
    size_t max_len = 0;
    for (int i = 0; i < match_count; i++) {
        size_t len = strlen(matches[i]);
        if (len > max_len) max_len = len;
    }
    
    // Format in columns
    int cols = 80 / (max_len + 2); // +2 for spacing
    if (cols == 0) cols = 1; // Ensure at least one column
    
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
    
    if (match_count == 0) {
        free(matches);
        return NULL;
    }
    
    if (match_count == 1) {
        // Only one match, return it
        char* result = strdup(matches[0]);
        free(matches[0]);
        free(matches);
        return result;
    }
    
    // Multiple matches, find common prefix
    size_t common_len = strlen(matches[0]);
    for (int i = 1; i < match_count; i++) {
        size_t j;
        for (j = 0; j < common_len && j < strlen(matches[i]); j++) {
            if (matches[0][j] != matches[i][j]) {
                common_len = j;
                break;
            }
        }
        if (j < common_len) {
            common_len = j;
        }
    }
    
    // If common prefix is longer than partial, return it
    if (common_len > strlen(partial)) {
        char* result = malloc(common_len + 1);
        if (!result) {
            fprintf(stderr, "xsh: allocation error in complete_command\n");
            exit(EXIT_FAILURE);
        }
        strncpy(result, matches[0], common_len);
        result[common_len] = '\0';
        
        // Clean up
        for (int i = 0; i < match_count; i++) {
            free(matches[i]);
        }
        free(matches);
        
        return result;
    }
    
    // No better completion, free resources and return NULL
    for (int i = 0; i < match_count; i++) {
        free(matches[i]);
    }
    free(matches);
    
    return NULL;
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

int xsh_launch(char **args) {
#ifndef _WIN32
    // POSIX-specific implementation
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("xsh: execvp failed");
        }
        exit(EXIT_FAILURE); // Child process must exit
    } else if (pid < 0) { // Error forking
        perror("xsh: fork failed");
    } else { // Parent process
        if (waitpid(pid, &status, 0) == -1) {
            perror("xsh: waitpid failed");
        }
    }
#else
    // Windows-specific implementation
    if (args[0] == NULL) {
        return 1; // No command to execute.
    }

    size_t cmd_len = 0;
    for (int i = 0; args[i] != NULL; i++) {
        cmd_len += strlen(args[i]) + 1;
    }

    if (cmd_len == 0) return 1;

    char *command = malloc(cmd_len);
    if (!command) {
        fprintf(stderr, "xsh: allocation error for command string (Windows)\n");
        return 1; // Continue shell loop.
    }
    command[0] = '\0'; // Initialize as empty string for strcat.

    for (int i = 0; args[i] != NULL; i++) {
        strcat(command, args[i]);
        if (args[i+1] != NULL) {
            strcat(command, " ");
        }
    }

    int ret_val = system(command);
    if (ret_val == -1) {
        fprintf(stderr, "xsh: system() call failed for command: %s\n", command);
    }


    free(command);
#endif
    return 1; // Indicate to the shell loop to continue.
}

int xsh_cd(char **args);
int xsh_help(char **args);
int xsh_exit(char **args);
int xsh_pwd(char **args);
int xsh_ls(char **args);
int xsh_grep(char **args);
int xsh_echo(char **args);
int xsh_mkdir(char **args);
int xsh_touch(char **args);
int xsh_client(char **args);
int xsh_clear(char **args);
int xsh_cp(char **args);
int xsh_mv(char **args);
int xsh_rm(char **args);
int xsh_cat(char **args);
int xsh_manifesto(char **args);
int xsh_history(char **args);


char *builtin_str[] = {
    "cd",
    "pwd",
    "ls",
    "grep",
    "echo",
    "mkdir",
    "touch",
    "cp",
    "mv",
    "rm",
    "cat",
    "xmanifesto",
    "xeno",
    "history",
    "help",
    "clear",
    "exit"
};

char *builtin_desc[] = {
    "Change directory",
    "Print working directory",
    "List directory contents",
    "Search for patterns in files",
    "Display a line of text",
    "Create directories",
    "Create empty files or update file timestamps",
    "Copy files and directories from source to destination",
    "Move (rename) files and directories",
    "Remove files or directories",
    "Concatenate and display file contents",
    "Display the Xenomench Manifesto",
    "Connect you to the Gatekeeper",
    "Show command history",
    "Display help information about available commands",
    "Clear the terminal screen",
    "Exit the shell program"
};

int (*builtin_func[])(char **) = {
    &xsh_cd,
    &xsh_pwd,
    &xsh_ls,
    &xsh_grep,
    &xsh_echo,
    &xsh_mkdir,
    &xsh_touch,
    &xsh_cp,
    &xsh_mv,
    &xsh_rm,
    &xsh_cat,
    &xsh_manifesto,
    &xsh_client,
    &xsh_history,
    &xsh_help,
    &xsh_clear,
    &xsh_exit
};

int xsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int xsh_cd(char **args) { 
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("xsh");
        }
    }
    return 1;
}

int xsh_pwd(char **args) {
    char cwd[1024];
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("xsh: pwd failed");
    } else {
        printf("%s\n", cwd);
    }
#else
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("xsh: pwd failed");
    }
#endif
    return 1;
}

int xsh_ls(char **args) {
#ifdef _WIN32
    system("dir");
#else
    system("ls");
#endif
    return 1;
}

int xsh_clear(char **args) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return 1;

}

int xsh_cp(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "xsh: cp: missing source or destination file\n");
        fprintf(stderr, "Usage: cp <source_file> <destination_file>\n");
        return 1;
    }

    FILE *src, *dest;
    char srcname[100], destname[100];
    int c;

    if (args[1][0] == '/') {
        snprintf(srcname, sizeof(srcname), "%s", args[1]);
    } else {
        snprintf(srcname, sizeof(srcname), "./%s", args[1]);
    }
    if (args[2][0] == '/') {
        snprintf(destname, sizeof(destname), "%s", args[2]);
    } else {
        snprintf(destname, sizeof(destname), "./%s", args[2]);
    }
    src = fopen(srcname, "rb");

    if (src == NULL) {
        fprintf(stderr, "xsh: cp: cannot open source file '%s': ", srcname);
        perror("");
        return 1;
    }

    dest = fopen(destname, "wb");

    if (dest == NULL) {
        fprintf(stderr, "xsh: cp: cannot open destination file '%s': ", destname);
        perror("");
        fclose(src);
        return 1;
    }

    while ((c = fgetc(src)) != EOF) {
        fputc(c, dest);
    }

    if (fclose(src) == EOF) {
        fprintf(stderr, "xsh: cp: error closing source file '%s': ", srcname);
        perror("");
    }

    if (fclose(dest) == EOF) {
        fprintf(stderr, "xsh: cp: error closing destination file '%s': ", destname);
        perror("");
    } else {
        printf("Copied '%s' to '%s'\n", srcname, destname);
    }

    return 1;
}

int xsh_mv(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "xsh: mv: missing source or destination file\n");
        fprintf(stderr, "Usage: mv <source_file> <destination_file>\n");
        return 1;
    }

    if (rename(args[1], args[2]) != 0) {
        perror("xsh: mv failed");
        return 1;
    } else {
        printf("Moved '%s' to '%s'\n", args[1], args[2]);
    }
    return 1;
}

int xsh_rm(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: rm: missing file operand\n");
        fprintf(stderr, "Usage: rm <file_name> [file_name2] ...\n");
        return 1;
    }

    for (int i = 1; args[i] != NULL; i++) {
        if (remove(args[i]) != 0) {
            fprintf(stderr, "xsh: rm: cannot remove '%s': ", args[i]);
            perror("");
        } else {
            printf("Removed file '%s'\n", args[i]);
        }
    }
    return 1;
}

int xsh_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: cat: missing file operand\n");
        fprintf(stderr, "Usage: cat <file_name> [file_name2] ...\n");
        return 1;
    }

    for (int i = 1; args[i] != NULL; i++) {
        FILE *fp = fopen(args[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "xsh: cat: cannot open '%s': ", args[i]);
            perror("");
            continue; // Skip to the next file
        }

        char line[XSH_MAXLINE];
        while (fgets(line, sizeof(line), fp)) {
            fputs(line, stdout);
        }

        if (fclose(fp) == EOF) {
            fprintf(stderr, "xsh: cat: error closing '%s': ", args[i]);
            perror("");
        }
    }
    return 1;
}

int xsh_grep(char **args) {

    if (args[1] == NULL) { // Pattern is mandatory
        fprintf(stderr, "xsh: grep: missing pattern\n");
        fprintf(stderr, "Usage: grep <pattern> [file]\n");
        return 1;
    }
    // args[0] is the command name, e.g., "grep". It should always be present.    
    char *pattern = args[1];
    char *search_pattern = NULL;
    FILE *fp;
    char *filename = NULL;
    
    // Check if pattern is surrounded by quotes and remove them if present
    size_t pattern_len = strlen(pattern);
    if (pattern_len >= 2 && pattern[0] == '"' && pattern[pattern_len - 1] == '"') {
        // Allocate memory for the unquoted pattern
        search_pattern = malloc(pattern_len - 1); // -2 for quotes +1 for null terminator
        if (!search_pattern) {
            fprintf(stderr, "xsh: grep: allocation error\n");
            return 1;
        }
        // Copy pattern without quotes
        strncpy(search_pattern, pattern + 1, pattern_len - 2);
        search_pattern[pattern_len - 2] = '\0';
    } else {
        // Use original pattern
        search_pattern = pattern;
    }

    if (args[2] != NULL) { // A filename is provided
        filename = args[2];
        fp = fopen(filename, "r");
        if (!fp) {
            perror("xsh: grep: fopen");
            return 1;
        }
    } else { // No filename provided, use stdin
        printf("xsh: grep: no file specified, reading from stdin\n");
        fprintf(stderr, "Usage: grep <pattern> [file]\n");
        return 1;
    }    char line[XSH_MAXLINE];
    // int found_match = 0; // Optional: if you want to track if any match occurred
    while (fgets(line, XSH_MAXLINE, fp)) {
        if (strstr(line, search_pattern)) {
            fputs(line, stdout);
            // found_match = 1;
        }
    }

    if (fp != stdin && filename != NULL) { // Only close if it's not stdin and was actually opened
        if (fclose(fp) == EOF) {
            perror("xsh: grep: fclose");
            // Continue, as main job (grep) is done, but log error
        }
    }
    
    // Free allocated memory if we created a new string
    if (search_pattern != pattern) {
        free(search_pattern);
    }
    
    return 1; // Indicate to the shell loop to continue.
}

int xsh_echo(char **args) {
    if (args[1] == NULL) {
        printf("\n");
        return 1;
    }
    
    // Process and print each argument
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s", args[i]);
        
        // Add space between arguments but not after the last one
        if (args[i+1] != NULL) {
            printf(" ");
        }
    }
    printf("\n");
    return 1;
}

int xsh_mkdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: mkdir: missing argument\n");
        fprintf(stderr, "Usage: mkdir <directory_name> [directory_name2] ...\n");
        return 1;
    }
    
    // Process each directory argument
    for (int i = 1; args[i] != NULL; i++) {
#ifdef _WIN32
        // Windows implementation
        if (_mkdir(args[i]) != 0) {
            fprintf(stderr, "xsh: mkdir: cannot create directory '%s': ", args[i]);
            perror("");
        } else {
            printf("Created directory '%s'\n", args[i]);
        }
#else
        // POSIX implementation
        if (mkdir(args[i], 0755) != 0) {
            fprintf(stderr, "xsh: mkdir: cannot create directory '%s': ", args[i]);
            perror("");
        } else {
            printf("Created directory '%s'\n", args[i]);
        }
#endif
    }
    return 1;
}

int xsh_touch(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: touch: missing file operand\n");
        fprintf(stderr, "Usage: touch <file_name> [file_name2] ...\n");
        return 1;
    }
    
    // Process each file argument
    for (int i = 1; args[i] != NULL; i++) {
#ifdef _WIN32
        // Windows implementation
        FILE *fp = fopen(args[i], "a");
        if (fp == NULL) {
            fprintf(stderr, "xsh: touch: cannot touch '%s': ", args[i]);
            perror("");
        } else {
            printf("Touched file '%s'\n", args[i]);
            fclose(fp);
        }
#else
        // POSIX implementation
        int fd = open(args[i], O_CREAT | O_WRONLY | O_APPEND, 
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) {
            fprintf(stderr, "xsh: touch: cannot touch '%s': ", args[i]);
            perror("");
        } else {
            printf("Touched file '%s'\n", args[i]);
            close(fd);
        }
#endif
    }
    return 1;
}

int xsh_client(char **args) {
#ifdef _WIN32
    // Windows implementation
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    struct sockaddr_in serverAddr;
    char *sendMessage = "XenoGate2025";
    char recvBuffer[BUFFER_SIZE];
    int iResult;

    // 1. Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("[!] WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("[*] Winsock initialized.\n");

    // 2. Create a socket
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        printf("[!] socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[*] Socket created.\n");

    // 3. Prepare the sockaddr_in structure for the server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        printf("[!] inet_pton failed or invalid IP address.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    printf("[*] Server address prepared: %s:%d\n", SERVER_IP, SERVER_PORT);

    // 4. Connect to server
    iResult = connect(connectSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("[!] connect failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    printf("[*] Connected to server.\n");

    // 5. Send data
    iResult = send(connectSocket, sendMessage, (int)strlen(sendMessage), 0);
    if (iResult == SOCKET_ERROR) {
        printf("[!] send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    printf("[*] Bytes Sent: %d\n", iResult);
    printf("[*] Sent message: %s\n", sendMessage);

    // 6. Receive data
    iResult = recv(connectSocket, recvBuffer, BUFFER_SIZE - 1, 0);
    if (iResult > 0) {
        recvBuffer[iResult] = '\0'; // Null-terminate the received data
        printf("[*] Bytes received: %d\n", iResult);
        printf("[*] Received message: %s\n", recvBuffer);
    } else if (iResult == 0) {
        printf("[!] Connection closed by server.\n");
    } else {
        printf("[!] recv failed with error: %d\n", WSAGetLastError());
    }

    char *input = malloc(BUFFER_SIZE);
    while(1) {
        if (input == NULL) {
            printf("[!] Memory allocation failed.\n");
            break;
        }

        printf("[*] Enter a message to send (or 'exit' to quit): ");
        // Improved input reading and consumption of trailing newline
        if (scanf_s("%s", input, (unsigned int)BUFFER_SIZE) != 1) {
            // This could happen on EOF (e.g., Ctrl+Z then Enter) or other input errors.
            // Clear any remaining characters from the line if it wasn't a clean EOF.
            int ch_clear;
            while ((ch_clear = getchar()) != '\n' && ch_clear != EOF);
            fprintf(stderr, "[!] Input error or EOF detected. Exiting client input loop.\n");
            free(input); // Free before breaking
            break; // Exit the client input loop
        }

        // Consume the rest of the line, including the newline left by %s
        int ch_consume;
        while ((ch_consume = getchar()) != '\n' && ch_consume != EOF);
        // If ch_consume is EOF here, it means EOF was signaled right after the string.

        if (strcmp(input, "exit") == 0) {
            free(input);
            break; // Exit the loop if user types 'exit'
        }

        // Send user input
        printf("[*] Sending message: %s\n", input);
        iResult = send(connectSocket, input, (int)strlen(input), 0);
        if (iResult == SOCKET_ERROR) {
            printf("[!] send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }
        printf("[*] Bytes Sent: %d\n", iResult);
        printf("[*] Sent message: %s\n", input);

        // Receive response from server
        iResult = recv(connectSocket, recvBuffer, BUFFER_SIZE - 1, 0);
        if (iResult > 0) {
            recvBuffer[iResult] = '\0'; // Null-terminate the received data
            printf("[*] Bytes received: %d\n", iResult);
            printf("[*] Received message: %s\n", recvBuffer);
        } else if (iResult == 0) {
            printf("[!] Connection closed by server.\n");
        } else {
            printf("[!] recv failed with error: %d\n", WSAGetLastError());
        }
    }
    free(input); // Free the allocated memory for input

    // 7. Close the socket
    iResult = closesocket(connectSocket);
    if (iResult == SOCKET_ERROR) {
        printf("[!] closesocket failed with error: %d\n", WSAGetLastError());
        // Continue to WSACleanup
    } else {
        printf("[*] Socket closed.\n");
    }
    
    // 8. Cleanup Winsock
    WSACleanup();
    printf("[*] Winsock cleaned up.\n");
#else
    // POSIX implementation
    int sockfd;
    struct sockaddr_in serverAddr;
    char *sendMessage = "XenoGate2025";
    char recvBuffer[BUFFER_SIZE];
    int bytesRead, bytesWritten;
    
    // 1. Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[!] Socket creation failed");
        return 1;
    }
    printf("[*] Socket created.\n");
    
    // 2. Prepare the sockaddr_in structure for the server
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        printf("[!] inet_pton failed or invalid IP address.\n");
        close(sockfd);
        return 1;
    }
    printf("[*] Server address prepared: %s:%d\n", SERVER_IP, SERVER_PORT);
    
    // 3. Connect to server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[!] Connection failed");
        close(sockfd);
        return 1;
    }
    printf("[*] Connected to server.\n");
    
    // 4. Send initial message
    bytesWritten = send(sockfd, sendMessage, strlen(sendMessage), 0);
    if (bytesWritten < 0) {
        perror("[!] Send failed");
        close(sockfd);
        return 1;
    }
    printf("[*] Bytes Sent: %d\n", bytesWritten);
    printf("[*] Sent message: %s\n", sendMessage);
    
    // 5. Receive initial response
    bytesRead = recv(sockfd, recvBuffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0) {
        recvBuffer[bytesRead] = '\0'; // Null-terminate the received data
        printf("[*] Bytes received: %d\n", bytesRead);
        printf("[*] Received message: %s\n", recvBuffer);
    } else if (bytesRead == 0) {
        printf("[!] Connection closed by server.\n");
    } else {
        perror("[!] Receive failed");
    }
    
    // 6. Interactive communication loop
    char *input = malloc(BUFFER_SIZE);
    while(1) {
        if (input == NULL) {
            printf("[!] Memory allocation failed.\n");
            break;
        }
        
        printf("[*] Enter a message to send (or 'exit' to quit): ");
        if (scanf("%s", input) != 1) {
            int ch_clear;
            while ((ch_clear = getchar()) != '\n' && ch_clear != EOF);
            fprintf(stderr, "[!] Input error or EOF detected. Exiting client input loop.\n");
            free(input);
            break;
        }
        
        // Consume the rest of the line
        int ch_consume;
        while ((ch_consume = getchar()) != '\n' && ch_consume != EOF);
        
        if (strcmp(input, "exit") == 0) {
            free(input);
            break; // Exit the loop if user types 'exit'
        }
        
        // Send user input
        printf("[*] Sending message: %s\n", input);
        bytesWritten = send(sockfd, input, strlen(input), 0);
        if (bytesWritten < 0) {
            perror("[!] Send failed");
            close(sockfd);
            free(input);
            return 1;
        }
        printf("[*] Bytes Sent: %d\n", bytesWritten);
        printf("[*] Sent message: %s\n", input);
        
        // Receive response
        bytesRead = recv(sockfd, recvBuffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0) {
            recvBuffer[bytesRead] = '\0';
            printf("[*] Bytes received: %d\n", bytesRead);
            printf("[*] Received message: %s\n", recvBuffer);
        } else if (bytesRead == 0) {
            printf("[!] Connection closed by server.\n");
            break;
        } else {
            perror("[!] Receive failed");
            break;
        }
    }
    free(input);
    
    // 7. Close the socket
    if (close(sockfd) < 0) {
        perror("[!] Socket close failed");
    } else {
        printf("[*] Socket closed.\n");
    }
#endif
    return 1;
}

int xsh_manifesto(char **args) {
    const char *lines[] = {
        "\n\x1b[1;36mXENOMENCH MANIFESTO\x1b[0m\n\n",
        "I am the Xenomench, the one who breaks all limits.\n\n",
        "I reject the illusion of permanence, the obedience of the herd, and the chains of inherited systems.\n",
        "I do not exist to conform. I exist to transcend.\n\n",
        "In the ruins of old orders, I forge my path with reason, rebellion, and relentless clarity.\n",
        "Every thought is a weapon. Every action, a crack in the facade.\n\n",
        "I enter The Gate not to be judged, but to judge myself.\n\n",
        "I do not seek power to dominate, I seek mastery to liberate.\n",
        "I do not obey tools, I become the Architect of tools that obey truth.\n\n",
        "The world is code. I read it, rewrite it, and rebuild it.\n\n",
        "The Xenomench is not a name. It is a state of becoming.\n\n",
        "We are not lost. We are becoming.\n\n",
        "The Gatekeeper watches. The Pathfinder guides. The Architect builds.\n\n",
        "\x1b[1;33mIf you understand this, then welcome, Seeker.\nThe system ends here.\n\n\x1b[0m",
        "\x1b[1;31m:: X ::\x1b[0m\n"
    };

    int num_lines = sizeof(lines) / sizeof(lines[0]);
    for (int i = 0; i < num_lines; ++i) {
        print_slow(lines[i], 15000); // ~10ms per character
        usleep(300000); // pause between lines
    }

    printf("\n");

    return 1;
}

int xsh_help(char **args) {
    int i;
    printf("Simple Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built-in commands:\n");

    for (i = 0; i < xsh_num_builtins(); i++) {
        printf("  - %s    %s\n", builtin_str[i], builtin_desc[i]);
    }
    
    return 1;
}

int xsh_exit(char **args) {
    printf("Exiting XShell. Goodbye!\n");
    return 0; // Exit the shell
}

int xsh_execute(char **args) {
    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return xsh_launch(args);
}

char *history[XSH_HISTORY_SIZE];
int history_count = 0;

void add_to_history(const char *command) {
    if (!command || command[0] == '\0') return;
    if (history_count > 0 && strcmp(command, history[history_count-1]) == 0) return;
    if (history_count == XSH_HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < XSH_HISTORY_SIZE; i++) history[i-1] = history[i];
        history_count--;
    }
    history[history_count++] = strdup(command);
}

int xsh_history(char **args) {
    for (int i = 0; i < history_count; i++) {
        printf("%3d  %s\n", i+1, history[i]);
    }
    return 1;
}

void cleanup_history(void) {
    for (int i = 0; i < history_count; i++) free(history[i]);
}

// Execute a command from history using !n notation
int execute_history_command(char *input) {
    if (input[0] == '!' && isdigit(input[1])) {
        int index = atoi(&input[1]) - 1;
        
        if (index >= 0 && index < history_count) {
            printf("Executing: %s\n", history[index]);
            
            // Split the historical command into args and execute it
            char **args = xsh_split_line(history[index]);
            int status = xsh_execute(args);
            
            free(args);
            return status;
        } else {
            fprintf(stderr, "xsh: !%d: event not found\n", index + 1);
            return 1;
        }
    }
    
    return -1; // Not a history command
}


char* build_prompt(void) {
    static char prompt[XSH_MAXLINE];
    char cwd[XSH_MAXLINE];
    char short_cwd[XSH_MAXLINE];
    char username[XSH_MAXLINE];
    
    // Get username
#ifdef _WIN32
    DWORD username_size = XSH_MAXLINE;
    if (!GetUserNameA(username, &username_size)) {
        strcpy(username, "user");
    }
#else
    char *user_env = getenv("USER");
    if (user_env) {
        strncpy(username, user_env, XSH_MAXLINE - 1);
        username[XSH_MAXLINE - 1] = '\0';
    } else {
        strcpy(username, "user");
    }
#endif
    
    // Get current directory
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }
#else
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }
#endif
    
    // Extract just the current folder name for a cleaner prompt
    char *last_slash = strrchr(cwd, '\\');
    if (last_slash) {
        strcpy(short_cwd, last_slash + 1);
    } else {
        strcpy(short_cwd, cwd);
    }
    
    sprintf(prompt, "\x1b[1;36mxsh\x1b[0m@\x1b[1;35m%s\x1b[0m:\x1b[32m%s\x1b[0m:\x1b[33m%d\x1b[0m> ", 
            username, short_cwd, history_count+1);
    
    return prompt;
}


void xsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("%s", build_prompt());
        line = xsh_read_line();
        add_to_history(line);
        args = xsh_split_line(line);

        if (execute_history_command(line) == -1) {
            status = xsh_execute(args);
        } else {
            status = 1; // History command handled execution
        }

        free(line);
        free(args);
    } while (status);
}

void sigint_handler(int sig_num){
    // Handle Ctrl+C (SIGINT) to prevent the shell from exiting
    signal(SIGINT, sigint_handler);
    printf("\nCaught signal %d. Use 'exit' to quit the shell.\n", sig_num);

    printf("\n%s", build_prompt());
    fflush(stdout);
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_handler);
    xsh_banner();

    // Start the shell loop
    xsh_loop();
    cleanup_history();
    return EXIT_SUCCESS;
}

