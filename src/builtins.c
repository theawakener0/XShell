#include "builtins.h"
#include "network.h" // For xsh_client, though it's declared in builtins.h for now
#include "history.h" // For history array and count (declared extern there)
#include "utils.h" // For utility functions like print_slow, build_prompt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For chdir, getcwd (POSIX)
#include <dirent.h> // For opendir, readdir, closedir (used indirectly by ls or completion)

#ifdef _WIN32
#include <direct.h> // For _mkdir, _getcwd
#include <io.h>     // For _access (if needed for file checks)
#else
#include <sys/stat.h> // For mkdir (POSIX)
#include <fcntl.h>    // For open (used in touch POSIX)
#endif

// Built-in command names
char *builtin_str[] = {
    "cd", "pwd", "ls", "grep", "echo", "mkdir", "touch", "cp", "mv",
    "rm", "cat", "xmanifesto", "xeno", "history", "help", "clear", "exit"
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
    "Connect you to the Gatekeeper (network client)",
    "Show command history",
    "Display help information about available commands",
    "Clear the terminal screen",
    "Exit the shell program"
};

// Array of function pointers for built-in commands
int (*builtin_func[])(char **) = {
    &xsh_cd, &xsh_pwd, &xsh_ls, &xsh_grep, &xsh_echo, &xsh_mkdir, &xsh_touch,
    &xsh_cp, &xsh_mv, &xsh_rm, &xsh_cat, &xsh_manifesto, &xsh_client, // xsh_client will move
    &xsh_history, &xsh_help, &xsh_clear, &xsh_exit
};

int xsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
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
    // This is a simplistic version. A more robust ls would use readdir etc.
    // and handle arguments for paths, flags etc.
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
            continue; 
        }
        char line[XSH_MAXLINE];
        while (fgets(line, sizeof(line), fp)) {
            fputs(line, stdout);
        }
        if (ferror(fp)) {
            fprintf(stderr, "xsh: cat: error reading file '%s'\n", args[i]);
        }
        if (fclose(fp) == EOF) {
            fprintf(stderr, "xsh: cat: error closing '%s': ", args[i]);
            perror("");
        }
    }
    return 1;
}

int xsh_grep(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "xsh: grep: missing pattern\n");
        fprintf(stderr, "Usage: grep <pattern> [file]\n");
        return 1;
    }
    char *pattern = args[1];
    char *search_pattern = pattern;
    FILE *fp;
    char *filename = NULL;

    size_t pattern_len = strlen(pattern);
    if (pattern_len >= 2 && pattern[0] == '"' && pattern[pattern_len - 1] == '"') {
        search_pattern = malloc(pattern_len - 1);
        if (!search_pattern) {
            fprintf(stderr, "xsh: grep: allocation error\n");
            return 1;
        }
        strncpy(search_pattern, pattern + 1, pattern_len - 2);
        search_pattern[pattern_len - 2] = '\0';
    } 

    if (args[2] != NULL) {
        filename = args[2];
        fp = fopen(filename, "r");
        if (!fp) {
            fprintf(stderr, "xsh: grep: cannot open file '%s': ", filename);
            perror("");
            if (search_pattern != pattern) free(search_pattern);
            return 1;
        }
    } else {
        // Reading from stdin is not implemented here for simplicity, 
        // but could be added by setting fp = stdin;
        fprintf(stderr, "xsh: grep: reading from stdin not implemented. Please specify a file.\n");
        fprintf(stderr, "Usage: grep <pattern> [file]\n");
        if (search_pattern != pattern) free(search_pattern);
        return 1;
    }

    char line[XSH_MAXLINE];
    while (fgets(line, XSH_MAXLINE, fp)) {
        if (strstr(line, search_pattern)) {
            fputs(line, stdout);
        }
    }

    if (fp != stdin && filename != NULL) {
        if (fclose(fp) == EOF) {
            perror("xsh: grep: fclose");
        }
    }
    if (search_pattern != pattern) {
        free(search_pattern);
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
    printf("XShell - A simple C Shell by Xenomench\n");
    printf("Type command names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < xsh_num_builtins(); i++) {
        printf("  - %s    %s\n", builtin_str[i], builtin_desc[i]);
    }

    return 1;
}

int xsh_exit(char **args) {
    return 0; // Signal to terminate the shell loop
}

