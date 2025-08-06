#ifdef _WIN32
#include <winsock2.h> // For Windows socket functions
#include <direct.h> // For _mkdir, _getcwd
#include <io.h>     // For _access (if needed for file checks)
#else
#include <sys/stat.h> // For mkdir (POSIX)
#include <fcntl.h>    // For open (used in touch POSIX)
#endif

#include "builtins.h"
#include "network.h" // For xsh_client, though it's declared in builtins.h for now
#include "history.h" // For history array and count (declared extern there)
#include "utils.h" // For utility functions like print_slow, build_prompt
#include "xproj.h" // For xsh_xproj (project creation command)
#include "xnote.h" // For xsh_xnote (encrypted note keeper command)
#include "xpass.h" // For xsh_xpass (password strength analyzer and generator)
#include "xnet.h"
#include "xscan.h"
#include "xcodex.h" // For xsh_xcodex (text editor command, POSIX only)
#include "xcrypt.h" // For xsh_xcrypt (file encryption/decryption tool)
#include "config.h" // For configuration management
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For chdir, getcwd (POSIX)
#include <dirent.h> // For opendir, readdir, closedir (used indirectly by ls or completion)
#include <ctype.h>  // For tolower (needed for case-insensitive grep)





// Built-in command names
char *builtin_str[] = {
    "cd", "pwd", "ls", "grep", "echo", "mkdir", "touch", "cp", "mv",
    "rm", "cat", "xmanifesto", "xproj", "xnote", "xpass", "xeno", "xnet", "xscan", "xcodex", "xcrypt", "config", "history", "stats", "analytics", "cleardata", "help", "clear", "exit"
};

// Descriptions for built-in commands (for help)
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
    "Create a new project structure (C, Python, or Web)",
    "Encrypted Note Keeper",
    "Password Strength Analyzer and Generator",
    "Connect you to the Gatekeeper (network client)",
    "Minimal Network Diagnostic Tools",
    "Custom Port Scanner",
    "XCodex (Xenomench Codex) - A built-in code editor",
    "Simple file encryption/decryption tool",
    "Easy-to-use configuration management for XShell and XCodex",
    "Show command history",
    "Show command statistics and analytics",
    "Display comprehensive performance analytics",
    "Clear all analytics and learning data",
    "Display help information about available commands",
    "Clear the terminal screen",
    "Exit the shell program"
};

// Usage strings for built-in commands
char *builtin_usage[] = {
    "Usage: cd <directory>",
    "Usage: pwd",
    "Usage: ls [path]",
    "Usage: grep [-i] <pattern> [file...]",
    "Usage: echo [string ...]",
    "Usage: mkdir <directory_name> [directory_name2] ...",
    "Usage: touch <file_name> [file_name2] ...",
    "Usage: cp <source_file> <destination_file>",
    "Usage: mv <source_file> <destination_file>",
    "Usage: rm <name1> [name2] ...",
    "Usage: cat <file_name> [file_name2] ...",
    "Usage: xmanifesto",
    "Usage: xproj <project_type (c, py, web)> <project_name> [--git]",
    "Usage: xnote <command> [options]\nCommands:\n  add <name> \"<content>\" - Create a new note.\n  view <name>             - View a decrypted note.\n  lock <name>             - Encrypt a note with a password.\n  list                    - List all available notes.\n  delete <name>           - Delete a note.",
    "Usage: xpass <command> [options]\nCommands:\n  gen [length] [--no-upper] [--no-lower] [--no-digits] [--no-symbols] - Generate a password.\n  check <password> - Check the strength of a password.",
    "Usage: xeno [hostname] [port]  (Note: Actual arguments depend on client implementation)",
    "Usage: xnet [show|ping|traceroute] [host]",
    "Usage: xscan <target IP> <start port> [end port]",
    "Usage: xcodex <file_name>",
    "Usage: xcrypt <encrypt|decrypt> <input_file> <output_file>\nNote: 'encrypt' and 'decrypt' use the same symmetric XOR operation.",
    "Usage: config [command] [options]\nPopular commands:\n  show                 - Display current settings\n  get <setting>        - Get a setting value\n  set <setting> <val>  - Change a setting\n  save                 - Save changes to file\n  help                 - Show detailed help",
    "Usage: history",
    "Usage: stats [command_name]",
    "Usage: analytics",
    "Usage: cleardata",
    "Usage: help [command]",
    "Usage: clear",
    "Usage: exit"
};

// Array of function pointers for built-in commands
int (*builtin_func[])(char **) = {
    &xsh_cd, &xsh_pwd, &xsh_ls, &xsh_grep, &xsh_echo, &xsh_mkdir, &xsh_touch,
    &xsh_cp, &xsh_mv, &xsh_rm, &xsh_cat, &xsh_manifesto, &xsh_xproj, &xsh_xnote,
    &xsh_xpass, &xsh_client, &xsh_xnet, &xsh_xscan, &xsh_xcodex, &xsh_xcrypt,
    &xsh_config, &xsh_history, &xsh_stats, &xsh_analytics, &xsh_cleardata, &xsh_help, &xsh_clear, &xsh_exit
};

int xsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Check if a command is a built-in
int xsh_builtin_exists(const char *command) {
    if (!command) return 0;
    
    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strcmp(command, builtin_str[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Execute a built-in command
int xsh_execute_builtin(char **args) {
    if (!args || !args[0]) return 1;
    
    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return 1; // Command not found
}

// Implementations of built-in functions

int xsh_cd(char **args) { 
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("xsh: chdir failed");
        }
    }
    return 1;
}

int xsh_pwd(char **args) {
    char cwd[XSH_MAXLINE]; // Use defined constant
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
#else
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
#endif
        perror("xsh: pwd failed");
    } else {
        printf("%s\n", cwd);
    }
    return 1;
}

int xsh_ls(char **args) {
    const char *path_to_list = "."; // Default to current directory

    if (args[1] != NULL) {
        // If a path is provided, use it.
        path_to_list = args[1];
    }

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char searchPath[XSH_MAXLINE];

    // Prepare search path (e.g., "directory\*")
    strncpy(searchPath, path_to_list, XSH_MAXLINE - 1);
    searchPath[XSH_MAXLINE - 1] = '\0'; // Ensure null termination

    size_t current_len = strlen(searchPath);
    if (current_len > 0 && (searchPath[current_len - 1] == '\\' || searchPath[current_len - 1] == '/')) {
        if (current_len + 1 < XSH_MAXLINE) {
            strcat(searchPath, "*");
        } else {
            fprintf(stderr, "xsh: ls: path too long to append wildcard: %s\n", path_to_list);
            return 1;
        }
    } else {
        if (current_len + 2 < XSH_MAXLINE) { // Check space for '\*' and null terminator
            strcat(searchPath, "\\*");
        } else {
            fprintf(stderr, "xsh: ls: path too long to append wildcard: %s\n", path_to_list);
            return 1;
        }
    }

    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_FILE_NOT_FOUND) {
            fprintf(stderr, "xsh: ls: cannot access '%s': ", path_to_list);
            LPVOID lpMsgBuf;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf, 0, NULL );
            fprintf(stderr, "%s\n", (char*)lpMsgBuf);
            LocalFree(lpMsgBuf);
        }
        // If ERROR_FILE_NOT_FOUND, it could be an empty or non-existent directory.
        // ls typically prints nothing for an empty directory and returns success.
        return 1;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }
        printf("%s\n", findFileData.cFileName);
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

#else // POSIX implementation
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path_to_list);
    if (dir == NULL) {
        fprintf(stderr, "xsh: ls: cannot access '%s': ", path_to_list);
        perror("");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("%s\n", entry->d_name);
    }

    if (closedir(dir) == -1) {
        perror("xsh: ls: closedir failed");
    }
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

    FILE *src_file, *dest_file;
    int ch;

    src_file = fopen(args[1], "rb");
    if (src_file == NULL) {
        fprintf(stderr, "xsh: cp: cannot open source file '%s': ", args[1]);
        perror("");
        return 1;
    }

    dest_file = fopen(args[2], "wb");
    if (dest_file == NULL) {
        fprintf(stderr, "xsh: cp: cannot open destination file '%s': ", args[2]);
        perror("");
        fclose(src_file);
        return 1;
    }

    while ((ch = fgetc(src_file)) != EOF) {
        fputc(ch, dest_file);
    }

    if (ferror(src_file)) {
        fprintf(stderr, "xsh: cp: error reading source file '%s'\n", args[1]);
    }
    if (ferror(dest_file)) {
        fprintf(stderr, "xsh: cp: error writing to destination file '%s'\n", args[2]);
    }

    if (fclose(src_file) == EOF) {
        fprintf(stderr, "xsh: cp: error closing source file '%s': ", args[1]);
        perror("");
    }
    if (fclose(dest_file) == EOF) {
        fprintf(stderr, "xsh: cp: error closing destination file '%s': ", args[2]);
        perror("");
    } else {
        printf("Copied '%s' to '%s'\n", args[1], args[2]);
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
        // rename might fail across different filesystems. 
        // A more robust mv would copy then delete.
        return 1;
    } else {
        printf("Moved '%s' to '%s'\n", args[1], args[2]);
    }
    return 1;
}

int xsh_rm(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: rm: missing file or directory operand\n");
        fprintf(stderr, "Usage: rm <name1> [name2] ...\n");
        return 1;
    }
    for (int i = 1; args[i] != NULL; i++) {
        // remove_recursively_internal is now defined in utils.c and declared in utils.h
        if (remove_recursively_internal(args[i]) != 0) {
            // Error messages are printed by remove_recursively_internal
            // Standard rm typically continues with other arguments even if one fails.
            // We might want to return a different status or set a flag if any removal fails,
            // but for now, it prints errors and continues.
        } else {
            printf("Removed '%s'\n", args[i]);
        }
    }
    return 1; // Continue shell loop
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
            continue; 
        }
        char line[XSH_MAXLINE];
        while (fgets(line, sizeof(line), fp)) {
            fputs(line, stdout);
        }
        printf("\n");
        if (ferror(fp)) {
            fprintf(stderr, "xsh: cat: error reading file '%s'\n", args[i]);
        }
        if (fclose(fp) == EOF) {
            fprintf(stderr, "xsh: cat: error closing '%s': \n", args[i]);
            perror("");
        }
    }
    return 1;
}


