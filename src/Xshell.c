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

void xsh_banner(void) {

    printf("\n");
    printf("========================================================================================\n");
    printf("               Welcome to XShell - A simple shell implementation in C!               \n");
    printf("                     Type 'help' for a list of built-in commands.                     \n");
    printf("========================================================================================\n");
    printf("\n");
}


// Main shell loop
void xsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        char *prompt = build_prompt(); // Get dynamic prompt
        printf("%s", prompt);
        line = xsh_read_line();
        add_to_history(line); // Add command to history
        args = xsh_split_line(line);
        status = xsh_execute(args); // xsh_execute will handle builtins or launch

        free(line);
        free(args);
    } while (status);
}

// Execute command: checks for built-ins first, then launches external command
int xsh_execute(char **args) {
    if (args[0] == NULL) {
        // An empty command was entered.
        return 1; // Continue shell loop
    }

    for (int i = 0; i < xsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return xsh_launch(args);
}

int main(int argc, char **argv) {
    // Load config files, if any.
    xsh_banner();

    // Run command loop.
    xsh_loop();

    // Perform any shutdown/cleanup.
    // Free history
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return EXIT_SUCCESS;
}

