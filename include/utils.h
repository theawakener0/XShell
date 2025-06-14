#ifndef UTILS_H
#define UTILS_H

#include "xsh.h" // For useconds_t (on POSIX) and other common defs

// Function prototype for utils.c
// Standardized to take delay in milliseconds
void print_slow(const char *text, unsigned int delay_ms);

char* build_prompt(void);

#endif // UTILS_H