int xsh_grep(char **args) {
    int case_insensitive = 0;
    char *pattern_arg = NULL;
    int first_file_arg_index = -1;

    if (args[1] == NULL) { // No arguments after "grep"
        fprintf(stderr, "xsh: grep: missing pattern\nUsage: grep [-i] <pattern> [file...]\n");
        return 1;
    }

    int current_arg_idx = 1;
    // Parse options
    while (args[current_arg_idx] != NULL && args[current_arg_idx][0] == '-' && strlen(args[current_arg_idx]) > 1) {
        if (strcmp(args[current_arg_idx], "-i") == 0) {
            case_insensitive = 1;
            current_arg_idx++;
        } else {
            fprintf(stderr, "xsh: grep: unknown option %s\nUsage: grep [-i] <pattern> [file...]\n", args[current_arg_idx]);
            return 1;
        }
    }

    // Next argument is the pattern
    if (args[current_arg_idx] == NULL) {
        fprintf(stderr, "xsh: grep: missing pattern after options\nUsage: grep [-i] <pattern> [file...]\n");
        return 1;
    }
    pattern_arg = args[current_arg_idx];
    current_arg_idx++; // Move to where filenames would start
    first_file_arg_index = current_arg_idx;

    char *actual_pattern = pattern_arg;
    char *allocated_quoted_pattern_buffer = NULL;
    size_t pattern_len = strlen(pattern_arg);

    if (pattern_len >= 2 && pattern_arg[0] == '"' && pattern_arg[pattern_len - 1] == '"') {
        allocated_quoted_pattern_buffer = malloc(pattern_len - 1);
        if (!allocated_quoted_pattern_buffer) {
            fprintf(stderr, "xsh: grep: memory allocation error for pattern\n");
            return 1;
        }
        strncpy(allocated_quoted_pattern_buffer, pattern_arg + 1, pattern_len - 2);
        allocated_quoted_pattern_buffer[pattern_len - 2] = '\0';
        actual_pattern = allocated_quoted_pattern_buffer;
    }

    int num_file_args = 0;
    for (int i = first_file_arg_index; args[i] != NULL; i++) {
        num_file_args++;
    }
    int print_filenames_flag = (num_file_args > 1);

    if (args[first_file_arg_index] == NULL) { // No file arguments, read from stdin
        process_grep_stream(stdin, actual_pattern, case_insensitive, NULL); // No filename to print for stdin
    } else { // Process one or more files
        for (int i = first_file_arg_index; args[i] != NULL; i++) {
            FILE *fp = fopen(args[i], "r");
            if (fp == NULL) {
                fprintf(stderr, "xsh: grep: %s: \n", args[i]);
                perror("");
                continue; // Standard grep continues with other files
            }
            process_grep_stream(fp, actual_pattern, case_insensitive, print_filenames_flag ? args[i] : NULL);
            if (fclose(fp) == EOF) {
                fprintf(stderr, "xsh: grep: error closing %s: \n", args[i]);
                perror("");
            }
        }
    }

    if (allocated_quoted_pattern_buffer) {
        free(allocated_quoted_pattern_buffer);
    }
    return 1;
}

int xsh_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s%s", args[i], (args[i+1] != NULL ? " " : ""));
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
    for (int i = 1; args[i] != NULL; i++) {
#ifdef _WIN32
        if (_mkdir(args[i]) != 0) {
#else
        if (mkdir(args[i], 0755) != 0) { // 0755 are typical permissions
#endif
            fprintf(stderr, "xsh: mkdir: cannot create directory '%s': ", args[i]);
            perror("");
        } else {
            printf("Created directory '%s'\n", args[i]);
        }
    }
    return 1;
}

