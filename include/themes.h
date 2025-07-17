#ifndef XCODEX_THEMES_H
#define XCODEX_THEMES_H

/* Theme definitions */
typedef struct {
    char *name;
    int colors[17];  /* Colors for each highlight type */
    int bg_color;
    int cursor_color;
    int status_bg;
    int status_fg;
    int line_number_color; /* Color for line numbers */
    int cursor_line_color; /* Color for current line highlighting */
} theme_t;

/* Neovim-inspired themes */
extern theme_t themes[];
extern const int NUM_THEMES;

#endif