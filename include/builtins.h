#ifndef BUILTINS_H
#define BUILTINS_H

#include "xsh.h" // For common definitions like XSH_MAXLINE, types, and platform checks

// External declarations for built-in command arrays and utility function
// Definitions are in builtins.c
extern char *builtin_str[];
extern char *builtin_desc[];
extern int (*builtin_func[])(char **);
int xsh_num_builtins(void);

// Helper functions for built-in command execution
int xsh_builtin_exists(const char *command);
int xsh_execute_builtin(char **args);

// Prototypes for built-in command functions
int xsh_cd(char **args);
int xsh_help(char **args);
int xsh_exit(char **args);
int xsh_pwd(char **args);
int xsh_ls(char **args);
int xsh_grep(char **args);
int xsh_echo(char **args);
int xsh_mkdir(char **args);
int xsh_touch(char **args);
int xsh_client(char **args); // Note: Planned to move to a separate network module
int xsh_clear(char **args);
int xsh_cp(char **args);
int xsh_mv(char **args);
int xsh_rm(char **args);
int xsh_cat(char **args);
int xsh_manifesto(char **args);
int xsh_history(char **args); // Note: History management might be expanded into its own module
int xsh_stats(char **args); // Enhanced history analytics
int xsh_analytics(char **args); // Performance analytics display
int xsh_cleardata(char **args); // Clear analytics data
int xsh_xnet(char **args);
int xsh_xproj(char **args);
int xsh_xnote(char **args);
int xsh_xpass(char **args);
int xsh_xscan(char **args);
int xsh_xcrypt(char **args); // Simple file encryption/decryption tool
int xsh_xcodex(char **args); // Simple text editor for POSIX systems only
int xsh_config(char **args); // Configuration management command

#endif // BUILTINS_H
