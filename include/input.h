#ifndef INPUT_H
#define INPUT_H

#include "xsh.h" // For common definitions

// Arrow key constants
#ifdef _WIN32
#define ARROW_UP 72
#define ARROW_DOWN 80
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define EXTENDED_KEY 224
#else
#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'
#define ESC_SEQUENCE 27
#endif

// Function prototypes for input.c
char *xsh_read_line(void);
char **xsh_split_line(char *line);
char** find_matches(const char* partial, int* match_count);
void display_matches(char** matches, int match_count);
char* complete_command(const char* partial);

#endif // INPUT_H
