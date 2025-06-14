#include "utils.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h> // For Sleep(), GetUserName()
#include <lmcons.h>  // For UNLEN
#else
#include <unistd.h>  // For gethostname(), getpwuid(), getuid(), usleep()
#include <sys/utsname.h> // For uname()
#include <pwd.h>     // For getpwuid()
#endif

// Implementation of utility functions

void print_slow(const char *text, unsigned int delay_ms) {
    for (const char *p = text; *p != '\0'; p++) {
        putchar(*p);
        fflush(stdout);
#ifdef _WIN32
        Sleep(delay_ms);
#else
        usleep(delay_ms * 1000); // Convert ms to microseconds for usleep
#endif
    }
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