int xsh_touch(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: touch: missing file operand\n");
        fprintf(stderr, "Usage: touch <file_name> [file_name2] ...\n");
        return 1;
    }
    for (int i = 1; args[i] != NULL; i++) {
#ifdef _WIN32
        FILE *fp = fopen(args[i], "ab"); // Open in append binary mode to create if not exists, or update timestamp
        if (fp == NULL) {
            // If opening failed, try to create it
            fp = fopen(args[i], "wb");
            if (fp == NULL) {
                fprintf(stderr, "xsh: touch: cannot touch '%s': ", args[i]);
                perror("");
                continue;
            }
        }
        printf("Touched file '%s'\n", args[i]);
        fclose(fp);
#else
        // POSIX: O_CREAT creates if not exists. O_APPEND updates timestamp.
        // utime() or utimensat() would be more precise for just updating timestamps.
        int fd = open(args[i], O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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

int xsh_xnet(char **args) {
    int argc = 0;
    while(args[argc] != NULL) {
        argc++;
    }
    xnet_main(argc, args);
    return 1;
}

int xsh_xscan(char **args) {
    int argc = 0;
    while(args[argc] != NULL) {
        argc++;
    }
    xscan_main(argc, args);
    return 1;
}

int xsh_xcodex(char **args) {
    int argc = 0;
    while(args[argc] != NULL) {
        argc++;
    }
#if defined(XCODEX_ENABLED)
    return xcodex_main(argc, args);
#else
    return XcodexMain(argc, args);
#endif
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
    unsigned int char_delay_ms = 15;    // Delay per character in milliseconds
    unsigned int line_delay_ms = 300;   // Delay between lines in milliseconds

    for (int i = 0; i < num_lines; ++i) {
        print_slow(lines[i], char_delay_ms); // print_slow now expects milliseconds
        
        // Pause between lines
#ifdef _WIN32
        Sleep(line_delay_ms);
#else
        usleep(line_delay_ms * 1000); // usleep still needs microseconds
#endif
    }

    printf("\n");

    return 1;
}

// xsh_client is defined in network.c
// xsh_history will be more fleshed out in history.c
int xsh_history(char **args) {
    if (history_count == 0) {
        printf("No commands in history.\n");
        return 1;
    }
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
    return 1;
}

int xsh_help(char **args) {
    if (args[1] == NULL) {
        printf("XShell - A simple C Shell by Xenomench\n");
        printf("Type command names and arguments, and hit enter.\n");
        printf("The following are built in:\n");

        for (int i = 0; i < xsh_num_builtins(); i++) {
            printf("  - %s    %s\n", builtin_str[i], builtin_desc[i]);
        }

        printf("\nFor more information on a specific command, type 'help <command>'.\n");
    } else {
        // Display help for a specific command
        for (int i = 0; i < xsh_num_builtins(); i++) {
            if (strcmp(args[1], builtin_str[i]) == 0) {
                printf("%s: %s\n", builtin_str[i], builtin_desc[i]);
                printf("%s\n", builtin_usage[i]);
                return 1;
            }
        }
        fprintf(stderr, "xsh: help: no help topics match '%s'.\n", args[1]);
        fprintf(stderr, "Run 'help' to see a list of available commands.\n");
    }

    return 1;
}

int xsh_exit(char **args) {
    printf("Exiting XShell. Goodbye!\n");
    return 0; // Signal to terminate the shell loop
}

// Configuration management command
int xsh_config(char **args) {
    if (!args[1]) {
        printf("\n\x1b[1;36m🔧 XShell Configuration Manager\x1b[0m\n");
        printf("═══════════════════════════════════\n\n");
        
        printf("\x1b[1;32m📋 Basic Commands:\x1b[0m\n");
        printf("  config show                    - Show current settings\n");
        printf("  config get <key>               - Get a setting value\n");
        printf("  config set <key> <value>       - Change a setting\n");
        printf("  config save                    - Save changes to file\n\n");
        
        printf("\x1b[1;33m� Examples:\x1b[0m\n");
        printf("  config set prompt_style simple\n");
        printf("  config set history_size 50\n");
        printf("  config get theme\n");
        printf("  config show\n\n");
        
        printf("\x1b[1;37m💡 Tip:\x1b[0m Add 'xcodex' to work with XCodex settings\n");
        printf("       Example: config show xcodex\n\n");
        return 1;
    }
    
    char *command = args[1];
    config_t *config = &xshell_config; // Default to xshell
    char *config_name = "xshell";
    
    // Check if last argument is xcodex
    int argc = 0;
    while (args[argc]) argc++; // Count arguments
    
    if (argc > 2 && strcmp(args[argc-1], "xcodex") == 0) {
        config = &xcodex_config;
        config_name = "xcodex";
    }
    
    // Show command
    if (strcmp(command, "show") == 0 || strcmp(command, "list") == 0) {
        printf("\n\x1b[1;36m⚙️  %s Configuration\x1b[0m\n", 
                strcmp(config_name, "xshell") == 0 ? "XShell" : "XCodex");
        printf("═══════════════════════════════════\n");
        
        if (config->count == 0) {
            printf("\x1b[1;33m⚠️  No configuration found.\x1b[0m\n\n");
            return 1;
        }
        
        for (int i = 0; i < config->count; i++) {
            printf("  %-20s = %s\n", config->pairs[i].key, config->pairs[i].value);
        }
        printf("\n");
        return 1;
    }
    
    // Get command
    if (strcmp(command, "get") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Usage: config get <key>\x1b[0m\n\n");
            return 1;
        }
        
        const char *value = config_get(config, args[2]);
        if (value) {
            printf("%s = %s\n", args[2], value);
        } else {
            printf("\x1b[1;33m⚠️  Key '%s' not found in %s configuration\x1b[0m\n", args[2], config_name);
        }
        return 1;
    }
    
    // Set command
    if (strcmp(command, "set") == 0) {
        if (!args[2] || !args[3]) {
            printf("\x1b[1;31m❌ Usage: config set <key> <value>\x1b[0m\n\n");
            return 1;
        }
        
        if (config_set(config, args[2], args[3]) == 0) {
            printf("\x1b[1;32m✅ Set %s = %s\x1b[0m\n", args[2], args[3]);
            
            // Special handling for XCodex theme changes with validation
            if (strcmp(config_name, "xcodex") == 0 && strcmp(args[2], "theme") == 0) {
                const char *valid_themes[] = {
                    "xcodex_dark", "xcodex_light", "gruvbox_dark", 
                    "tokyo_night_dark", "tokyo_night_light", "tokyo_night_storm"
                };
                int theme_valid = 0;
                for (int i = 0; i < 6; i++) {
                    if (strcmp(args[3], valid_themes[i]) == 0) {
                        theme_valid = 1;
                        break;
                    }
                }
                
                if (theme_valid) {
                    printf("\x1b[1;32m✅ XCodex theme '%s' applied successfully\x1b[0m\n", args[3]);
                    printf("💡 Start XCodex to see the new theme in action\n");
                } else {
                    printf("\x1b[1;33m⚠️  Theme '%s' not found. Available XCodex themes:\x1b[0m\n", args[3]);
                    for (int i = 0; i < 6; i++) {
                        printf("   - %s\n", valid_themes[i]);
                    }
                }
            }
            // Special handling for XShell theme changes - apply immediately with better preview
            else if (strcmp(config_name, "xshell") == 0 && strcmp(args[2], "theme") == 0) {
                printf("\x1b[1;34m🎨 Applying XShell theme '%s'...\x1b[0m\n", args[3]);
                // Show immediate preview of new theme
                char *preview_prompt = build_prompt();
                printf("New prompt preview: %s\x1b[0m[PREVIEW]\n", preview_prompt);
                printf("\x1b[1;32m✅ XShell theme applied! Changes take effect immediately.\x1b[0m\n");
            }
            // Special handling for prompt changes
            else if (strcmp(config_name, "xshell") == 0 && 
                    (strcmp(args[2], "prompt") == 0 || strcmp(args[2], "prompt_style") == 0 || strcmp(args[2], "color_output") == 0)) {
                printf("\x1b[1;34m📋 Updating prompt...\x1b[0m\n");
                char *preview_prompt = build_prompt();
                printf("New prompt preview: %s\x1b[0m[RESET]\n", preview_prompt);
            }
            
            printf("\x1b[2m💡 Use 'config save' to make this change permanent\x1b[0m\n\n");
        } else {
            printf("\x1b[1;31m❌ Failed to set '%s' to '%s'\x1b[0m\n", args[2], args[3]);
            printf("💡 Check if the value is valid for this setting type\n\n");
        }
        return 1;
    }
    
    // Save command
    if (strcmp(command, "save") == 0) {
        char home_path[512];
        const char *home = getenv("HOME");
        if (!home) {
#ifdef _WIN32
            home = getenv("USERPROFILE");
#endif
        }
        
        if (!home) {
            printf("\x1b[1;31m❌ Could not determine home directory\x1b[0m\n");
            return 1;
        }
        
        const char *filename = strcmp(config_name, "xshell") == 0 ? XSHELL_CONFIG_FILE : XCODEX_CONFIG_FILE;
        
#ifdef _WIN32
        snprintf(home_path, sizeof(home_path), "%s\\%s", home, filename);
#else
        snprintf(home_path, sizeof(home_path), "%s/%s", home, filename);
#endif
        
        if (config_save_file(config, home_path) == 0) {
            printf("\x1b[1;32m✅ Saved %s configuration to %s\x1b[0m\n", config_name, home_path);
        } else {
            printf("\x1b[1;31m❌ Failed to save configuration\x1b[0m\n");
        }
        return 1;
    }
    else if (strcmp(command, "reset") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Missing setting name\x1b[0m\n");
            printf("Usage: config reset <setting_name> [xshell|xcodex]\n");
            printf("Example: config reset theme\n\n");
            return 1;
        }
        
        if (config_remove(config, args[2]) == 0) {
            printf("\x1b[1;32m♻️  Reset '%s' to default value\x1b[0m\n\n", args[2]);
        } else {
            printf("\x1b[1;31m❌ Setting '%s' not found\x1b[0m\n\n", args[2]);
        }
        return 1;
    }
    else if (strcmp(command, "backup") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Missing backup filename\x1b[0m\n");
            printf("Usage: config backup <filename> [xshell|xcodex]\n");
            printf("Example: config backup my_backup.ini\n\n");
            return 1;
        }
        printf("\x1b[1;34m💾 Creating backup...\x1b[0m\n");
        config_backup(config, args[2]);
        printf("\n");
    }
    else if (strcmp(command, "restore") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Missing backup filename\x1b[0m\n");
            printf("Usage: config restore <filename> [xshell|xcodex]\n");
            printf("Example: config restore my_backup.ini\n\n");
            return 1;
        }
        printf("\x1b[1;34m📥 Restoring from backup...\x1b[0m\n");
        config_restore(config, args[2]);
        printf("\n");
    }
    else if (strcmp(command, "export") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Missing export filename\x1b[0m\n");
            printf("Usage: config export <filename> [xshell|xcodex]\n");
            printf("Example: config export my_settings.ini\n\n");
            return 1;
        }
        
        printf("\x1b[1;34m📤 Exporting configuration...\x1b[0m\n");
        if (config_export_ini(config, args[2]) == 0) {
            printf("\x1b[1;32m✅ Exported %s configuration to %s\x1b[0m\n\n", config_name, args[2]);
        } else {
            printf("\x1b[1;31m❌ Failed to export configuration\x1b[0m\n\n");
        }
    }
    else if (strcmp(command, "options") == 0 || strcmp(command, "keys") == 0) {
        printf("\n\x1b[1;36m⚙️  Available %s Settings\x1b[0m\n", 
               strcmp(config_name, "xshell") == 0 ? "XShell" : "XCodex");
        printf("═══════════════════════════════════════\n\n");
        config_list_available_keys(config_name);
        printf("\n");
    }
    else if (strcmp(command, "status") == 0 || strcmp(command, "validate") == 0) {
        printf("\n\x1b[1;36m🔍 Configuration Health Check\x1b[0m\n");
        printf("════════════════════════════════════\n");
        printf("Checking %s configuration...\n\n", config_name);
        
        int errors = 0;
        int warnings = 0;
        
        for (int i = 0; i < config->count; i++) {
            if (!config_validate_value(config->pairs[i].value, config->pairs[i].type)) {
                printf("\x1b[1;31m❌ ERROR:\x1b[0m %s = '%s' (invalid type)\n", 
                       config->pairs[i].key, config->pairs[i].value);
                errors++;
            }
        }
        
        if (config->count == 0) {
            printf("\x1b[1;33m⚠️  WARNING:\x1b[0m No configuration found\n");
            warnings++;
        }
        
        printf("\n\x1b[1;36m📊 Summary:\x1b[0m\n");
        printf("Settings: %d\n", config->count);
        printf("Errors: %s%d\x1b[0m\n", errors > 0 ? "\x1b[1;31m" : "\x1b[1;32m", errors);
        printf("Warnings: %s%d\x1b[0m\n", warnings > 0 ? "\x1b[1;33m" : "\x1b[1;32m", warnings);
        
        if (errors == 0 && warnings == 0) {
            printf("\n\x1b[1;32m✅ Configuration is healthy!\x1b[0m\n");
        } else if (errors > 0) {
            printf("\n\x1b[1;31m❌ Configuration has errors that need fixing\x1b[0m\n");
        } else {
            printf("\n\x1b[1;33m⚠️  Configuration has warnings\x1b[0m\n");
        }
        printf("\n");
    }
    else if (strcmp(command, "init") == 0) {
        printf("\x1b[1;34m🚀 Creating default configuration files...\x1b[0m\n");
        if (config_create_default_files() == 0) {
            printf("\x1b[1;32m✅ Default configuration files created\x1b[0m\n");
            printf("💡 Use 'config show' to see your new settings\n\n");
        } else {
            printf("\x1b[1;31m❌ Failed to create configuration files\x1b[0m\n\n");
        }
    }
    else if (strcmp(command, "help") == 0) {
        printf("\n\x1b[1;36m📖 XShell Configuration Help\x1b[0m\n");
        printf("═══════════════════════════════════\n\n");
        config_show_help(config_name);
    }
    else if (strcmp(command, "test") == 0) {
        printf("\x1b[1;34m🧪 Configuration Test\x1b[0m\n");
        printf("════════════════════════\n");
        
        printf("Testing XShell configuration:\n");
        printf("  prompt = '%s'\n", config_get_default(&xshell_config, "prompt", "not found"));
        printf("  prompt_style = '%s'\n", config_get_default(&xshell_config, "prompt_style", "not found"));
        printf("  color_output = %s\n", config_get_bool(&xshell_config, "color_output", 0) ? "true" : "false");
        printf("  theme = '%s'\n", config_get_default(&xshell_config, "theme", "not found"));
        
        // Test prompt preview with current settings
        printf("\nPrompt Preview:\n");
        char *test_prompt = build_prompt();
        printf("  Current prompt: %s\x1b[0m[RESET]\n", test_prompt);
        
        printf("\nTesting XCodex configuration:\n");
        printf("  theme = '%s'\n", config_get_default(&xcodex_config, "theme", "not found"));
        printf("  line_numbers = %s\n", config_get_bool(&xcodex_config, "line_numbers", 0) ? "true" : "false");
        printf("  syntax_highlighting = %s\n", config_get_bool(&xcodex_config, "syntax_highlighting", 0) ? "true" : "false");
        
        // Test available themes
        printf("\nAvailable XShell themes: default, gruvbox_dark, tokyo_night, light\n");
        printf("Available XCodex themes:\n");
        const char* xcodex_themes[] = {
            "xcodex_dark", "xcodex_light", "gruvbox_dark", 
            "tokyo_night_dark", "tokyo_night_light", "tokyo_night_storm"
        };
        for (int i = 0; i < 6; i++) {
            printf("  - %s%s\n", xcodex_themes[i], (i == 0) ? " (default)" : "");
        }
        
        // Test current colors
        printf("\nCurrent XShell color test:\n");
        printf("  Color output: %s\n", config_get_bool(&xshell_config, "color_output", 0) ? "enabled" : "disabled");
        printf("  Theme: %s\n", config_get_default(&xshell_config, "theme", "default"));
        printf("  Current prompt: %s\x1b[0m[RESET]\n", build_prompt());
        
        printf("\nColor test - XShell themes:\n");
        const char *old_theme = config_get_default(&xshell_config, "theme", "default");
        const char *test_themes[] = {"default", "gruvbox_dark", "tokyo_night", "light"};
        for (int i = 0; i < 4; i++) {
            config_set(&xshell_config, "theme", test_themes[i]);
            config_set(&xshell_config, "color_output", "true");
            char *test_prompt = build_prompt();
            printf("  %-15s: %s\x1b[0m\n", test_themes[i], test_prompt);
        }
        // Restore original theme
        config_set(&xshell_config, "theme", old_theme);
        
        printf("\n");
    }
    else if (strcmp(command, "preview") == 0) {
        if (!args[2]) {
            printf("\x1b[1;31m❌ Missing preview type\x1b[0m\n");
            printf("Usage: config preview <type> [xshell|xcodex]\n");
            printf("Types: themes, prompts, colors\n");
            printf("Examples:\n");
            printf("  config preview themes xshell\n");
            printf("  config preview prompts\n");
            printf("  config preview colors\n\n");
            return 1;
        }
        
        if (strcmp(args[2], "themes") == 0) {
            if (argc > 3 && strcmp(args[argc-1], "xshell") == 0) {
                printf("\n\x1b[1;36m🎨 XShell Theme Preview\x1b[0m\n");
                printf("═══════════════════════════\n");
                
                // Save current settings
                const char *current_theme = config_get_default(&xshell_config, "theme", "default");
                int current_color = config_get_bool(&xshell_config, "color_output", 1);
                
                const char *themes[] = {"default", "gruvbox_dark", "tokyo_night", "light"};
                for (int i = 0; i < 4; i++) {
                    config_set(&xshell_config, "theme", themes[i]);
                    config_set(&xshell_config, "color_output", "true");
                    
                    char *preview_prompt = build_prompt();
                    printf("%-15s: %s\x1b[0m[RESET]\n", themes[i], preview_prompt);
                }
                
                // Restore original settings
                config_set(&xshell_config, "theme", current_theme);
                config_set(&xshell_config, "color_output", current_color ? "true" : "false");
                printf("\n💡 Use 'config set theme <name>' to apply a theme\n\n");
            } else {
                printf("\n\x1b[1;36m🎨 XCodex Theme Preview\x1b[0m\n");
                printf("═══════════════════════════\n");
                printf("Available XCodex themes:\n");
                printf("  \x1b[48;5;0m\x1b[38;5;37m xcodex_dark     \x1b[0m - Dark theme with purple/blue accents\n");
                printf("  \x1b[48;5;15m\x1b[38;5;16m xcodex_light    \x1b[0m - Light theme for bright environments\n");
                printf("  \x1b[48;5;237m\x1b[38;5;223m gruvbox_dark    \x1b[0m - Popular retro-style dark theme\n");
                printf("  \x1b[48;5;0m\x1b[38;5;111m tokyo_night_dark\x1b[0m - Modern dark blue theme\n");
                printf("  \x1b[48;5;15m\x1b[38;5;24m tokyo_night_light\x1b[0m - Light variant of Tokyo Night\n");
                printf("  \x1b[48;5;235m\x1b[38;5;116m tokyo_night_storm\x1b[0m - Darker variant with storm colors\n");
                printf("\n💡 Use 'config set theme <name> xcodex' to apply a theme\n\n");
            }
        } else if (strcmp(args[2], "prompts") == 0) {
            printf("\n\x1b[1;36m📋 Prompt Style Preview\x1b[0m\n");
            printf("═══════════════════════════\n");
            
            // Save current settings
            const char *current_style = config_get_default(&xshell_config, "prompt_style", "enhanced");
            const char *current_prompt = config_get_default(&xshell_config, "prompt", "xsh@{user}:{cwd}:{history}> ");
            
            // Preview simple style
            config_set(&xshell_config, "prompt_style", "simple");
            char *simple_prompt = build_prompt();
            printf("simple  : %s\x1b[0m[RESET]\n", simple_prompt);
            
            // Preview enhanced style
            config_set(&xshell_config, "prompt_style", "enhanced");
            char *enhanced_prompt = build_prompt();
            printf("enhanced: %s\x1b[0m[RESET]\n", enhanced_prompt);
            
            // Preview custom styles
            config_set(&xshell_config, "prompt_style", "custom");
            config_set(&xshell_config, "prompt", "[{user}@{cwd}]$ ");
            char *custom1_prompt = build_prompt();
            printf("custom 1: %s\x1b[0m[RESET]\n", custom1_prompt);
            
            config_set(&xshell_config, "prompt", "{user}:{history}> ");
            char *custom2_prompt = build_prompt();
            printf("custom 2: %s\x1b[0m[RESET]\n", custom2_prompt);
            
            // Restore original settings
            config_set(&xshell_config, "prompt_style", current_style);
            config_set(&xshell_config, "prompt", current_prompt);
            printf("\n💡 Use 'config set prompt_style <style>' to change style\n");
            printf("💡 Use 'config set prompt \"<format>\"' for custom prompts\n\n");
        } else if (strcmp(args[2], "colors") == 0) {
            printf("\n\x1b[1;36m🌈 Color Preview\x1b[0m\n");
            printf("═══════════════════\n");
            printf("Color output ON : ");
            config_set(&xshell_config, "color_output", "true");
            char *color_on = build_prompt();
            printf("%s\x1b[0m[RESET]\n", color_on);
            
            printf("Color output OFF: ");
            config_set(&xshell_config, "color_output", "false");
            char *color_off = build_prompt();
            printf("%s[RESET]\n", color_off);
            
            // Restore color setting
            config_set(&xshell_config, "color_output", "true");
            printf("\n💡 Use 'config set color_output true/false' to toggle colors\n\n");
        } else {
            printf("\x1b[1;31m❌ Unknown preview type: %s\x1b[0m\n", args[2]);
            printf("Available types: themes, prompts, colors\n\n");
        }
    }
    else if (strcmp(command, "debug") == 0) {
        printf("\x1b[1;31m🔧 Configuration Debug Information\x1b[0m\n");
        printf("════════════════════════════════════════\n");
        
        printf("XShell Configuration State:\n");
        printf("  Initialized: %s\n", xshell_config.count > 0 ? "Yes" : "No");
        printf("  Settings count: %d\n", xshell_config.count);
        printf("  Config file: %s\n", XSHELL_CONFIG_FILE);
        
        printf("\nXCodex Configuration State:\n");
        printf("  Initialized: %s\n", xcodex_config.count > 0 ? "Yes" : "No");
        printf("  Settings count: %d\n", xcodex_config.count);
        printf("  Config file: %s\n", XCODEX_CONFIG_FILE);
        
        printf("\nPrompt Debug:\n");
        printf("  Style: %s\n", config_get_default(&xshell_config, "prompt_style", "not found"));
        printf("  Format: %s\n", config_get_default(&xshell_config, "prompt", "not found"));
        printf("  Color output: %s\n", config_get_bool(&xshell_config, "color_output", 0) ? "true" : "false");
        printf("  Theme: %s\n", config_get_default(&xshell_config, "theme", "not found"));
        
        printf("\nPrompt Generation Test:\n");
        char *test_prompt = build_prompt();
        printf("  Generated: %s\x1b[0m[RESET]\n", test_prompt);
        
        printf("\nXCodex Theme Debug:\n");
        printf("  Theme: %s\n", config_get_default(&xcodex_config, "theme", "not found"));
        printf("  Line numbers: %s\n", config_get_bool(&xcodex_config, "line_numbers", 0) ? "true" : "false");
        printf("  Syntax highlighting: %s\n", config_get_bool(&xcodex_config, "syntax_highlighting", 0) ? "true" : "false");
        
        printf("\n\x1b[1;36m💡 Issues Found:\x1b[0m\n");
        if (xshell_config.count == 0) {
            printf("  ❌ XShell config not loaded - run 'config init'\n");
        }
        if (xcodex_config.count == 0) {
            printf("  ❌ XCodex config not loaded - run 'config init'\n");
        }
        if (!config_get_bool(&xshell_config, "color_output", 1)) {
            printf("  ⚠️  XShell colors disabled - run 'config set color_output true'\n");
        }
        printf("\n");
    }
    else if (strcmp(command, "reload") == 0) {
        printf("\x1b[1;34m🔄 Reloading configuration files...\x1b[0m\n");
        config_free(&xshell_config);
        config_free(&xcodex_config);
        config_load_all_files();
        printf("\x1b[1;32m✅ Configuration reloaded from files\x1b[0m\n\n");
    }
    else if (strcmp(command, "defaults") == 0) {
        printf("\x1b[1;33m⚠️  This will reset ALL %s settings to defaults!\x1b[0m\n", config_name);
        printf("Are you sure? Type 'yes' to continue: ");
        
        char response[10];
        if (fgets(response, sizeof(response), stdin) != NULL) {
            // Remove newline
            response[strcspn(response, "\n")] = 0;
            
            if (strcmp(response, "yes") == 0) {
                if (strcmp(config_name, "xshell") == 0) {
                    config_free(&xshell_config);
                    config_init(&xshell_config);
                    config_load_defaults(&xshell_config, "xshell");
                } else {
                    config_free(&xcodex_config);
                    config_init(&xcodex_config);
                    config_load_defaults(&xcodex_config, "xcodex");
                }
                printf("\x1b[1;32m♻️  Reset %s configuration to defaults\x1b[0m\n", config_name);
                printf("💡 Use 'config save' to make this permanent\n\n");
            } else {
                printf("\x1b[1;33m❌ Reset cancelled\x1b[0m\n\n");
            }
        }
    }
    else {
        printf("\x1b[1;31m❌ Unknown command: %s\x1b[0m\n", command);
        printf("💡 Use 'config' without arguments to see available commands\n");
        printf("💡 Or use 'config help' for detailed information\n\n");
    }
    
    return 1;
}

// Enhanced history statistics command
int xsh_stats(char **args) {
    if (args[1] == NULL) {
        // Show general statistics
        display_performance_analytics();
    } else {
        // Show statistics for specific command
        display_command_stats(args[1]);
    }
    return 1;
}

// Performance analytics display command
int xsh_analytics(char **args) {
    if (args[1] == NULL) {
        display_performance_analytics();
        display_usage_trends();
        analyze_command_patterns();
    } else if (strcmp(args[1], "performance") == 0) {
        display_performance_analytics();
    } else if (strcmp(args[1], "trends") == 0) {
        display_usage_trends();
    } else if (strcmp(args[1], "patterns") == 0) {
        analyze_command_patterns();
    } else {
        printf("Usage: analytics [performance|trends|patterns]\n");
        printf("  performance - Show command frequency and performance stats\n");
        printf("  trends      - Show usage trends over time\n");
        printf("  patterns    - Show learned command patterns\n");
        printf("  (no args)   - Show all analytics\n");
    }
    return 1;
}

// Clear analytics data command
int xsh_cleardata(char **args) {
    printf("This will clear all command history and analytics data.\n");
    printf("Are you sure? Type 'yes' to continue: ");
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) != NULL) {
        // Remove newline
        response[strcspn(response, "\n")] = 0;
        
        if (strcmp(response, "yes") == 0) {
            // Clear history and analytics data
            cleanup_history_system();
            if (init_history_system() == 0) {
                printf("Analytics data cleared successfully.\n");
            } else {
                printf("Warning: Failed to reinitialize history system.\n");
            }
        } else {
            printf("Clear operation cancelled.\n");
        }
    }
    
    return 1;
}

