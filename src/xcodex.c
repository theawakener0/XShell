// This file is part of the XShell project, a simple terminal editor based on kilo

#define XCODEX_VERSION "0.2.1"

/* ============================ XCodex Modal System ============================ */

/* Editor Modes - Simple and Intuitive */
#define XCODEX_MODE_NORMAL     0  /* Navigate and execute commands */
#define XCODEX_MODE_INSERT     1  /* Insert text */
#define XCODEX_MODE_VISUAL     2  /* Select text */
#define XCODEX_MODE_VISUAL_LINE 3  /* Select lines */
#define XCODEX_MODE_VISUAL_BLOCK 4  /* Select blocks */
#define XCODEX_MODE_COMMAND    5  /* Execute commands */

/* Motion Types for consistent movement */
#define XCODEX_MOTION_CHAR     0  /* Character-wise movement */
#define XCODEX_MOTION_WORD     1  /* Word-wise movement */
#define XCODEX_MOTION_LINE     2  /* Line-wise movement */
#define XCODEX_MOTION_SCREEN   3  /* Screen-wise movement */

/* Key Constants */
#define XCODEX_KEY_ESC         27
#define XCODEX_KEY_CTRL_C      3
#define XCODEX_KEY_CTRL_D      4
#define XCODEX_KEY_CTRL_U      21
#define XCODEX_KEY_CTRL_F      6
#define XCODEX_KEY_CTRL_B      2

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64)
    #define XCODEX_WINDOWS 1
    #define XCODEX_POSIX 0
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define XCODEX_POSIX 1
    #define XCODEX_WINDOWS 0
#else
    #define XCODEX_POSIX 0
    #define XCODEX_WINDOWS 0
#endif

/* Platform-specific includes */
#if XCODEX_WINDOWS
    #include <windows.h>
    #include <conio.h>
    #include <io.h>
    #include <fcntl.h>
    #define STDIN_FILENO 0
    #define STDOUT_FILENO 1
    #define STDERR_FILENO 2
    #define access _access
    #define F_OK 0
    /* Define types that Windows doesn't have */
    #ifndef _SSIZE_T_DEFINED
    typedef long long ssize_t;
    #define _SSIZE_T_DEFINED
    #endif
#endif

#if XCODEX_POSIX
    #ifdef __linux__
    #define _POSIX_C_SOURCE 200809L
    #define _GNU_SOURCE
    #endif
    #ifdef __APPLE__
    #define _DARWIN_C_SOURCE
    #endif
    #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
    #endif
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <signal.h>
#endif

/* Common includes */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <fcntl.h>
#include "xcodex_types.h"
#include "syntax.h"
#include "themes.h"
#include "config.h"

/* Plugin system includes - conditional compilation */
#ifdef XCODEX_ENABLE_LUA
#include "xcodex_lua.h"
#endif

#ifdef XCODEX_ENABLE_COMPLETION
#include "xcodex_completion.h"
#endif

#ifdef XCODEX_ENABLE_LSP
#include "xcodex_lsp.h"
#endif

#define XCODEX_ENABLED 1

/* Cross-platform write function */
#if XCODEX_WINDOWS
int xcodex_write(int fd, const void *buf, size_t count) {
    DWORD written;
    HANDLE handle = (fd == STDOUT_FILENO) ? GetStdHandle(STD_OUTPUT_HANDLE) : GetStdHandle(STD_ERROR_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) return -1;
    if (!WriteConsoleA(handle, buf, count, &written, NULL)) return -1;
    return written;
}
#else
#define xcodex_write(fd, buf, count) write(fd, buf, count)
#endif

/* Windows compatibility functions */
#if XCODEX_WINDOWS
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) {
        errno = EINVAL;
        return -1;
    }
    
    if (*lineptr == NULL) {
        *n = 128;
        *lineptr = malloc(*n);
        if (!*lineptr) {
            errno = ENOMEM;
            return -1;
        }
    }
    
    ssize_t pos = 0;
    int c;
    
    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            *n *= 2;
            char *tmp = realloc(*lineptr, *n);
            if (!tmp) {
                errno = ENOMEM;
                return -1;
            }
            *lineptr = tmp;
        }
        
        (*lineptr)[pos++] = c;
        if (c == '\n') break;
    }
    
    if (pos == 0 && c == EOF) {
        return -1;
    }
    
    (*lineptr)[pos] = '\0';
    return pos;
}

/* Windows doesn't have ftruncate, so we implement it */
int ftruncate(int fd, off_t length) {
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    
    LARGE_INTEGER li;
    li.QuadPart = length;
    
    if (!SetFilePointerEx(handle, li, NULL, FILE_BEGIN)) {
        errno = EINVAL;
        return -1;
    }
    
    if (!SetEndOfFile(handle)) {
        errno = EINVAL;
        return -1;
    }
    
    return 0;
}
#endif

/* ============================ XCodex Global Variables ============================ */
/* Current theme index, used for syntax highlighting and UI colors */
static int current_theme = 0;

/* Forward declarations */
void editorSetStatusMessage(const char *fmt, ...);
void editorSetBackgroundColor(int color);
void editorMoveCursor(int key);
void editorInsertNewline(void);
void editorDelChar(void);
void editorInsertChar(int c);
int editorSave(void);
void editorFind(int fd);
void xcodex_execute_command(char *command);
void editorInsertRow(int at, char *s, size_t len);
void editorDelRow(int at);
char* editorRowsToString(int *buflen);

/* Theme management function declarations */
int xcodex_get_theme_index(const char *theme_name);
const char* xcodex_get_theme_name(int index);

/* Forward declarations that need erow */
void editorRowDelChar(erow *row, int at);
void editorUpdateRow(erow *row);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s, size_t len);

/* Undo system forward declarations */
void xcodex_init_undo_system(void);
void xcodex_free_undo_system(void);
void xcodex_push_undo(int type, int row, int col, const char *data, int data_len);
void xcodex_start_undo_group(void);
void xcodex_clear_undo_stack(void);
void xcodex_undo(void);
#if XCODEX_WINDOWS
void handleSigWinCh(void);
#endif
#if XCODEX_POSIX
void handleSigWinCh(int sig);
#endif
int xcodex_can_undo(void);

typedef struct hlcolor {
    int r,g,b;
} hlcolor;

/* ============================ XCodex Undo System ============================ */
#define UNDO_INSERT_CHAR    1
#define UNDO_DELETE_CHAR    2
#define UNDO_INSERT_LINE    3
#define UNDO_DELETE_LINE    4
#define UNDO_SPLIT_LINE     5
#define UNDO_JOIN_LINE      6
#define UNDO_REPLACE_TEXT   7

#define UNDO_STACK_SIZE     100

struct editorConfig E;

/* Global configuration settings that aren't part of the editor structure */
static int global_syntax_highlighting = 1;
static int global_show_status_bar = 1;
static int global_search_highlight = 1;
static int global_auto_indent = 1;
static int global_modal_editing = 1;
static int global_tab_size = 4;

/* Accessor functions for global settings */
int xcodex_get_syntax_highlighting(void) { return global_syntax_highlighting; }
int xcodex_get_show_status_bar(void) { return global_show_status_bar; }
int xcodex_get_search_highlight(void) { return global_search_highlight; }
int xcodex_get_auto_indent(void) { return global_auto_indent; }
int xcodex_get_modal_editing(void) { return global_modal_editing; }
int xcodex_get_tab_size(void) { return global_tab_size; }

/*Maps syntax highlight token types to themed colors*/
int editorSyntaxToColor(int hl) {
    if (hl >= 0 && hl < 17) {
        return themes[current_theme].colors[hl];
    }
    return themes[current_theme].colors[HL_NORMAL];
}

/*Theme switching function*/
void editorSwitchTheme(int theme_index) {
    if (theme_index < 0 || theme_index >= NUM_THEMES) {
        editorSetStatusMessage("Invalid theme index: %d (Available: 0-%d)", 
                            theme_index, NUM_THEMES - 1);
        return;
    }
    current_theme = theme_index;
    
    /* Apply background color with bounds checking */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        editorSetBackgroundColor(themes[current_theme].bg_color);
    }
    
    /* Update the status message to reflect the new theme */
    editorSetStatusMessage("Theme: %s (%d/%d) - Use Ctrl+T to cycle", 
                        themes[current_theme].name, 
                        current_theme + 1, 
                        NUM_THEMES);
}

/* Enhanced theme cycling with better feedback */
void editorCycleTheme(void) {
    current_theme = (current_theme + 1) % NUM_THEMES;
    
    /* Apply background color with bounds checking */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        editorSetBackgroundColor(themes[current_theme].bg_color);
    }
    
    editorSetStatusMessage("Theme: %s (%d/%d) - Use Ctrl+T to cycle", 
                        themes[current_theme].name,
                        current_theme + 1,
                        NUM_THEMES);
}

/* Enhanced theme name-to-index mapping function */
int xcodex_get_theme_index(const char *theme_name) {
    if (!theme_name) return 0; // Default to first theme
    
    // Map theme names to indices:
    // Index 0: "xcodex_dark" (default dark theme)
    // Index 1: "xcodex_light" (light theme) 
    // Index 2: "gruvbox_dark" (retro dark theme)
    // Index 3: "tokyo_night_dark" (modern dark blue)
    // Index 4: "tokyo_night_light" (light variant)
    // Index 5: "tokyo_night_storm" (storm variant)
    
    for (int i = 0; i < NUM_THEMES; i++) {
        if (strcmp(theme_name, themes[i].name) == 0) {
            return i;
        }
    }
    
    return -1; // Theme not found
}

/* Get theme name by index (for validation) */
const char* xcodex_get_theme_name(int index) {
    if (index >= 0 && index < NUM_THEMES) {
        return themes[index].name;
    }
    return NULL;
}

/* Calculate line number width based on total number of rows - Memory efficient */
void editorUpdateLineNumberWidth(void) {
    if (!E.show_line_numbers) {
        E.line_numbers_width = 0;
        return;
    }
    
    int max_line = E.numrows > 0 ? E.numrows : 1;
    E.line_numbers_width = 1;
    
    /* Performance optimized: calculate width without string operations */
    while (max_line >= 10) {
        E.line_numbers_width++;
        max_line /= 10;
    }
    
    E.line_numbers_width += 2; /* Add space for padding and separator */
}

/* Toggle line numbers on/off with theme compatibility */
void editorToggleLineNumbers(void) {
    E.show_line_numbers = !E.show_line_numbers;
    editorUpdateLineNumberWidth();
    editorSetStatusMessage("Line numbers: %s (Ctrl+N to toggle)", 
                        E.show_line_numbers ? "ON" : "OFF");
}

/* ============================ XCodex Modal System Functions ============================ */

/* Get human-readable mode name for status display */
const char* xcodex_get_mode_name(int mode) {
    switch (mode) {
        case XCODEX_MODE_NORMAL:      return "NORMAL";
        case XCODEX_MODE_INSERT:      return "INSERT";
        case XCODEX_MODE_VISUAL:      return "VISUAL";
        case XCODEX_MODE_VISUAL_LINE: return "VISUAL LINE";
        case XCODEX_MODE_VISUAL_BLOCK: return "VISUAL BLOCK";
        case XCODEX_MODE_COMMAND:     return "COMMAND";
        default:                      return "UNKNOWN";
    }
}

/* Switch editor mode with proper state management */
void xcodex_set_mode(int new_mode) {
    if (E.mode == new_mode) return;
    
    /* Save current state if needed */
    int old_mode = E.mode;
    E.mode = new_mode;
    
    /* Reset mode-specific state */
    E.motion_count = 0;
    E.command_len = 0;
    E.command_buffer[0] = '\0';
    
    /* Handle undo groups for mode transitions */
    if (old_mode == XCODEX_MODE_INSERT && new_mode != XCODEX_MODE_INSERT) {
        /* Exiting insert mode - end undo group */
        xcodex_start_undo_group();
    } else if (old_mode != XCODEX_MODE_INSERT && new_mode == XCODEX_MODE_INSERT) {
        /* Entering insert mode - start new undo group */
        xcodex_start_undo_group();
    }
    
    /* Mode-specific initialization */
    switch (new_mode) {
        case XCODEX_MODE_NORMAL:
            /* When entering normal mode, ensure cursor is on valid character */
            if (E.numrows > 0) {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows && E.cx > 0) {
                    erow *row = &E.row[filerow];
                    if (E.cx >= row->size && row->size > 0) {
                        E.cx = row->size - 1;
                    }
                }
            }
            editorSetStatusMessage("-- %s --", xcodex_get_mode_name(new_mode));
            break;
            
        case XCODEX_MODE_INSERT:
            editorSetStatusMessage("-- %s --", xcodex_get_mode_name(new_mode));
            break;
            
        case XCODEX_MODE_VISUAL:
            /* Initialize visual selection */
            E.visual_start_row = E.cy;
            E.visual_start_col = E.cx;
            E.visual_end_row = E.cy;
            E.visual_end_col = E.cx;
            E.visual_line_mode = 0;  /* Character-wise by default */
            editorSetStatusMessage("-- %s --", xcodex_get_mode_name(new_mode));
            break;
            
        case XCODEX_MODE_VISUAL_LINE:
            /* Initialize visual line selection */
            E.visual_start_row = E.cy;
            E.visual_start_col = 0;  /* Start of line */
            E.visual_end_row = E.cy;
            E.visual_end_col = -1;   /* End of line marker */
            E.visual_line_mode = 1;  /* Line-wise mode */
            editorSetStatusMessage("-- %s --", xcodex_get_mode_name(new_mode));
            break;
            
        case XCODEX_MODE_VISUAL_BLOCK:
            /* Initialize visual block selection */
            E.visual_start_row = E.cy;
            E.visual_start_col = E.cx;
            E.visual_end_row = E.cy;
            E.visual_end_col = E.cx;
            E.visual_line_mode = 0;  /* Character-wise by default */
            editorSetStatusMessage("-- %s --", xcodex_get_mode_name(new_mode));
            break;
            
        case XCODEX_MODE_COMMAND:
            /* Reset command buffer when entering command mode */
            E.command_len = 0;
            E.command_buffer[0] = '\0';
            editorSetStatusMessage(":");
            break;
    }
}

/* Initialize modal system */
void xcodex_init_modal_system(void) {
    E.mode = XCODEX_MODE_NORMAL;
    E.motion_count = 0;
    E.command_len = 0;
    E.command_buffer[0] = '\0';
    E.visual_start_row = 0;
    E.visual_start_col = 0;
    E.visual_end_row = 0;
    E.visual_end_col = 0;
    E.visual_line_mode = 0;
    E.last_search[0] = '\0';
    E.search_direction = 1;
    E.quit_requested = 0;
    
    /* Initialize yank buffer */
    E.yank_buffer = NULL;
    E.yank_buffer_size = 0;
    E.yank_is_line_mode = 0;
}

/* ============================ XCodex Motion Functions ============================ */

/* Check if character is a word boundary */
int xcodex_is_word_boundary(char c) {
    return !isalnum(c) && c != '_';
}

/* Move cursor to start of line (0) */
void xcodex_move_line_start(void) {
    E.cx = 0;
    E.coloff = 0;
}

/* Move cursor to end of line ($) */
void xcodex_move_line_end(void) {
    int filerow = E.rowoff + E.cy;
    if (filerow >= E.numrows || filerow < 0) return;
    
    erow *row = &E.row[filerow];
    int target_col = row->size > 0 ? row->size - 1 : 0;
    
    int effective_cols = E.screencols - E.line_numbers_width;
    if (target_col >= effective_cols) {
        E.coloff = target_col - effective_cols + 1;
        E.cx = effective_cols - 1;
    } else {
        E.cx = target_col;
        E.coloff = 0;
    }
}

/* Move to next word (w) */
void xcodex_move_word_forward(int count) {
    for (int i = 0; i < count; i++) {
        int filerow = E.rowoff + E.cy;
        if (filerow >= E.numrows) break;
        
        erow *row = &E.row[filerow];
        int filecol = E.coloff + E.cx;
        
        /* Skip current word */
        while (filecol < row->size && !xcodex_is_word_boundary(row->chars[filecol])) {
            filecol++;
        }
        
        /* Skip whitespace */
        while (filecol < row->size && xcodex_is_word_boundary(row->chars[filecol])) {
            filecol++;
        }
        
        /* Move to next line if at end */
        if (filecol >= row->size && filerow < E.numrows - 1) {
            E.cy++;
            E.cx = 0;
            E.coloff = 0;
            if (E.cy >= E.screenrows) {
                E.rowoff++;
                E.cy = E.screenrows - 1;
            }
        } else {
            /* Update cursor position */
            int effective_cols = E.screencols - E.line_numbers_width;
            if (filecol >= E.coloff + effective_cols) {
                E.coloff = filecol - effective_cols + 1;
                E.cx = effective_cols - 1;
            } else {
                E.cx = filecol - E.coloff;
            }
        }
    }
}

/* Move to previous word (b) */
void xcodex_move_word_backward(int count) {
    for (int i = 0; i < count; i++) {
        int filerow = E.rowoff + E.cy;
        if (filerow < 0) break;
        
        int filecol = E.coloff + E.cx;
        
        if (filecol == 0) {
            /* Move to previous line */
            if (filerow > 0) {
                E.cy--;
                if (E.cy < 0) {
                    E.rowoff--;
                    E.cy = 0;
                }
                
                /* Bounds check before accessing row */
                int new_filerow = E.rowoff + E.cy;
                if (new_filerow >= 0 && new_filerow < E.numrows) {
                    erow *row = &E.row[new_filerow];
                    filecol = row->size;
                    E.cx = filecol;
                    E.coloff = 0;
                    
                    int effective_cols = E.screencols - E.line_numbers_width;
                    if (E.cx >= effective_cols) {
                        E.coloff = E.cx - effective_cols + 1;
                        E.cx = effective_cols - 1;
                    }
                }
            }
        } else {
            /* Bounds check before accessing row */
            if (filerow >= 0 && filerow < E.numrows) {
                erow *row = &E.row[filerow];
                filecol--;
                
                /* Skip whitespace */
                while (filecol > 0 && xcodex_is_word_boundary(row->chars[filecol])) {
                    filecol--;
                }
                
                /* Skip word characters */
                while (filecol > 0 && !xcodex_is_word_boundary(row->chars[filecol])) {
                    filecol--;
                }
                
                if (filecol > 0) filecol++; /* Move to start of word */
                
                /* Update cursor position */
                if (filecol < E.coloff) {
                    E.coloff = filecol;
                    E.cx = 0;
                } else {
                    E.cx = filecol - E.coloff;
                }
            }
        }
    }
}

/* Go to specific line (G or :number) */
void xcodex_go_to_line(int line) {
    if (line < 1) line = 1;
    if (E.numrows == 0) return;  /* No lines to go to */
    if (line > E.numrows) line = E.numrows;
    
    line--; /* Convert to 0-based */
    
    E.cy = 0;
    E.rowoff = line;
    E.cx = 0;
    E.coloff = 0;
    
    /* Adjust if line is within screen */
    if (line < E.screenrows) {
        E.cy = line;
        E.rowoff = 0;
    }
}

/* Go to first line (gg) */
void xcodex_go_to_first_line(void) {
    E.cy = 0;
    E.rowoff = 0;
    E.cx = 0;
    E.coloff = 0;
}

/* Go to last line (G) */
void xcodex_go_to_last_line(void) {
    if (E.numrows == 0) return;  /* No lines to go to */
    
    int last_line = E.numrows - 1;
    E.cy = 0;
    E.rowoff = last_line;
    E.cx = 0;
    E.coloff = 0;
    
    /* Adjust if line is within screen */
    if (last_line < E.screenrows) {
        E.cy = last_line;
        E.rowoff = 0;
    }
}

/* ============================ XCodex Yank/Clipboard System ============================ */

/* Free the yank buffer */
void xcodex_free_yank_buffer(void) {
    if (E.yank_buffer) {
        free(E.yank_buffer);
        E.yank_buffer = NULL;
        E.yank_buffer_size = 0;
    }
}

/* Store text in yank buffer */
void xcodex_yank_text(const char *text, int size, int is_line_mode) {
    xcodex_free_yank_buffer();
    
    if (text && size > 0) {
        E.yank_buffer = malloc(size + 1);
        if (E.yank_buffer) {
            memcpy(E.yank_buffer, text, size);
            E.yank_buffer[size] = '\0';
            E.yank_buffer_size = size;
            E.yank_is_line_mode = is_line_mode;
        }
    }
}

/* Get selected text from visual selection */
char *xcodex_get_selected_text(int *size, int *is_line_mode) {
    if (E.mode != XCODEX_MODE_VISUAL && E.mode != XCODEX_MODE_VISUAL_LINE && E.mode != XCODEX_MODE_VISUAL_BLOCK) {
        *size = 0;
        return NULL;
    }
    
    /* Convert screen coordinates to file coordinates */
    int start_row = E.rowoff + E.visual_start_row;
    int start_col = E.coloff + E.visual_start_col;
    int end_row = E.rowoff + E.visual_end_row;
    int end_col = E.coloff + E.visual_end_col;
    
    /* Normalize selection (start should be before end) */
    if (start_row > end_row || (start_row == end_row && start_col > end_col)) {
        int temp = start_row;
        start_row = end_row;
        end_row = temp;
        temp = start_col;
        start_col = end_col;
        end_col = temp;
    }
    
    /* Validate bounds */
    if (start_row < 0) start_row = 0;
    if (end_row >= E.numrows) end_row = E.numrows - 1;
    if (start_row > end_row) {
        *size = 0;
        return NULL;
    }
    
    *is_line_mode = (E.mode == XCODEX_MODE_VISUAL_LINE);
    
    /* Calculate total size needed */
    int total_size = 0;
    for (int row = start_row; row <= end_row; row++) {
        if (row >= E.numrows) break;
        
        erow *r = &E.row[row];
        if (E.mode == XCODEX_MODE_VISUAL_LINE) {
            /* Include entire line */
            total_size += r->size + 1; /* +1 for newline */
        } else if (E.mode == XCODEX_MODE_VISUAL_BLOCK) {
            /* Block-wise selection */
            int start_pos = start_col;
            int end_pos = end_col;
            
            if (start_pos < 0) start_pos = 0;
            if (end_pos >= r->size) end_pos = r->size - 1;
            
            if (start_pos <= end_pos) {
                total_size += (end_pos - start_pos + 1);
                if (row < end_row) total_size++; /* +1 for newline */
            }
        } else {
            /* Character-wise selection */
            int start_pos = (row == start_row) ? start_col : 0;
            int end_pos = (row == end_row) ? end_col : r->size - 1;
            
            if (start_pos < 0) start_pos = 0;
            if (end_pos >= r->size) end_pos = r->size - 1;
            
            if (start_pos <= end_pos) {
                total_size += (end_pos - start_pos + 1);
                if (row < end_row) total_size++; /* +1 for newline */
            }
        }
    }
    
    if (total_size == 0) {
        *size = 0;
        return NULL;
    }
    
    /* Allocate buffer */
    char *buffer = malloc(total_size + 1);
    if (!buffer) {
        *size = 0;
        return NULL;
    }
    
    /* Copy selected text */
    int pos = 0;
    for (int row = start_row; row <= end_row; row++) {
        if (row >= E.numrows) break;
        
        erow *r = &E.row[row];
        if (E.mode == XCODEX_MODE_VISUAL_LINE) {
            /* Include entire line */
            memcpy(buffer + pos, r->chars, r->size);
            pos += r->size;
            buffer[pos++] = '\n';
        } else if (E.mode == XCODEX_MODE_VISUAL_BLOCK) {
            /* Block-wise selection */
            int start_pos = start_col;
            int end_pos = end_col;
            
            if (start_pos < 0) start_pos = 0;
            if (end_pos >= r->size) end_pos = r->size - 1;
            
            if (start_pos <= end_pos) {
                int copy_len = end_pos - start_pos + 1;
                memcpy(buffer + pos, r->chars + start_pos, copy_len);
                pos += copy_len;
                if (row < end_row) {
                    buffer[pos++] = '\n';
                }
            }
        } else {
            /* Character-wise selection */
            int start_pos = (row == start_row) ? start_col : 0;
            int end_pos = (row == end_row) ? end_col : r->size - 1;
            
            if (start_pos < 0) start_pos = 0;
            if (end_pos >= r->size) end_pos = r->size - 1;
            
            if (start_pos <= end_pos) {
                int copy_len = end_pos - start_pos + 1;
                memcpy(buffer + pos, r->chars + start_pos, copy_len);
                pos += copy_len;
                if (row < end_row) {
                    buffer[pos++] = '\n';
                }
            }
        }
    }
    
    buffer[pos] = '\0';
    *size = pos;
    return buffer;
}

/* Delete selected text */
void xcodex_delete_selected_text(void) {
    if (E.mode != XCODEX_MODE_VISUAL && E.mode != XCODEX_MODE_VISUAL_LINE && E.mode != XCODEX_MODE_VISUAL_BLOCK) {
        return;
    }
    
    /* Start new undo group for the deletion operation */
    xcodex_start_undo_group();
    
    /* Convert screen coordinates to file coordinates */
    int start_row = E.rowoff + E.visual_start_row;
    int start_col = E.coloff + E.visual_start_col;
    int end_row = E.rowoff + E.visual_end_row;
    int end_col = E.coloff + E.visual_end_col;
    
    /* Normalize selection (start should be before end) */
    if (start_row > end_row || (start_row == end_row && start_col > end_col)) {
        int temp = start_row;
        start_row = end_row;
        end_row = temp;
        temp = start_col;
        start_col = end_col;
        end_col = temp;
    }
    
    /* Validate bounds */
    if (start_row < 0) start_row = 0;
    if (end_row >= E.numrows) end_row = E.numrows - 1;
    if (start_row > end_row || start_row >= E.numrows) return;  /* Nothing to delete */
    
    if (E.mode == XCODEX_MODE_VISUAL_LINE) {
        /* Delete entire lines */
        for (int row = end_row; row >= start_row; row--) {
            if (row >= 0 && row < E.numrows) {  /* Bounds check */
                editorDelRow(row);
            }
        }
        
        /* Position cursor at start of selection - with bounds checking */
        if (start_row < E.numrows) {
            E.cy = start_row - E.rowoff;
            if (E.cy < 0) {
                E.rowoff = start_row;
                E.cy = 0;
            }
        }
        E.cx = 0;
        E.coloff = 0;
    } else {
        /* Character-wise or block-wise deletion */
        if (start_row == end_row) {
            /* Single line deletion */
            if (start_row >= 0 && start_row < E.numrows) {
                erow *row = &E.row[start_row];
                if (start_col >= 0 && start_col < row->size && end_col >= 0 && end_col < row->size && start_col <= end_col) {
                    int delete_len = end_col - start_col + 1;
                    if (delete_len > 0 && start_col + delete_len <= row->size) {
                        memmove(row->chars + start_col, 
                                row->chars + start_col + delete_len,
                                row->size - start_col - delete_len);
                        row->size -= delete_len;
                        row->chars[row->size] = '\0';
                        editorUpdateRow(row);
                    }
                }
            }
        } else {
            /* Multi-line deletion - simplified and safer */
            for (int row = end_row; row >= start_row; row--) {
                if (row >= 0 && row < E.numrows) {
                    if (row == start_row && row == end_row) {
                        /* Single line case handled above */
                    } else if (row == start_row) {
                        /* First line - delete from start_col to end */
                        erow *row_ptr = &E.row[row];
                        if (start_col >= 0 && start_col < row_ptr->size) {
                            row_ptr->size = start_col;
                            row_ptr->chars[row_ptr->size] = '\0';
                            editorUpdateRow(row_ptr);
                        }
                    } else if (row == end_row) {
                        /* Last line - delete from beginning to end_col */
                        erow *row_ptr = &E.row[row];
                        if (end_col >= 0 && end_col < row_ptr->size) {
                            int remaining = row_ptr->size - end_col - 1;
                            if (remaining > 0) {
                                memmove(row_ptr->chars, row_ptr->chars + end_col + 1, remaining);
                                row_ptr->size = remaining;
                                row_ptr->chars[row_ptr->size] = '\0';
                                editorUpdateRow(row_ptr);
                            } else {
                                row_ptr->size = 0;
                                row_ptr->chars[0] = '\0';
                                editorUpdateRow(row_ptr);
                            }
                        }
                    } else {
                        /* Middle lines - delete entire line */
                        editorDelRow(row);
                    }
                }
            }
            
            /* Merge first and last lines if they exist */
            if (start_row < E.numrows && start_row + 1 < E.numrows) {
                erow *first_row = &E.row[start_row];
                erow *second_row = &E.row[start_row + 1];
                editorRowAppendString(first_row, second_row->chars, second_row->size);
                editorDelRow(start_row + 1);
            }
        }
        
        /* Position cursor at start of selection - with bounds checking */
        E.cy = start_row - E.rowoff;
        if (E.cy < 0) {
            E.rowoff = start_row;
            E.cy = 0;
        }
        E.cx = start_col - E.coloff;
        if (E.cx < 0) {
            E.coloff = start_col;
            E.cx = 0;
        }
    }
    
    /* Ensure cursor is within bounds */
    if (E.cy >= E.screenrows) E.cy = E.screenrows - 1;
    if (E.cx >= E.screencols - E.line_numbers_width) E.cx = E.screencols - E.line_numbers_width - 1;
    
    E.dirty++;
}

/* Paste text from yank buffer */
void xcodex_paste_text(void) {
    if (!E.yank_buffer || E.yank_buffer_size == 0) {
        editorSetStatusMessage("Nothing to paste");
        return;
    }
    
    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    
    if (E.yank_is_line_mode) {
        /* Line-wise paste - insert after current line */
        int paste_row = filerow + 1;
        
        /* Split buffer into lines and insert each */
        char *text = E.yank_buffer;
        int pos = 0;
        
        while (pos < E.yank_buffer_size) {
            int line_end = pos;
            while (line_end < E.yank_buffer_size && text[line_end] != '\n') {
                line_end++;
            }
            
            int line_len = line_end - pos;
            if (line_len > 0) {
                editorInsertRow(paste_row, text + pos, line_len);
                paste_row++;
            }
            
            pos = line_end + 1; /* Skip newline */
        }
        
        /* Move cursor to first pasted line */
        E.cy++;
        E.cx = 0;
        E.coloff = 0;
        
        if (E.cy >= E.screenrows) {
            E.rowoff++;
            E.cy = E.screenrows - 1;
        }
    } else {
        /* Character-wise paste */
        char *text = E.yank_buffer;
        int pos = 0;
        
        while (pos < E.yank_buffer_size) {
            if (text[pos] == '\n') {
                editorInsertNewline();
                pos++;
            } else {
                editorInsertChar(text[pos]);
                pos++;
            }
        }
    }
    
    editorSetStatusMessage("Pasted %d bytes", E.yank_buffer_size);
}

/* Delete current line */
void xcodex_delete_current_line(void) {
    int filerow = E.rowoff + E.cy;
    
    if (filerow < 0 || filerow >= E.numrows) {
        editorSetStatusMessage("No line to delete");
        return;
    }
    
    /* Get the current line content for yank buffer */
    erow *row = &E.row[filerow];
    char *line_content = malloc(row->size + 2); /* +2 for newline and null terminator */
    if (line_content) {
        memcpy(line_content, row->chars, row->size);
        line_content[row->size] = '\n';
        line_content[row->size + 1] = '\0';
        
        /* Yank the line */
        xcodex_yank_text(line_content, row->size + 1, 1); /* 1 = line mode */
        free(line_content);
    }
    
    /* Delete the line */
    editorDelRow(filerow);
    
    /* Adjust cursor position */
    if (E.numrows == 0) {
        /* No lines left, create an empty line */
        editorInsertRow(0, "", 0);
        E.cy = 0;
        E.cx = 0;
        E.coloff = 0;
        E.rowoff = 0;
    } else {
        /* If we deleted the last line, move cursor up */
        if (filerow >= E.numrows) {
            if (E.cy > 0) {
                E.cy--;
            } else if (E.rowoff > 0) {
                E.rowoff--;
            }
        }
        
        /* Make sure cursor is at the beginning of the line */
        E.cx = 0;
        E.coloff = 0;
    }
    
    editorSetStatusMessage("Deleted line");
}

/* ============================ XCodex Undo System ============================ */

/* Initialize the undo system */
void xcodex_init_undo_system(void) {
    E.undo_stack = malloc(sizeof(struct undoEntry) * UNDO_STACK_SIZE);
    if (!E.undo_stack) {
        fprintf(stderr, "Failed to allocate undo stack\n");
        exit(1);
    }
    
    /* Initialize all entries to safe values */
    for (int i = 0; i < UNDO_STACK_SIZE; i++) {
        E.undo_stack[i].type = 0;
        E.undo_stack[i].row = 0;
        E.undo_stack[i].col = 0;
        E.undo_stack[i].data = NULL;
        E.undo_stack[i].data_len = 0;
        E.undo_stack[i].group_id = 0;
        E.undo_stack[i].cursor_row = 0;
        E.undo_stack[i].cursor_col = 0;
    }
    
    E.undo_count = 0;
    E.undo_capacity = UNDO_STACK_SIZE;
    E.undo_group_id = 0;
    E.xcodex_undo_in_progress = 0;
}

/* Free the undo system */
void xcodex_free_undo_system(void) {
    if (E.undo_stack) {
        /* Free all data in undo entries */
        for (int i = 0; i < E.undo_count; i++) {
            if (E.undo_stack[i].data) {
                free(E.undo_stack[i].data);
                E.undo_stack[i].data = NULL;
            }
        }
        free(E.undo_stack);
        E.undo_stack = NULL;
    }
    E.undo_count = 0;
    E.undo_capacity = 0;
    E.xcodex_undo_in_progress = 0;
}

/* Start a new undo group for related operations */
void xcodex_start_undo_group(void) {
    E.undo_group_id++;
}

/* Push an undo entry onto the stack */
void xcodex_push_undo(int type, int row, int col, const char *data, int data_len) {
    /* Don't push undo entries if an undo operation is in progress */
    if (E.xcodex_undo_in_progress) {
        return;
    }
    
    if (!E.undo_stack || E.undo_count >= E.undo_capacity) {
        return; /* Stack full or not initialized */
    }
    
    /* If stack is nearly full, remove oldest entries */
    if (E.undo_count >= UNDO_STACK_SIZE - 1) {
        /* Free the oldest entry */
        if (E.undo_stack[0].data) {
            free(E.undo_stack[0].data);
        }
        
        /* Shift all entries forward */
        for (int i = 0; i < E.undo_count - 1; i++) {
            E.undo_stack[i] = E.undo_stack[i + 1];
        }
        E.undo_count--;
    }
    
    struct undoEntry *entry = &E.undo_stack[E.undo_count];
    entry->type = type;
    entry->row = row;
    entry->col = col;
    entry->group_id = E.undo_group_id;
    
    /* Save cursor position with bounds checking */
    int cursor_row = E.rowoff + E.cy;
    int cursor_col = E.coloff + E.cx;
    
    /* Validate cursor position */
    if (cursor_row < 0) cursor_row = 0;
    if (cursor_row >= E.numrows) cursor_row = E.numrows > 0 ? E.numrows - 1 : 0;
    if (cursor_col < 0) cursor_col = 0;
    
    entry->cursor_row = cursor_row;
    entry->cursor_col = cursor_col;
    
    /* Copy data if provided */
    if (data && data_len > 0) {
        entry->data = malloc(data_len + 1);
        if (entry->data) {
            memcpy(entry->data, data, data_len);
            entry->data[data_len] = '\0';
            entry->data_len = data_len;
        } else {
            /* Memory allocation failed - don't add this entry */
            return;
        }
    } else {
        entry->data = NULL;
        entry->data_len = 0;
    }
    
    E.undo_count++;
}

/* Clear the undo stack */
void xcodex_clear_undo_stack(void) {
    if (E.undo_stack) {
        /* Free all data in undo entries */
        for (int i = 0; i < E.undo_count; i++) {
            if (E.undo_stack[i].data) {
                free(E.undo_stack[i].data);
                E.undo_stack[i].data = NULL;
            }
        }
        E.undo_count = 0;
        E.undo_group_id = 0;
    }
}

/* Check if undo is possible */
int xcodex_can_undo(void) {
    return E.undo_count > 0 && !E.xcodex_undo_in_progress;
}


/* Perform undo operation */
void xcodex_undo(void) {
    if (E.undo_count == 0) {
        editorSetStatusMessage("Nothing to undo");
        return;
    }
    
    /* Prevent recursive undo operations */
    if (E.xcodex_undo_in_progress) {
        editorSetStatusMessage("Undo operation already in progress");
        return;
    }
    
    /* Set the flag to prevent recursion */
    E.xcodex_undo_in_progress = 1;
    
    /* Get the last undo group and save cursor position BEFORE any modifications */
    int target_group = E.undo_stack[E.undo_count - 1].group_id;
    int saved_cursor_row = E.undo_stack[E.undo_count - 1].cursor_row;
    int saved_cursor_col = E.undo_stack[E.undo_count - 1].cursor_col;
    int operations_undone = 0;
    
    /* Undo all operations in the group (in reverse order) */
    while (E.undo_count > 0 && E.undo_stack[E.undo_count - 1].group_id == target_group) {
        struct undoEntry *entry = &E.undo_stack[E.undo_count - 1];
        
        switch (entry->type) {
            case UNDO_INSERT_CHAR:
                /* Undo character insertion by deleting it */
                if (entry->row >= 0 && entry->row < E.numrows) {
                    erow *row = &E.row[entry->row];
                    if (entry->col >= 0 && entry->col < row->size) {
                        /* Direct character deletion without triggering undo */
                        for (int i = entry->col; i < row->size - 1; i++) {
                            row->chars[i] = row->chars[i + 1];
                        }
                        row->size--;
                        row->chars[row->size] = '\0';
                        editorUpdateRow(row);
                    }
                }
                break;
                
            case UNDO_DELETE_CHAR:
                /* Undo character deletion by inserting it back */
                if (entry->data && entry->data_len > 0) {
                    if (entry->row >= 0 && entry->row < E.numrows) {
                        erow *row = &E.row[entry->row];
                        if (entry->col >= 0 && entry->col <= row->size) {
                            /* Direct character insertion without triggering undo */
                            row->chars = realloc(row->chars, row->size + 2);
                            if (row->chars) {
                                for (int i = row->size; i > entry->col; i--) {
                                    row->chars[i] = row->chars[i - 1];
                                }
                                row->chars[entry->col] = entry->data[0];
                                row->size++;
                                row->chars[row->size] = '\0';
                                editorUpdateRow(row);
                            }
                        }
                    }
                }
                break;
                
            case UNDO_INSERT_LINE:
                /* Undo line insertion by deleting it */
                if (entry->row >= 0 && entry->row < E.numrows) {
                    /* Direct line deletion without triggering undo */
                    erow *row = &E.row[entry->row];
                    free(row->chars);
                    free(row->render);
                    free(row->hl);
                    
                    /* Shift all rows up */
                    for (int i = entry->row; i < E.numrows - 1; i++) {
                        E.row[i] = E.row[i + 1];
                        E.row[i].idx = i;
                    }
                    E.numrows--;
                    if (E.numrows == 0) {
                        free(E.row);
                        E.row = NULL;
                    } else {
                        E.row = realloc(E.row, sizeof(erow) * E.numrows);
                    }
                }
                break;
                
            case UNDO_DELETE_LINE:
                /* Undo line deletion by inserting it back */
                if (entry->data && entry->row >= 0 && entry->row <= E.numrows) {
                    /* Direct line insertion without triggering undo */
                    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
                    if (E.row) {
                        /* Shift rows down */
                        for (int i = E.numrows; i > entry->row; i--) {
                            E.row[i] = E.row[i - 1];
                            E.row[i].idx = i;
                        }
                        
                        /* Create new row */
                        E.row[entry->row].size = entry->data_len;
                        E.row[entry->row].chars = malloc(entry->data_len + 1);
                        if (E.row[entry->row].chars) {
                            memcpy(E.row[entry->row].chars, entry->data, entry->data_len);
                            E.row[entry->row].chars[entry->data_len] = '\0';
                            E.row[entry->row].idx = entry->row;
                            E.row[entry->row].render = NULL;
                            E.row[entry->row].hl = NULL;
                            E.row[entry->row].rsize = 0;
                            E.row[entry->row].hl_oc = 0;
                            editorUpdateRow(&E.row[entry->row]);
                            E.numrows++;
                        }
                    }
                }
                break;
                
            case UNDO_SPLIT_LINE:
                /* Undo line split by joining the lines */
                if (entry->row >= 0 && entry->row < E.numrows - 1) {
                    erow *row = &E.row[entry->row];
                    erow *next_row = &E.row[entry->row + 1];
                    
                    /* Join lines directly */
                    row->chars = realloc(row->chars, row->size + next_row->size + 1);
                    if (row->chars) {
                        memcpy(row->chars + row->size, next_row->chars, next_row->size);
                        row->size += next_row->size;
                        row->chars[row->size] = '\0';
                        editorUpdateRow(row);
                        
                        /* Delete the next row */
                        free(next_row->chars);
                        free(next_row->render);
                        free(next_row->hl);
                        
                        /* Shift rows up */
                        for (int i = entry->row + 1; i < E.numrows - 1; i++) {
                            E.row[i] = E.row[i + 1];
                            E.row[i].idx = i;
                        }
                        E.numrows--;
                        E.row = realloc(E.row, sizeof(erow) * E.numrows);
                    }
                }
                break;
                
            case UNDO_JOIN_LINE:
                /* Undo line join by splitting the line */
                if (entry->row >= 0 && entry->row < E.numrows && entry->data) {
                    erow *row = &E.row[entry->row];
                    if (entry->col >= 0 && entry->col <= row->size) {
                        /* Create new row for the split */
                        E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
                        if (E.row) {
                            /* Shift rows down */
                            for (int i = E.numrows; i > entry->row + 1; i--) {
                                E.row[i] = E.row[i - 1];
                                E.row[i].idx = i;
                            }
                            
                            /* Create the new row with the split content */
                            int new_size = row->size - entry->col;
                            E.row[entry->row + 1].size = new_size;
                            E.row[entry->row + 1].chars = malloc(new_size + 1);
                            if (E.row[entry->row + 1].chars) {
                                memcpy(E.row[entry->row + 1].chars, row->chars + entry->col, new_size);
                                E.row[entry->row + 1].chars[new_size] = '\0';
                                E.row[entry->row + 1].idx = entry->row + 1;
                                E.row[entry->row + 1].render = NULL;
                                E.row[entry->row + 1].hl = NULL;
                                E.row[entry->row + 1].rsize = 0;
                                E.row[entry->row + 1].hl_oc = 0;
                                editorUpdateRow(&E.row[entry->row + 1]);
                                
                                /* Truncate the original row */
                                row->chars[entry->col] = '\0';
                                row->size = entry->col;
                                editorUpdateRow(row);
                                E.numrows++;
                            }
                        }
                    }
                }
                break;
        }
        
        /* Free the entry data */
        if (entry->data) {
            free(entry->data);
            entry->data = NULL;
        }
        
        E.undo_count--;
        operations_undone++;
    }
    
    /* Restore cursor position using saved values */
    if (operations_undone > 0) {
        /* Set cursor position */
        int target_row = saved_cursor_row;
        int target_col = saved_cursor_col;
        
        /* Validate row bounds */
        if (target_row < 0) target_row = 0;
        if (target_row >= E.numrows) target_row = E.numrows > 0 ? E.numrows - 1 : 0;
        
        /* Validate column bounds */
        if (E.numrows > 0 && target_row < E.numrows) {
            if (target_col < 0) target_col = 0;
            if (target_col > E.row[target_row].size) target_col = E.row[target_row].size;
        } else {
            target_col = 0;
        }
        
        /* Adjust for screen position */
        if (target_row < E.rowoff) {
            E.rowoff = target_row;
            E.cy = 0;
        } else if (target_row >= E.rowoff + E.screenrows) {
            E.rowoff = target_row - E.screenrows + 1;
            if (E.rowoff < 0) E.rowoff = 0;
            E.cy = E.screenrows - 1;
        } else {
            E.cy = target_row - E.rowoff;
        }
        
        int effective_cols = E.screencols - E.line_numbers_width;
        if (effective_cols < 1) effective_cols = 1;
        
        if (target_col < E.coloff) {
            E.coloff = target_col;
            E.cx = 0;
        } else if (target_col >= E.coloff + effective_cols) {
            E.coloff = target_col - effective_cols + 1;
            if (E.coloff < 0) E.coloff = 0;
            E.cx = effective_cols - 1;
        } else {
            E.cx = target_col - E.coloff;
        }
        
        /* Ensure bounds */
        if (E.cy < 0) E.cy = 0;
        if (E.cy >= E.screenrows) E.cy = E.screenrows - 1;
        if (E.cx < 0) E.cx = 0;
        if (E.cx >= effective_cols) E.cx = effective_cols - 1;
        
        editorSetStatusMessage("Undone %d operation%s", operations_undone, 
                            operations_undone == 1 ? "" : "s");
        E.dirty++;
    }
    
    /* Clear the recursion flag */
    E.xcodex_undo_in_progress = 0;
}

enum KEY_ACTION{
        KEY_NULL = 0,       /* NULL */
        CTRL_A = 1,         /* Ctrl-a */
        CTRL_B = 2,         /* Ctrl-b */
        CTRL_C = 3,         /* Ctrl-c */
        CTRL_D = 4,         /* Ctrl-d */
        CTRL_E = 5,         /* Ctrl-e */
        CTRL_F = 6,         /* Ctrl-f */
        CTRL_G = 7,         /* Ctrl-g */
        CTRL_H = 8,         /* Ctrl-h */
        CTRL_I = 9,         /* Ctrl-i (Tab) */
        CTRL_J = 10,        /* Ctrl-j */
        CTRL_K = 11,        /* Ctrl-k */
        CTRL_L = 12,        /* Ctrl+l */
        CTRL_M = 13,        /* Ctrl-m (Enter) */
        CTRL_N = 14,        /* Ctrl-n */
        CTRL_O = 15,        /* Ctrl-o */
        CTRL_P = 16,        /* Ctrl-p */
        CTRL_Q = 17,        /* Ctrl-q */
        CTRL_R = 18,        /* Ctrl-r */
        CTRL_S = 19,        /* Ctrl-s */
        CTRL_T = 20,        /* Ctrl-t */
        CTRL_U = 21,        /* Ctrl-u */
        CTRL_V = 22,        /* Ctrl-v */
        CTRL_W = 23,        /* Ctrl-w */
        CTRL_X = 24,        /* Ctrl-x */
        CTRL_Y = 25,        /* Ctrl-y */
        CTRL_Z = 26,        /* Ctrl-z */
        TAB = 9,            /* Tab */
        ENTER = 13,         /* Enter */
        ESC = 27,           /* Escape */
        BACKSPACE =  127,   /* Backspace */
        /* The following are just soft codes, not really reported by the
         * terminal directly. */
        ARROW_LEFT = 1000,
        ARROW_RIGHT,
        ARROW_UP,
        ARROW_DOWN,
        DEL_KEY,
        HOME_KEY,
        END_KEY,
        PAGE_UP,
        PAGE_DOWN
};

/* Forward declarations for append buffer */
void abAppend(struct abuf *ab, const char *s, int len);



/* ======================= Low level terminal handling ====================== */

/* Platform-specific terminal state */
#if XCODEX_WINDOWS
static HANDLE hStdin, hStdout;
static DWORD dwOriginalMode;
static CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif

#if XCODEX_POSIX
static struct termios orig_termios; /* In order to restore at exit.*/
#endif

#if XCODEX_WINDOWS
void disableRawMode(int fd) {
    if (E.rawmode && hStdin != INVALID_HANDLE_VALUE) {
        SetConsoleMode(hStdin, dwOriginalMode);
        E.rawmode = 0;
    }
}
#endif

#if XCODEX_POSIX
void disableRawMode(int fd) {
    /* Don't even check the return value as it's too late. */
    if (E.rawmode) {
        tcsetattr(fd,TCSAFLUSH,&orig_termios);
        E.rawmode = 0;
    }
}
#endif

/* Called at exit to avoid remaining in raw mode. */
void editorAtExit(void) {
    disableRawMode(STDIN_FILENO);
    
    /* Cleanup plugin systems */
#ifdef XCODEX_ENABLE_COMPLETION
    xcodex_completion_cleanup();
#endif

#ifdef XCODEX_ENABLE_LUA
    xcodex_lua_cleanup();
#endif

#ifdef XCODEX_ENABLE_LSP
    xcodex_lsp_cleanup();
#endif
    
    /* Free yank buffer */
    xcodex_free_yank_buffer();
    /* Free undo system */
    xcodex_free_undo_system();
    /* Reset background color */
    editorSetBackgroundColor(-1);
    /* Clear the screen and reposition cursor at top-left on exit. */
#if XCODEX_WINDOWS
    DWORD written;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        WriteConsoleA(hOut, "\x1b[2J", 4, &written, NULL);
        WriteConsoleA(hOut, "\x1b[H", 3, &written, NULL);
    }
#else
    if (xcodex_write(STDOUT_FILENO, "\x1b[2J", 4) == -1) {
        perror("write");
    }
    if (xcodex_write(STDOUT_FILENO, "\x1b[H", 3) == -1) {
        perror("write");
    }
#endif
}

/* Raw mode: Cross-platform terminal control */
#if XCODEX_WINDOWS
int enableRawMode(int fd) {
    if (E.rawmode) return 0;
    
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hStdin == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    if (!GetConsoleMode(hStdin, &dwOriginalMode)) {
        return -1;
    }
    
    DWORD dwMode = dwOriginalMode;
    dwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    dwMode |= ENABLE_WINDOW_INPUT;  /* Enable window resize events */
    
    if (!SetConsoleMode(hStdin, dwMode)) {
        return -1;
    }
    
    /* Try to enable VT100 sequences for output (optional) */
    DWORD dwOutMode;
    if (GetConsoleMode(hStdout, &dwOutMode)) {
        dwOutMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hStdout, dwOutMode);  /* Don't fail if this doesn't work */
    }
    
    E.rawmode = 1;
    return 0;
}
#endif

#if XCODEX_POSIX
int enableRawMode(int fd) {
    struct termios raw;

    if (E.rawmode) return 0; /* Already enabled. */
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(editorAtExit);
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    E.rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}
#endif

/* Read a key from the terminal put in raw mode, trying to handle
 * escape sequences. */
#if XCODEX_WINDOWS
int editorReadKey(int fd) {
    INPUT_RECORD irec;
    DWORD events;
    
    while (1) {
        /* Use ReadConsoleInput to get both key and window events */
        if (!ReadConsoleInput(hStdin, &irec, 1, &events) || events == 0) {
            Sleep(1);
            continue;
        }
        
        if (irec.EventType == KEY_EVENT && irec.Event.KeyEvent.bKeyDown) {
            KEY_EVENT_RECORD keyEvent = irec.Event.KeyEvent;
            
            /* Handle special keys first */
            switch (keyEvent.wVirtualKeyCode) {
                case VK_ESCAPE:
                    return ESC;
                case VK_UP:
                    return ARROW_UP;
                case VK_DOWN:
                    return ARROW_DOWN;
                case VK_LEFT:
                    return ARROW_LEFT;
                case VK_RIGHT:
                    return ARROW_RIGHT;
                case VK_DELETE:
                    return DEL_KEY;
                case VK_HOME:
                    return HOME_KEY;
                case VK_END:
                    return END_KEY;
                case VK_PRIOR:  /* Page Up */
                    return PAGE_UP;
                case VK_NEXT:   /* Page Down */
                    return PAGE_DOWN;
                case VK_BACK:   /* Backspace */
                    return BACKSPACE;
                case VK_TAB:
                    return TAB;
                case VK_RETURN:
                    return ENTER;
                case VK_SPACE:
                    return ' ';
                default:
                    /* Handle control keys */
                    if (keyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                        switch (keyEvent.wVirtualKeyCode) {
                            case 'A': return CTRL_A;
                            case 'B': return CTRL_B;
                            case 'C': return CTRL_C;
                            case 'D': return CTRL_D;
                            case 'E': return CTRL_E;
                            case 'F': return CTRL_F;
                            case 'G': return CTRL_G;
                            case 'H': return CTRL_H;
                            case 'I': return CTRL_I;
                            case 'J': return CTRL_J;
                            case 'K': return CTRL_K;
                            case 'L': return CTRL_L;
                            case 'M': return CTRL_M;
                            case 'N': return CTRL_N;
                            case 'O': return CTRL_O;
                            case 'P': return CTRL_P;
                            case 'Q': return CTRL_Q;
                            case 'R': return CTRL_R;
                            case 'S': return CTRL_S;
                            case 'T': return CTRL_T;
                            case 'U': return CTRL_U;
                            case 'V': return CTRL_V;
                            case 'W': return CTRL_W;
                            case 'X': return CTRL_X;
                            case 'Y': return CTRL_Y;
                            case 'Z': return CTRL_Z;
                        }
                    }
                    
                    /* Handle regular characters */
                    if (keyEvent.uChar.AsciiChar != 0) {
                        return keyEvent.uChar.AsciiChar;
                    }
                    break;
            }
        }
        else if (irec.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            /* Handle window resize */
            handleSigWinCh();
        }
    }
    
    return -1;
}
#endif

#if XCODEX_POSIX
int editorReadKey(int fd) {
    int nread;
    char c, seq[3];
    while ((nread = read(fd,&c,1)) == 0);
    if (nread == -1) exit(1);

    while(1) {
        switch(c) {
        case ESC:    /* escape sequence */
            /* Use select() to implement timeout for ESC detection */
            fd_set readfds;
            struct timeval timeout;
            
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;  /* 100ms timeout */
            
            int ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
            if (ready <= 0) {
                return ESC;  /* Timeout or error - this is a plain ESC */
            }
            
            /* If this is just an ESC, we'll timeout here. */
            if (read(fd,seq,1) == 0) return ESC;
            
            /* Check for second character with timeout */
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;  /* 100ms timeout */
            
            ready = select(fd + 1, &readfds, NULL, NULL, &timeout);
            if (ready <= 0) {
                return ESC;  /* Timeout or error - this is a plain ESC */
            }
            
            if (read(fd,seq+1,1) == 0) return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (read(fd,seq+2,1) == 0) return ESC;
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        case '3': return DEL_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    }
                }
            }

            /* ESC O sequences. */
            else if (seq[0] == 'O') {
                switch(seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
            break;
        default:
            return c;
        }
    }
}
#endif

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor is stored at *rows and *cols and 0 is returned. */
#if XCODEX_WINDOWS
int getCursorPosition(int ifd, int ofd, int *rows, int *cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hOut == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        *cols = csbi.dwCursorPosition.X + 1;
        *rows = csbi.dwCursorPosition.Y + 1;
        return 0;
    }
    return -1;
}
#endif

#if XCODEX_POSIX
int getCursorPosition(int ifd, int ofd, int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    /* Report cursor location */
    if (write(ofd, "\x1b[6n", 4) != 4) return -1;

    /* Read the response: ESC [ rows ; cols R */
    while (i < sizeof(buf)-1) {
        if (read(ifd,buf+i,1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    /* Parse it. */
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",rows,cols) != 2) return -1;
    return 0;
}
#endif

/* Try to get the number of columns in the current terminal. If the ioctl()
 * call fails the function will try to query the terminal itself.
 * Returns 0 on success, -1 on error. */
#if XCODEX_WINDOWS
int getWindowSize(int ifd, int ofd, int *rows, int *cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hOut == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return 0;
    }
    return -1;
}
#endif

#if XCODEX_POSIX
int getWindowSize(int ifd, int ofd, int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        /* ioctl() failed. Try to query the terminal itself. */
        int orig_row, orig_col, retval;

        /* Get the initial position so we can restore it later. */
        retval = getCursorPosition(ifd,ofd,&orig_row,&orig_col);
        if (retval == -1) goto failed;

        /* Go to right/bottom margin and get position. */
        if (write(ofd,"\x1b[999C\x1b[999B",12) != 12) goto failed;
        retval = getCursorPosition(ifd,ofd,rows,cols);
        if (retval == -1) goto failed;

        /* Restore position. */
        char seq[32];
        snprintf(seq,32,"\x1b[%d;%dH",orig_row,orig_col);
        if (write(ofd,seq,strlen(seq)) == -1) {
            /* Can't recover... */
        }
        return 0;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

failed:
    return -1;
}
#endif

/* ====================== Syntax highlight color scheme  ==================== */

int is_separator(int c) {
    return c == '\0' || isspace(c) || strchr(",.()+-/*=~%[];",c) != NULL;
}

/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
int editorRowHasOpenComment(erow *row) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < 2 || (row->render[row->rsize-2] != '*' ||
                            row->render[row->rsize-1] != '/'))) return 1;
    return 0;
}

/* Set every byte of row->hl (that corresponds to every character in the line)
 * to the right syntax highlight type (HL_* defines). */
void editorUpdateSyntax(erow *row) {
    row->hl = realloc(row->hl,row->rsize);
    memset(row->hl,HL_NORMAL,row->rsize);

    if (E.syntax == NULL) return;

    int i, prev_sep, in_string, in_comment, in_char;
    char *p;
    char **keywords = E.syntax->keywords;
    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    p = row->render;
    i = 0;
    while(*p && isspace(*p)) {
        p++;
        i++;
    }
    prev_sep = 1;
    in_string = 0;
    in_char = 0;
    in_comment = 0;

    if (row->idx > 0 && editorRowHasOpenComment(&E.row[row->idx-1]))
        in_comment = 1;

    while(*p) {
        /* Handle preprocessor directives */
        if (i == 0 && *p == '#') {
            while(*p && *p != '\n') {
                row->hl[i] = HL_PREPROCESSOR;
                p++; i++;
            }
            continue;
        }

        /* Handle single line comments */
        if (prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            memset(row->hl+i,HL_COMMENT,row->size-i);
            return;
        }

        /* Handle multi line comments */
        if (in_comment) {
            row->hl[i] = HL_MLCOMMENT;
            if (*p == mce[0] && *(p+1) == mce[1]) {
                row->hl[i+1] = HL_MLCOMMENT;
                p += 2; i += 2;
                in_comment = 0;
                prev_sep = 1;
                continue;
            } else {
                prev_sep = 0;
                p++; i++;
                continue;
            }
        } else if (*p == mcs[0] && *(p+1) == mcs[1]) {
            row->hl[i] = HL_MLCOMMENT;
            row->hl[i+1] = HL_MLCOMMENT;
            p += 2; i += 2;
            in_comment = 1;
            prev_sep = 0;
            continue;
        }

        /* Handle strings and character literals */
        if (in_string || in_char) {
            row->hl[i] = HL_STRING;
            if (*p == '\\') {
                row->hl[i+1] = HL_STRING;
                p += 2; i += 2;
                prev_sep = 0;
                continue;
            }
            if (*p == in_string || *p == in_char) {
                in_string = in_char = 0;
            }
            p++; i++;
            continue;
        } else {
            if (*p == '"') {
                in_string = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            } else if (*p == '\'') {
                in_char = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle operators */
        if (strchr("+-*/%=<>!&|^~?:", *p)) {
            row->hl[i] = HL_OPERATOR;
            p++; i++;
            prev_sep = 1;
            continue;
        }

        /* Handle brackets */
        if (strchr("(){}[]", *p)) {
            row->hl[i] = HL_BRACKET;
            p++; i++;
            prev_sep = 1;
            continue;
        }

        /* Handle non printable chars */
        if (!isprint(*p)) {
            row->hl[i] = HL_NONPRINT;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle numbers */
        if ((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) ||
            (*p == '.' && i >0 && row->hl[i-1] == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle function calls */
        if (prev_sep && isalpha(*p)) {
            int j = i;
            while (j < row->rsize && (isalnum(row->render[j]) || row->render[j] == '_')) {
                j++;
            }
            if (j < row->rsize && row->render[j] == '(') {
                memset(row->hl+i, HL_FUNCTION, j-i);
                p += (j-i);
                i = j;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle keywords */
        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen-1] == '|';
                int kw3 = kw2 && keywords[j][klen-2] == '|';
                
                if (kw3) klen -= 2;
                else if (kw2) klen--;

                if (!memcmp(p,keywords[j],klen) &&
                    is_separator(*(p+klen)))
                {
                    int hl_type = kw3 ? HL_KEYWORD3 : (kw2 ? HL_KEYWORD2 : HL_KEYWORD1);
                    memset(row->hl+i, hl_type, klen);
                    p += klen;
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(*p);
        p++; i++;
    }

    int oc = editorRowHasOpenComment(row);
    if (row->hl_oc != oc && row->idx+1 < E.numrows)
        editorUpdateSyntax(&E.row[row->idx+1]);
    row->hl_oc = oc;
}

/* Maps syntax highlight token types to terminal colors. */
void editorSelectSyntaxHighlight(char *filename) {
    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        struct editorSyntax *s = HLDB+j;
        unsigned int i = 0;
        while(s->filematch[i]) {
            char *p;
            int patlen = strlen(s->filematch[i]);
            if ((p = strstr(filename,s->filematch[i])) != NULL) {
                if (s->filematch[i][0] != '.' || p[patlen] == '\0') {
                    E.syntax = s;
                    return;
                }
            }
            i++;
        }
    }
}

/* Set terminal background color */
void editorSetBackgroundColor(int color) {
    if (color == -1) {
        /* Reset to default background */
        xcodex_write(STDOUT_FILENO, "\x1b[49m\x1b[2J\x1b[H", 12);
    } else {
        /* Set 256-color background and clear screen to apply it */
        char bg_buf[32];
        int len = snprintf(bg_buf, sizeof(bg_buf), "\x1b[48;5;%dm\x1b[2J\x1b[H", color);
        if (len > 0 && len < sizeof(bg_buf)) {
            xcodex_write(STDOUT_FILENO, bg_buf, len);
        }
        /* Force flush to ensure immediate application */
        fflush(stdout);
    }
}

/* Enhanced status bar with theme colors and mode display */
void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab,"\x1b[0K",4);
    
    /* Set status bar background */
    char status_bg[16];
    snprintf(status_bg, sizeof(status_bg), "\x1b[48;5;%dm", themes[current_theme].status_bg);
    abAppend(ab, status_bg, strlen(status_bg));
    
    /* Set status bar foreground */
    char status_fg[16];
    snprintf(status_fg, sizeof(status_fg), "\x1b[38;5;%dm", themes[current_theme].status_fg);
    abAppend(ab, status_fg, strlen(status_fg));

    char status[80], rstatus[80];
    
    /* Build status string with plugin info */
    char plugin_info[32] = "";
#ifdef XCODEX_ENABLE_LUA
    if (g_plugin_manager.count > 0) {
        snprintf(plugin_info, sizeof(plugin_info), " | Plugins: %d", g_plugin_manager.count);
    }
#endif

#ifdef XCODEX_ENABLE_COMPLETION
    char completion_info[16] = "";
    if (xcodex_completion_is_visible()) {
        snprintf(completion_info, sizeof(completion_info), " | Complete");
    }
#endif
    
    int len = snprintf(status, sizeof(status), " %.15s - %d lines %s | %s | Theme: %s | Line#: %s%s%s",
        E.filename ? E.filename : "[No Name]", E.numrows, 
        E.dirty ? "(modified)" : "", 
        xcodex_get_mode_name(E.mode),
        themes[current_theme].name,
        E.show_line_numbers ? "ON" : "OFF",
        plugin_info
#ifdef XCODEX_ENABLE_COMPLETION
        , completion_info
#else
        , ""
#endif
        );
    
    int rlen = snprintf(rstatus, sizeof(rstatus),
        "%d/%d ", E.rowoff+E.cy+1, E.numrows);
    
    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);
    
    while(len < E.screencols) {
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }
    abAppend(ab, "\x1b[0m\r\n", 6);
}

/* ======================= Editor rows implementation ======================= */

/* Update the rendered version and the syntax highlight of a row. */
void editorUpdateRow(erow *row) {
    if (!row) return;
    
    unsigned int tabs = 0, nonprint = 0;
    int j, idx;

/* Create a version of the row we can directly print on the screen,
     * respecting tabs, substituting non printable characters with '?'. */
    free(row->render);
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;

    unsigned long long allocsize =
        (unsigned long long) row->size + tabs*4 + nonprint*9 + 1;
    if (allocsize > UINT32_MAX) {
        printf("Some line of the edited file is too long for xcodex\n");
        exit(1);
    }

    row->render = malloc(row->size + tabs*4 + nonprint*9 + 1);
    if (!row->render) {
        printf("Out of memory!\n");
        exit(1);
    }
    idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == TAB) {
            row->render[idx++] = ' ';
            while(idx % 4 != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->rsize = idx;
    row->render[idx] = '\0';
    
    /* Update the syntax highlighting attributes of the row. */
    editorUpdateSyntax(row);
}

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
void editorInsertRow(int at, char *s, size_t len) {
    if (at < 0 || at > E.numrows || !s) return;
    
    /* Track undo for line insertion */
    xcodex_push_undo(UNDO_INSERT_LINE, at, 0, NULL, 0);
    
    E.row = realloc(E.row,sizeof(erow)*(E.numrows+1));
    if (!E.row) {
        printf("Out of memory!\n");
        exit(1);
    }
    if (at != E.numrows) {
        memmove(E.row+at+1,E.row+at,sizeof(E.row[0])*(E.numrows-at));
        for (int j = at+1; j <= E.numrows; j++) E.row[j].idx++;
    }
    E.row[at].size = len;
    E.row[at].chars = malloc(len+1);
    if (!E.row[at].chars) {
        printf("Out of memory!\n");
        exit(1);
    }
    memcpy(E.row[at].chars,s,len);
    E.row[at].chars[len] = '\0';
    E.row[at].hl = NULL;
    E.row[at].hl_oc = 0;
    E.row[at].render = NULL;
    E.row[at].rsize = 0;
    E.row[at].idx = at;
    editorUpdateRow(E.row+at);
    E.numrows++;
    editorUpdateLineNumberWidth(); /* Update line number width */
    E.dirty++;
}

/* Free row's heap allocated stuff. */
void editorFreeRow(erow *row) {
    if (row) {
        free(row->render);
        free(row->chars);
        free(row->hl);
        row->render = NULL;
        row->chars = NULL;
        row->hl = NULL;
        row->size = 0;
        row->rsize = 0;
    }
}

/* Remove the row at the specified position, shifting the remainign on the
 * top. */
void editorDelRow(int at) {
    erow *row;

    if (at < 0 || at >= E.numrows) return;
    row = E.row+at;
    
    /* Track undo for line deletion */
    xcodex_push_undo(UNDO_DELETE_LINE, at, 0, row->chars, row->size);
    
    editorFreeRow(row);
    memmove(E.row+at,E.row+at+1,sizeof(E.row[0])*(E.numrows-at-1));
    for (int j = at; j < E.numrows-1; j++) E.row[j].idx++;
    E.numrows--;
    editorUpdateLineNumberWidth(); /* Update line number width */
    E.dirty++;
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, escluding
 * the final nulterm. */
char *editorRowsToString(int *buflen) {
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    if (!buflen) return NULL;

    /* Compute count of bytes */
    for (j = 0; j < E.numrows; j++) {
        if (E.row[j].size >= 0) {
            totlen += E.row[j].size+1; /* +1 is for "\n" at end of every row */
        }
    }
    *buflen = totlen;
    
    if (totlen == 0) {
        /* Empty file */
        buf = malloc(1);
        if (buf) buf[0] = '\0';
        return buf;
    }
    
    totlen++; /* Also make space for nulterm */

    p = buf = malloc(totlen);
    if (!buf) {
        *buflen = 0;
        return NULL;
    }
    
    for (j = 0; j < E.numrows; j++) {
        if (E.row[j].chars && E.row[j].size > 0) {
            memcpy(p,E.row[j].chars,E.row[j].size);
            p += E.row[j].size;
        }
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
void editorRowInsertChar(erow *row, int at, int c) {
    if (!row || at < 0) return;
    
    if (at > row->size) {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = at-row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars,row->size+padlen+2);
        if (!row->chars) {
            printf("Out of memory!\n");
            exit(1);
        }
        memset(row->chars+row->size,' ',padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars,row->size+2);
        if (!row->chars) {
            printf("Out of memory!\n");
            exit(1);
        }
        memmove(row->chars+at+1,row->chars+at,row->size-at+1);
        row->size++;
    }
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

/* Append the string 's' at the end of a row */
void editorRowAppendString(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars,row->size+len+1);
    memcpy(row->chars+row->size,s,len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

/* Delete the character at offset 'at' from the specified row. */
void editorRowDelChar(erow *row, int at) {
    if (!row || at < 0 || row->size <= at) return;
    memmove(row->chars+at,row->chars+at+1,row->size-at);
    editorUpdateRow(row);
    row->size--;
    E.dirty++;
}

/* Insert the specified char at the current prompt position. */
void editorInsertChar(int c) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    int effective_screencols = E.screencols - E.line_numbers_width;

    /* If the row where the cursor is currently located does not exist in our
     * logical representaion of the file, add enough empty rows as needed. */
    if (!row) {
        while(E.numrows <= filerow)
            editorInsertRow(E.numrows,"",0);
    }
    row = &E.row[filerow];
    
    /* Track undo for character insertion */
    xcodex_push_undo(UNDO_INSERT_CHAR, filerow, filecol, NULL, 0);
    
    editorRowInsertChar(row,filecol,c);
    if (E.cx == effective_screencols-1)
        E.coloff++;
    else
        E.cx++;
    E.dirty++;
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row) {
        if (filerow == E.numrows) {
            editorInsertRow(filerow,"",0);
            goto fixcursor;
        }
        return;
    }
    /* If the cursor is over the current line size, we want to conceptually
     * think it's just over the last character. */
    if (filecol >= row->size) filecol = row->size;
    if (filecol == 0) {
        editorInsertRow(filerow,"",0);
    } else {
        /* We are in the middle of a line. Split it between two rows. */
        /* Track undo for line split operation */
        xcodex_push_undo(UNDO_SPLIT_LINE, filerow, filecol, NULL, 0);
        
        editorInsertRow(filerow+1,row->chars+filecol,row->size-filecol);
        row = &E.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        editorUpdateRow(row);
    }
fixcursor:
    if (E.cy == E.screenrows-1) {
        E.rowoff++;
    } else {
        E.cy++;
    }
    E.cx = 0;
    E.coloff = 0;
}

/* Delete the char at the current prompt position. */
void editorDelChar(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row || (filecol == 0 && filerow == 0)) return;
    if (filecol == 0) {
        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        
        /* Track undo for line join operation */
        xcodex_push_undo(UNDO_JOIN_LINE, filerow-1, E.row[filerow-1].size, 
                        row->chars, row->size);
        
        filecol = E.row[filerow-1].size;
        editorRowAppendString(&E.row[filerow-1],row->chars,row->size);
        editorDelRow(filerow);
        row = NULL;
        if (E.cy == 0)
            E.rowoff--;
        else
            E.cy--;
        E.cx = filecol;
        if (E.cx >= E.screencols) {
            int shift = (E.screencols-E.cx)+1;
            E.cx -= shift;
            E.coloff += shift;
        }
    } else {
        /* Track undo for character deletion */
        char deleted_char = row->chars[filecol-1];
        xcodex_push_undo(UNDO_DELETE_CHAR, filerow, filecol-1, &deleted_char, 1);
        
        editorRowDelChar(row,filecol-1);
        if (E.cx == 0 && E.coloff)
            E.coloff--;
        else
            E.cx--;
    }
    if (row) editorUpdateRow(row);
    E.dirty++;
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpen(char *filename) {
    FILE *fp;

    if (!filename) {
        editorSetStatusMessage("No filename provided");
        return 1;
    }

    E.dirty = 0;
    free(E.filename);
    size_t fnlen = strlen(filename)+1;
    E.filename = malloc(fnlen);
    if (!E.filename) {
        editorSetStatusMessage("Out of memory");
        return 1;
    }
    memcpy(E.filename,filename,fnlen);

    fp = fopen(filename,"r");
    if (!fp) {
        if (errno != ENOENT) {
            perror("Opening file");
            exit(1);
        }
        editorSetStatusMessage("New file: %s", filename);
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int line_count = 0;
    
    while((linelen = getline(&line,&linecap,fp)) != -1) {
        if (linelen && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            line[--linelen] = '\0';
        editorInsertRow(E.numrows,line,linelen);
        line_count++;
        
        /* Prevent loading extremely large files that might cause memory issues */
        if (line_count > 100000) {
            editorSetStatusMessage("File too large (>100k lines), truncated");
            break;
        }
    }
    
    free(line);
    fclose(fp);
    E.dirty = 0;
    editorSetStatusMessage("Loaded %d lines from %s", line_count, filename);
    
    /* Auto-start LSP server for this file type */
#ifdef XCODEX_ENABLE_LSP
    if (xcodex_lsp_auto_start_for_file(filename) == 0) {
        /* LSP server started, send document open notification */
        lsp_server_t *server = xcodex_lsp_get_server_for_file(filename);
        if (server && server->initialized) {
            char *uri = xcodex_lsp_file_to_uri(filename);
            char *content = editorRowsToString(NULL);
            const char *language = "c"; /* Default, could be detected from extension */
            xcodex_lsp_send_text_document_open(server, uri, language, content);
            free(uri);
            free(content);
        }
    }
#endif

    /* Notify plugins about file open */
#ifdef XCODEX_ENABLE_LUA
    char hook_args[512];
    snprintf(hook_args, sizeof(hook_args), "\"%s\"", filename);
    xcodex_lua_call_hook(XCODEX_HOOK_ON_FILE_OPEN, hook_args);
#endif
    
    return 0;
}

/* Save the current file on disk. Return 0 on success, 1 on error. */
int editorSave(void) {
    if (!E.filename) {
        editorSetStatusMessage("No filename specified");
        return 1;
    }
    
    int len;
    char *buf = editorRowsToString(&len);
    if (!buf) {
        editorSetStatusMessage("Failed to serialize file content");
        return 1;
    }
    
    int fd = open(E.filename,O_RDWR|O_CREAT,0644);
    if (fd == -1) goto writeerr;

    /* Use truncate + a single write(2) call in order to make saving
     * a bit safer, under the limits of what we can do in a small editor. */
    if (ftruncate(fd,len) == -1) goto writeerr;
    
    ssize_t written = write(fd,buf,len);
    if (written != len) goto writeerr;

    close(fd);
    free(buf);
    E.dirty = 0;
    editorSetStatusMessage("%d bytes written to %s", len, E.filename);
    return 0;

writeerr:
    free(buf);
    if (fd != -1) close(fd);
    editorSetStatusMessage("Can't save! I/O error: %s",strerror(errno));
    return 1;
}

/* ============================= Terminal update ============================ */

#define ABUF_INIT {NULL,0}

void abAppend(struct abuf *ab, const char *s, int len) {
    if (!ab || !s || len <= 0) return;
    
    char *new = realloc(ab->b,ab->len+len);
    if (new == NULL) {
        /* Out of memory - could not append */
        return;
    }
    
    memcpy(new+ab->len,s,len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    if (ab) {
        free(ab->b);
        ab->b = NULL;
        ab->len = 0;
    }
}

/* This function writes the whole screen using VT100 escape characters
 * starting from the logical state of the editor in the global state 'E'. */
/* Check if a position is within visual selection */
int xcodex_is_in_visual_selection(int file_row, int file_col) {
    if (E.mode != XCODEX_MODE_VISUAL && E.mode != XCODEX_MODE_VISUAL_LINE && E.mode != XCODEX_MODE_VISUAL_BLOCK) {
        return 0;
    }
    
    /* Convert screen coordinates to file coordinates */
    int start_row = E.rowoff + E.visual_start_row;
    int start_col = E.coloff + E.visual_start_col;
    int end_row = E.rowoff + E.visual_end_row;
    int end_col = E.coloff + E.visual_end_col;
    
    /* Normalize selection (start should be before end) */
    if (start_row > end_row || (start_row == end_row && start_col > end_col)) {
        int temp = start_row;
        start_row = end_row;
        end_row = temp;
        temp = start_col;
        start_col = end_col;
        end_col = temp;
    }
    
    if (E.mode == XCODEX_MODE_VISUAL_LINE) {
        /* Line-wise selection */
        return (file_row >= start_row && file_row <= end_row);
    } else if (E.mode == XCODEX_MODE_VISUAL_BLOCK) {
        /* Block-wise selection */
        return (file_row >= start_row && file_row <= end_row && 
                file_col >= start_col && file_col <= end_col);
    } else {
        /* Character-wise selection */
        if (file_row < start_row || file_row > end_row) {
            return 0;
        }
        if (file_row == start_row && file_row == end_row) {
            return (file_col >= start_col && file_col <= end_col);
        }
        if (file_row == start_row) {
            return (file_col >= start_col);
        }
        if (file_row == end_row) {
            return (file_col <= end_col);
        }
        return 1;  /* Middle rows are fully selected */
    }
}

void editorRefreshScreen(void) {
    int y;
    erow *r;
    char buf[32];
    struct abuf ab = ABUF_INIT;

    abAppend(&ab,"\x1b[?25l",6); /* Hide cursor. */
    
    /* Apply background color at start of refresh and clear entire screen */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        char bg_color[16];
        int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
        if (bg_len > 0 && bg_len < sizeof(bg_color)) {
            abAppend(&ab, bg_color, bg_len);
        }
    }
    
    /* Clear entire screen with background color, then go home */
    abAppend(&ab,"\x1b[2J",4); /* Clear entire screen */
    abAppend(&ab,"\x1b[H",3); /* Go home. */
    for (y = 0; y < E.screenrows; y++) {
        int filerow = E.rowoff+y;

        if (filerow >= E.numrows) {
            /* Check if this empty line is the cursor line */
            int is_cursor_line = (filerow == E.rowoff + E.cy);
            
            /* Draw line numbers for empty lines */
            if (E.show_line_numbers) {
                char line_num[16];
                int line_num_len = snprintf(line_num, sizeof(line_num), 
                    "%*s ", E.line_numbers_width - 1, "");
                
                /* Set line number color from theme - with bounds checking */
                if (current_theme >= 0 && current_theme < NUM_THEMES) {
                    char line_color[16];
                    int color_len = snprintf(line_color, sizeof(line_color), "\x1b[38;5;%dm", themes[current_theme].line_number_color);
                    if (color_len > 0 && color_len < sizeof(line_color)) {
                        abAppend(&ab, line_color, color_len);
                    }
                }
                abAppend(&ab, line_num, line_num_len);
                abAppend(&ab, "\x1b[39m", 5); /* Reset color */
            }
            
            /* Apply cursor line background for empty lines - DISABLED
            if (is_cursor_line && current_theme >= 0 && current_theme < NUM_THEMES) {
                char cursor_bg[16];
                int bg_len = snprintf(cursor_bg, sizeof(cursor_bg), "\x1b[48;5;%dm", themes[current_theme].cursor_line_color);
                if (bg_len > 0 && bg_len < sizeof(cursor_bg)) {
                    abAppend(&ab, cursor_bg, bg_len);
                }
            }
            */
            
            if (E.numrows == 0 && y == E.screenrows/3) {
                char welcome[80];
                int welcomelen = snprintf(welcome,sizeof(welcome),
                    "XCodex editor -- version %s -- Theme: %s", 
                    XCODEX_VERSION, themes[current_theme].name);
                int effective_cols = E.screencols - E.line_numbers_width;
                int padding = (effective_cols - welcomelen)/2;
                if (padding) {
                    abAppend(&ab,"~",1);
                    padding--;
                }
                while(padding--) abAppend(&ab," ",1);
                abAppend(&ab,welcome,welcomelen);
                
                /* Fill remaining space for cursor line if needed - DISABLED
                if (is_cursor_line) {
                    int chars_used = 1 + (padding > 0 ? padding : 0) + welcomelen; // ~ + padding + welcome
                    int remaining = effective_cols - chars_used;
                    for (int i = 0; i < remaining; i++) {
                        abAppend(&ab, " ", 1);
                    }
                }
                */
            } else {
                abAppend(&ab,"~",1);
                
                /* Fill remaining space for cursor line if needed - DISABLED
                if (is_cursor_line) {
                    int effective_cols = E.screencols - E.line_numbers_width;
                    for (int i = 1; i < effective_cols; i++) { // Start from 1 since ~ is already added
                        abAppend(&ab, " ", 1);
                    }
                }
                */
            }
            
            /* Reset colors and clear to end of line */
            abAppend(&ab,"\x1b[39m",5);  /* Reset foreground */
            
            /* Reapply theme background color before clearing */
            if (current_theme >= 0 && current_theme < NUM_THEMES) {
                char bg_color[16];
                int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
                if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                    abAppend(&ab, bg_color, bg_len);
                }
            }
            
            abAppend(&ab,"\x1b[0K",4);   /* Clear to end of line */
            abAppend(&ab,"\r\n",2);
            continue;
        }

        r = &E.row[filerow];

        /* Draw line numbers */
        if (E.show_line_numbers) {
            char line_num[16];
            int current_line = filerow + 1;
            int line_num_len = snprintf(line_num, sizeof(line_num), 
                "%*d ", E.line_numbers_width - 1, current_line);
            
            /* Use theme-specific color for line numbers - with bounds checking */
            if (current_theme >= 0 && current_theme < NUM_THEMES) {
                char line_color[16];
                int color_len = snprintf(line_color, sizeof(line_color), "\x1b[38;5;%dm", themes[current_theme].line_number_color);
                if (color_len > 0 && color_len < sizeof(line_color)) {
                    abAppend(&ab, line_color, color_len);
                }
            }
            abAppend(&ab, line_num, line_num_len);
            abAppend(&ab, "\x1b[0m", 4); /* Reset formatting */
        }

        /* Apply cursor line background color if this is the current line */
        // int is_cursor_line = (filerow == E.rowoff + E.cy);
        /* Temporarily disabled cursor line highlighting to fix background color consistency
        if (is_cursor_line && current_theme >= 0 && current_theme < NUM_THEMES) {
            char cursor_bg[16];
            int bg_len = snprintf(cursor_bg, sizeof(cursor_bg), "\x1b[48;5;%dm", themes[current_theme].cursor_line_color);
            if (bg_len > 0 && bg_len < sizeof(cursor_bg)) {
                abAppend(&ab, cursor_bg, bg_len);
            }
        }
        */

        /* Apply theme background color at the start of each line */
        if (current_theme >= 0 && current_theme < NUM_THEMES) {
            char bg_color[16];
            int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
            if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                abAppend(&ab, bg_color, bg_len);
            }
        }

        int len = r->rsize - E.coloff;
        int effective_screencols = E.screencols - E.line_numbers_width;
        int current_color = -1;
        if (len > 0) {
            if (len > effective_screencols) len = effective_screencols;
            char *c = r->render+E.coloff;
            unsigned char *hl = r->hl+E.coloff;
            int j;
            for (j = 0; j < len; j++) {
                int file_col = E.coloff + j;
                int is_selected = xcodex_is_in_visual_selection(filerow, file_col);
                
                if (hl[j] == HL_NONPRINT) {
                    char sym;
                    if (is_selected) {
                        abAppend(&ab,"\x1b[7m",4);  /* Reverse video for selection */
                    } else {
                        abAppend(&ab,"\x1b[7m",4);
                    }
                    if (c[j] <= 26)
                        sym = '@'+c[j];
                    else
                        sym = '?';
                    abAppend(&ab,&sym,1);
                    abAppend(&ab,"\x1b[0m",4);
                    /* Restore theme background after reset */
                    if (current_theme >= 0 && current_theme < NUM_THEMES) {
                        char bg_color[16];
                        int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
                        if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                            abAppend(&ab, bg_color, bg_len);
                        }
                    }
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        abAppend(&ab,"\x1b[39m",5);  /* Reset foreground only */
                        current_color = -1;
                        /* Reapply background color after foreground reset */
                        if (current_theme >= 0 && current_theme < NUM_THEMES) {
                            char bg_color[16];
                            int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
                            if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                                abAppend(&ab, bg_color, bg_len);
                            }
                        }
                    }
                    
                    /* Apply selection highlight */
                    if (is_selected) {
                        abAppend(&ab,"\x1b[7m",4);  /* Reverse video for selection */
                    }
                    
                    abAppend(&ab,c+j,1);
                    
                    /* Reset selection highlight */
                    if (is_selected) {
                        abAppend(&ab,"\x1b[0m",4);  /* Reset all formatting */
                        /* Reapply background color */
                        if (current_theme >= 0 && current_theme < NUM_THEMES) {
                            char bg_color[16];
                            int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
                            if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                                abAppend(&ab, bg_color, bg_len);
                            }
                        }
                    }
                } else {
                    int color = editorSyntaxToColor(hl[j]);
                    if (color != current_color) {
                        char buf[32];
                        int clen = snprintf(buf,sizeof(buf),"\x1b[38;5;%dm",color);
                        current_color = color;
                        abAppend(&ab,buf,clen);
                    }
                    
                    /* Apply selection highlight */
                    if (is_selected) {
                        abAppend(&ab,"\x1b[7m",4);  /* Reverse video for selection */
                    }
                    
                    abAppend(&ab,c+j,1);
                    
                    /* Reset selection highlight and restore color */
                    if (is_selected) {
                        abAppend(&ab,"\x1b[0m",4);  /* Reset all formatting */
                        /* Reapply background color */
                        if (current_theme >= 0 && current_theme < NUM_THEMES) {
                            char bg_color[16];
                            int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
                            if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                                abAppend(&ab, bg_color, bg_len);
                            }
                        }
                        /* Reapply syntax color */
                        if (color != -1) {
                            char buf[32];
                            int clen = snprintf(buf,sizeof(buf),"\x1b[38;5;%dm",color);
                            abAppend(&ab,buf,clen);
                        }
                    }
                }
            }
        }
        
        /* Handle cursor line background extending to end of line - DISABLED
        if (is_cursor_line) {
            int chars_drawn = len > 0 ? (len > effective_screencols ? effective_screencols : len) : 0;
            int remaining_space = effective_screencols - chars_drawn;
            for (int i = 0; i < remaining_space; i++) {
                abAppend(&ab, " ", 1);
            }
        }
        */
        
        abAppend(&ab,"\x1b[39m",5);  /* Reset foreground color */
        
        /* Reapply background color before clearing to end of line */
        if (current_theme >= 0 && current_theme < NUM_THEMES) {
            char bg_color[16];
            int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
            if (bg_len > 0 && bg_len < sizeof(bg_color)) {
                abAppend(&ab, bg_color, bg_len);
            }
        }
        
        abAppend(&ab,"\x1b[0K",4);
        abAppend(&ab,"\r\n",2);
    }

    /* Use the new enhanced status bar function */
    editorDrawStatusBar(&ab);

    /* Second row depends on E.statusmsg and the status message update time. */
    /* Reapply background color before clearing status message line */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        char bg_color[16];
        int bg_len = snprintf(bg_color, sizeof(bg_color), "\x1b[48;5;%dm", themes[current_theme].bg_color);
        if (bg_len > 0 && bg_len < sizeof(bg_color)) {
            abAppend(&ab, bg_color, bg_len);
        }
    }
    abAppend(&ab,"\x1b[0K",4);
    int msglen = strlen(E.statusmsg);
    /* Show status message longer in command mode, or within 5 seconds otherwise */
    if (msglen && (E.mode == XCODEX_MODE_COMMAND || time(NULL)-E.statusmsg_time < 5))
        abAppend(&ab,E.statusmsg,msglen <= E.screencols ? msglen : E.screencols);

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int cx = 1; /* Start at position 1 */
    
    /* Add line number width offset if line numbers are enabled */
    if (E.show_line_numbers) {
        cx += E.line_numbers_width;
    }
    
    int filerow = E.rowoff+E.cy;
    erow *row = (filerow >= 0 && filerow < E.numrows) ? &E.row[filerow] : NULL;
    if (row) {
        int rx = 0;
        int j;
        for (j = 0; j < E.cx && j < row->size; j++) {
            if (row->chars[j] == TAB) {
                rx += (4 - (rx % 4));
            } else {
                rx++;
            }
        }
        cx += rx;
    } else {
        cx += E.cx;
    }
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy+1,cx);
    abAppend(&ab,buf,strlen(buf));
    
    /* Draw completion popup if visible */
#ifdef XCODEX_ENABLE_COMPLETION
    if (xcodex_completion_is_visible()) {
        xcodex_completion_draw(&ab);
        /* Restore cursor position after drawing popup */
        snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy+1,cx);
        abAppend(&ab,buf,strlen(buf));
    }
#endif
    
    abAppend(&ab,"\x1b[?25h",6); /* Show cursor. */
    xcodex_write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}

/* Set an editor status message for the second line of the status, at the
 * end of the screen. */
void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(E.statusmsg,sizeof(E.statusmsg),fmt,ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/* =============================== Find mode ================================ */

#define XCODEX_QUERY_LEN 256

void editorFind(int fd) {
    char query[XCODEX_QUERY_LEN+1] = {0};
    int qlen = 0;
    int last_match = -1; /* Last line where a match was found. -1 for none. */
    int find_next = 0; /* if 1 search next, if -1 search prev. */
    int saved_hl_line = -1;  /* No saved HL */
    char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
    if (saved_hl) { \
        memcpy(E.row[saved_hl_line].hl,saved_hl, E.row[saved_hl_line].rsize); \
        free(saved_hl); \
        saved_hl = NULL; \
    } \
} while (0)

    /* Save the cursor position in order to restore it later. */
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;

    while(1) {
        editorSetStatusMessage(
            "Search: %s (Use ESC/Arrows/Enter)", query);
        editorRefreshScreen();

        int c = editorReadKey(fd);
        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE) {
            if (qlen != 0) query[--qlen] = '\0';
            last_match = -1;
        } else if (c == ESC || c == ENTER) {
            if (c == ESC) {
                E.cx = saved_cx; E.cy = saved_cy;
                E.coloff = saved_coloff; E.rowoff = saved_rowoff;
            }
            FIND_RESTORE_HL;
            editorSetStatusMessage("");
            return;
        } else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
            find_next = 1;
        } else if (c == ARROW_LEFT || c == ARROW_UP) {
            find_next = -1;
        } else if (isprint(c)) {
            if (qlen < XCODEX_QUERY_LEN) {
                query[qlen++] = c;
                query[qlen] = '\0';
                last_match = -1;
            }
        }

        /* Search occurrence. */
        if (last_match == -1) find_next = 1;
        if (find_next) {
            char *match = NULL;
            int match_offset = 0;
            int i, current = last_match;

            for (i = 0; i < E.numrows; i++) {
                current += find_next;
                if (current == -1) current = E.numrows-1;
                else if (current == E.numrows) current = 0;
                match = strstr(E.row[current].render,query);
                if (match) {
                    match_offset = match-E.row[current].render;
                    break;
                }
            }
            find_next = 0;

            /* Highlight */
            FIND_RESTORE_HL;

            if (match) {
                erow *row = &E.row[current];
                last_match = current;
                if (row->hl) {
                    saved_hl_line = current;
                    saved_hl = malloc(row->rsize);
                    memcpy(saved_hl,row->hl,row->rsize);
                    memset(row->hl+match_offset,HL_MATCH,qlen);
                }
                E.cy = 0;
                E.cx = match_offset;
                E.rowoff = current;
                E.coloff = 0;
                /* Scroll horizontally as needed. */
                if (E.cx > E.screencols) {
                    int diff = E.cx - E.screencols;
                    E.cx -= diff;
                    E.coloff += diff;
                }
            }
        }
    }
}

/* ========================= Editor events handling  ======================== */

/* Handle cursor position change because arrow keys were pressed. */
void editorMoveCursor(int key) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    int rowlen;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    int effective_screencols = E.screencols - E.line_numbers_width;

    switch(key) {
    case ARROW_LEFT:
        if (E.cx == 0) {
            if (E.coloff) {
                E.coloff--;
            } else {
                if (filerow > 0) {
                    E.cy--;
                    E.cx = E.row[filerow-1].size;
                    if (E.cx > effective_screencols-1) {
                        E.coloff = E.cx-effective_screencols+1;
                        E.cx = effective_screencols-1;
                    }
                }
            }
        } else {
            E.cx -= 1;
        }
        break;
    case ARROW_RIGHT:
        if (row && filecol < row->size) {
            if (E.cx == effective_screencols-1) {
                E.coloff++;
            } else {
                E.cx += 1;
            }
        } else if (row && filecol == row->size) {
            E.cx = 0;
            E.coloff = 0;
            if (E.cy == E.screenrows-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    case ARROW_UP:
        if (E.cy == 0) {
            if (E.rowoff) E.rowoff--;
        } else {
            E.cy -= 1;
        }
        break;
    case ARROW_DOWN:
        if (filerow < E.numrows) {
            if (E.cy == E.screenrows-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    }
    /* Fix cx if the current line has not enough chars. */
    filerow = E.rowoff+E.cy;
    filecol = E.coloff+E.cx;
    row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    rowlen = row ? row->size : 0;
    if (filecol > rowlen) {
        E.cx -= filecol-rowlen;
        if (E.cx < 0) {
            E.coloff += E.cx;
            E.cx = 0;
        }
    }
}

/* Process events arriving from the standard input, which is, the user
 * is typing stuff on the terminal. */
#define XCODEX_QUIT_TIMES 3

/* ============================ XCodex Modal Key Processing ============================ */

/* Forward declarations */
void xcodex_process_normal_mode(int c, int *quit_times, int fd);
void xcodex_process_insert_mode(int c, int *quit_times);
void xcodex_process_visual_mode(int c);
void xcodex_process_command_mode(int c);

void editorProcessKeypress(int fd) {
    /* When the file is modified, requires Ctrl-q to be pressed N times
     * before actually quitting. */
    static int quit_times = XCODEX_QUIT_TIMES;

    int c = editorReadKey(fd);
    
    /* Route key to appropriate mode handler */
    switch (E.mode) {
        case XCODEX_MODE_NORMAL:
            xcodex_process_normal_mode(c, &quit_times, fd);
            break;
        case XCODEX_MODE_INSERT:
            xcodex_process_insert_mode(c, &quit_times);
            break;
        case XCODEX_MODE_VISUAL:
        case XCODEX_MODE_VISUAL_LINE:
        case XCODEX_MODE_VISUAL_BLOCK:
            xcodex_process_visual_mode(c);
            break;
        case XCODEX_MODE_COMMAND:
            xcodex_process_command_mode(c);
            break;
    }
}

/* Process keys in NORMAL mode - Navigation and commands */
void xcodex_process_normal_mode(int c, int *quit_times, int fd) {
    /* Handle number prefixes for motions (e.g., 5j) */
    if (isdigit(c) && (c != '0' || E.motion_count > 0)) {
        /* Prevent integer overflow */
        if (E.motion_count < 1000) {
            E.motion_count = E.motion_count * 10 + (c - '0');
            editorSetStatusMessage("-- NORMAL -- %d", E.motion_count);
        }
        return;
    }
    
    int count = E.motion_count > 0 ? E.motion_count : 1;
    /* Limit count to prevent excessive operations */
    if (count > 1000) count = 1000;
    E.motion_count = 0;
    
    switch (c) {
        /* ==== Mode Switching ==== */
        case 'i':  /* Insert before cursor */
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'I':  /* Insert at beginning of line */
            xcodex_move_line_start();
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'a':  /* Insert after cursor */
            if (E.cx < E.screencols - E.line_numbers_width - 1) {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    int filecol = E.coloff + E.cx;
                    if (filecol < row->size) {
                        E.cx++;
                    }
                }
            }
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'A':  /* Insert at end of line */
            xcodex_move_line_end();
            if (E.cx < E.screencols - E.line_numbers_width - 1) {
                E.cx++;
            }
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'o':  /* Open new line below */
            xcodex_move_line_end();
            editorInsertNewline();
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'O':  /* Open new line above */
            xcodex_move_line_start();
            editorInsertNewline();
            editorMoveCursor(ARROW_UP);
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;
        case 'v':  /* Visual mode */
            xcodex_set_mode(XCODEX_MODE_VISUAL);
            break;
        case 'V':  /* Visual line mode */
            xcodex_set_mode(XCODEX_MODE_VISUAL_LINE);
            break;
        case CTRL_V:  /* Visual block mode */
            xcodex_set_mode(XCODEX_MODE_VISUAL_BLOCK);
            break;
        case ':':  /* Command mode */
            xcodex_set_mode(XCODEX_MODE_COMMAND);
            break;
            
        /* ==== Basic Movement (hjkl) ==== */
        case 'h':
        case ARROW_LEFT:
            for (int i = 0; i < count; i++) {
                editorMoveCursor(ARROW_LEFT);
            }
            break;
        case 'j':
        case ARROW_DOWN:
            for (int i = 0; i < count; i++) {
                editorMoveCursor(ARROW_DOWN);
            }
            break;
        case 'k':
        case ARROW_UP:
            for (int i = 0; i < count; i++) {
                editorMoveCursor(ARROW_UP);
            }
            break;
        case 'l':
        case ARROW_RIGHT:
            for (int i = 0; i < count; i++) {
                editorMoveCursor(ARROW_RIGHT);
            }
            break;
            
        /* ==== Word Movement ==== */
        case 'w':  /* Next word */
            xcodex_move_word_forward(count);
            break;
        case 'b':  /* Previous word */
            xcodex_move_word_backward(count);
            break;
            
        /* ==== Line Movement ==== */
        case '0':  /* Start of line */
            xcodex_move_line_start();
            break;
        case '$':  /* End of line */
            xcodex_move_line_end();
            break;
            
        /* ==== File Movement ==== */
        case 'G':  /* Go to line (or last line if no count) */
            if (count == 1) {  /* No prefix number given */
                xcodex_go_to_last_line();
            } else {
                xcodex_go_to_line(count);
            }
            break;
        case 'g':  /* Handle gg command */
            if (E.command_len == 0) {
                E.command_buffer[0] = 'g';
                E.command_len = 1;
                E.command_buffer[1] = '\0';  /* Ensure null termination */
                editorSetStatusMessage("-- NORMAL -- g");
                return;
            } else if (E.command_len == 1 && E.command_buffer[0] == 'g') {
                xcodex_go_to_first_line();
                E.command_len = 0;
                E.command_buffer[0] = '\0';
            }
            break;
            
        /* ==== Editing Commands ==== */
        case 'x':  /* Delete character */
            xcodex_start_undo_group();
            for (int i = 0; i < count; i++) {
                int filerow = E.rowoff + E.cy;
                int filecol = E.coloff + E.cx;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    if (filecol >= 0 && filecol < row->size) {
                        editorRowDelChar(row, filecol);
                    }
                }
            }
            break;
        case 'd':  /* Delete line (dd) */
            if (E.command_len == 0) {
                E.command_buffer[0] = 'd';
                E.command_len = 1;
                E.command_buffer[1] = '\0';
                editorSetStatusMessage("-- NORMAL -- d");
                return;
            } else if (E.command_len == 1 && E.command_buffer[0] == 'd') {
                /* dd - Delete current line */
                xcodex_start_undo_group();
                for (int i = 0; i < count; i++) {
                    xcodex_delete_current_line();
                }
                E.command_len = 0;
                E.command_buffer[0] = '\0';
            }
            break;
        case 'X':  /* Delete character before cursor */
            xcodex_start_undo_group();
            for (int i = 0; i < count; i++) {
                editorDelChar();
            }
            break;
            
        /* ==== Yank and Paste ==== */
        case 'p':  /* Paste after cursor */
            xcodex_start_undo_group();
            for (int i = 0; i < count; i++) {
                xcodex_paste_text();
            }
            break;
        case 'P':  /* Paste before cursor */
            xcodex_start_undo_group();
            for (int i = 0; i < count; i++) {
                /* Move cursor to paste position */
                if (E.yank_is_line_mode) {
                    /* For line mode, paste before current line */
                    xcodex_move_line_start();
                    if (E.cy > 0) {
                        E.cy--;
                    } else if (E.rowoff > 0) {
                        E.rowoff--;
                    }
                } else {
                    /* For character mode, paste before cursor */
                    if (E.cx > 0) {
                        E.cx--;
                    } else if (E.coloff > 0) {
                        E.coloff--;
                    }
                }
                xcodex_paste_text();
            }
            break;
            
        /* ==== Undo ==== */
        case 'u':  /* Undo */
            xcodex_undo();
            break;
            
        /* ==== File Operations ==== */
        case CTRL_S:  /* Save file */
            editorSave();
            break;
        case CTRL_Q:  /* Quit */
            if (E.dirty && *quit_times) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                    "Press Ctrl-Q %d more times to quit.", *quit_times);
                (*quit_times)--;
                return;
            }
            E.quit_requested = 1;
            break;
            
        /* ==== Search ==== */
        case '/':  /* Search forward */
        case CTRL_F:
            editorFind(fd);
            break;
            
        /* ==== Display ==== */
        case CTRL_T:  /* Cycle themes */
            editorCycleTheme();
            break;
        case CTRL_N:  /* Toggle line numbers */
            editorToggleLineNumbers();
            break;
            
        case CTRL_L:  /* Clear screen (refresh) */
            /* Just refresh the screen as side effect */
            break;
            
        case CTRL_R:  /* Reload configuration */
            {
                // Reload configuration files
                config_free(&xcodex_config);
                config_init(&xcodex_config);
                config_load_defaults(&xcodex_config, "xcodex");
                
                // Try to load from user's home directory first
                char config_path[512];
                const char *home = getenv("HOME");
                if (!home) {
#ifdef _WIN32
                    home = getenv("USERPROFILE");
#endif
                }
                
                if (home) {
#ifdef _WIN32
                    snprintf(config_path, sizeof(config_path), "%s\\%s", home, XCODEX_CONFIG_FILE);
#else
                    snprintf(config_path, sizeof(config_path), "%s/%s", home, XCODEX_CONFIG_FILE);
#endif
                    // Try home directory first, fallback to current directory
                    if (access(config_path, F_OK) == 0) {
                        config_load_file(&xcodex_config, config_path);
                    } else {
                        config_load_file(&xcodex_config, XCODEX_CONFIG_FILE);
                    }
                } else {
                    // Fallback to current directory if no home found
                    config_load_file(&xcodex_config, XCODEX_CONFIG_FILE);
                }
                
                // Reapply configuration settings
                const char *theme_name = config_get(&xcodex_config, "theme");
                if (theme_name) {
                    int theme_found = 0;
                    for (int i = 0; i < NUM_THEMES; i++) {
                        if (strcmp(theme_name, themes[i].name) == 0) {
                            current_theme = i;
                            theme_found = 1;
                            break;
                        }
                    }
                    
                    if (theme_found) {
                        editorSetBackgroundColor(themes[current_theme].bg_color);
                        editorSetStatusMessage("Configuration reloaded - Applied theme: %s", theme_name);
                    } else {
                        editorSetStatusMessage("Configuration reloaded - Unknown theme: %s", theme_name);
                    }
                } else {
                    editorSetStatusMessage("Configuration reloaded");
                }
                
                // Update other settings
                E.show_line_numbers = config_get_bool(&xcodex_config, "line_numbers", 1);
                global_syntax_highlighting = config_get_bool(&xcodex_config, "syntax_highlighting", 1);
                global_show_status_bar = config_get_bool(&xcodex_config, "show_status_bar", 1);
                global_search_highlight = config_get_bool(&xcodex_config, "search_highlight", 1);
                global_auto_indent = config_get_bool(&xcodex_config, "auto_indent", 1);
                global_modal_editing = config_get_bool(&xcodex_config, "modal_editing", 1);
                
                int tab_size = config_get_int(&xcodex_config, "tab_size", 4);
                if (tab_size >= 1 && tab_size <= 16) {
                    global_tab_size = tab_size;
                }
                
                editorUpdateLineNumberWidth();
            }
            break;
            
        /* ==== Screen Movement ==== */
        case PAGE_UP:
        case PAGE_DOWN:
            if (c == PAGE_UP && E.cy != 0)
                E.cy = 0;
            else if (c == PAGE_DOWN && E.cy != E.screenrows-1)
                E.cy = E.screenrows-1;
            {
                int times = E.screenrows;
                while(times--)
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
            
        /* ==== Escape and Unknown ==== */
        case ESC:
            E.command_len = 0;
            E.command_buffer[0] = '\0';
            editorSetStatusMessage("-- NORMAL --");
            break;
            
        default:
            if (E.command_len > 0) {
                E.command_len = 0;
                E.command_buffer[0] = '\0';
                editorSetStatusMessage("-- NORMAL --");
            }
            break;
    }
    
    *quit_times = XCODEX_QUIT_TIMES;
}

/* Process keys in INSERT mode - Text insertion */
void xcodex_process_insert_mode(int c, int *quit_times) {
    switch (c) {
        case ESC:  /* Exit to normal mode */
        case CTRL_C:
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_hide();
                break;
            }
#endif
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            /* Move cursor left when exiting insert mode (vim behavior) */
            if (E.cx > 0) {
                E.cx--;
            } else if (E.coloff > 0) {
                E.coloff--;
            }
            break;
            
        case ENTER:  /* Insert newline */
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_accept();
                break;
            }
#endif
            editorInsertNewline();
            break;
            
        case BACKSPACE:  /* Delete character */
        case CTRL_H:
        case DEL_KEY:
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_hide();
            }
#endif
            editorDelChar();
            break;
            
        case TAB:  /* Insert tab character or accept completion */
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_accept();
                break;
            }
#endif
            editorInsertChar(c);
            break;
            
        case ARROW_UP:  /* Arrow key movement in insert mode */
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_prev();
                break;
            }
#endif
            editorMoveCursor(c);
            break;
            
        case ARROW_DOWN:
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_next();
                break;
            }
#endif
            editorMoveCursor(c);
            break;
            
        case ARROW_LEFT:  /* Arrow key movement in insert mode */
        case ARROW_RIGHT:
#ifdef XCODEX_ENABLE_COMPLETION
            if (xcodex_completion_is_visible()) {
                xcodex_completion_hide();
            }
#endif
            editorMoveCursor(c);
            break;
            
        case CTRL_N:  /* Trigger completion */
#ifdef XCODEX_ENABLE_COMPLETION
            xcodex_completion_trigger();
#endif
            break;
            
        case CTRL_S:  /* Save file */
            editorSave();
            break;
            
        case CTRL_Q:  /* Quit */
            if (E.dirty && *quit_times) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                    "Press Ctrl-Q %d more times to quit.", *quit_times);
                (*quit_times)--;
                return;
            }
            E.quit_requested = 1;
            break;
            
        default:
            if (isprint(c)) {
#ifdef XCODEX_ENABLE_COMPLETION
                // Hide completion on any printable character except when continuing word
                if (xcodex_completion_is_visible() && !isalnum(c) && c != '_') {
                    xcodex_completion_hide();
                }
#endif
                editorInsertChar(c);
                
#ifdef XCODEX_ENABLE_LUA
                // Call Lua hooks
                char char_str[2] = {c, '\0'};
                xcodex_lua_call_hook(XCODEX_HOOK_ON_CHAR_INSERT, char_str);
#endif

#ifdef XCODEX_ENABLE_COMPLETION
                // Auto-trigger completion after typing
                if (isalpha(c) || c == '_') {
                    char *prefix = xcodex_get_current_word_prefix();
                    if (prefix && strlen(prefix) >= 2) {  // Trigger after 2 characters
                        xcodex_completion_trigger();
                    }
                    free(prefix);
                }
#endif
            }
            break;
    }
    
    *quit_times = XCODEX_QUIT_TIMES;
}

/* Process keys in VISUAL mode - Text selection */
void xcodex_process_visual_mode(int c) {
    switch (c) {
        case ESC:  /* Exit to normal mode */
        case CTRL_C:
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            break;
            
        case 'v':  /* Switch to character-wise visual mode */
            if (E.mode == XCODEX_MODE_VISUAL_LINE || E.mode == XCODEX_MODE_VISUAL_BLOCK) {
                xcodex_set_mode(XCODEX_MODE_VISUAL);
            }
            break;
            
        case 'V':  /* Switch to line-wise visual mode */
            if (E.mode == XCODEX_MODE_VISUAL || E.mode == XCODEX_MODE_VISUAL_BLOCK) {
                xcodex_set_mode(XCODEX_MODE_VISUAL_LINE);
            }
            break;
            
        case CTRL_V:  /* Switch to block visual mode */
            if (E.mode == XCODEX_MODE_VISUAL || E.mode == XCODEX_MODE_VISUAL_LINE) {
                xcodex_set_mode(XCODEX_MODE_VISUAL_BLOCK);
            }
            break;
            
        /* Movement keys update selection */
        case 'h':
        case ARROW_LEFT:
            /* Move cursor left */
            if (E.cx > 0) {
                E.cx--;
            } else if (E.coloff > 0) {
                E.coloff--;
            } else if (E.cy > 0) {
                E.cy--;
                /* Move to end of previous line */
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    E.cx = row->size;
                    if (E.cx > E.screencols - E.line_numbers_width - 1) {
                        E.coloff = E.cx - (E.screencols - E.line_numbers_width - 1);
                        E.cx = E.screencols - E.line_numbers_width - 1;
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'j':
        case ARROW_DOWN:
            /* Move cursor down */
            if (E.cy < E.screenrows - 1) {
                E.cy++;
            } else if (E.rowoff + E.screenrows < E.numrows) {
                E.rowoff++;
            }
            /* Adjust cursor position for line length */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    if (E.cx > row->size) {
                        E.cx = row->size;
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'k':
        case ARROW_UP:
            /* Move cursor up */
            if (E.cy > 0) {
                E.cy--;
            } else if (E.rowoff > 0) {
                E.rowoff--;
            }
            /* Adjust cursor position for line length */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    if (E.cx > row->size) {
                        E.cx = row->size;
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'l':
        case ARROW_RIGHT:
            /* Move cursor right */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    if (E.cx < row->size) {
                        if (E.cx < E.screencols - E.line_numbers_width - 1) {
                            E.cx++;
                        } else {
                            E.coloff++;
                        }
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'w':  /* Word movement forward */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    int filecol = E.coloff + E.cx;
                    
                    /* Skip current word */
                    while (filecol < row->size && (isalnum(row->chars[filecol]) || row->chars[filecol] == '_')) {
                        filecol++;
                    }
                    /* Skip whitespace */
                    while (filecol < row->size && isspace(row->chars[filecol])) {
                        filecol++;
                    }
                    
                    /* Update cursor position */
                    if (filecol < row->size) {
                        E.cx = filecol - E.coloff;
                        if (E.cx >= E.screencols - E.line_numbers_width) {
                            E.coloff = filecol - (E.screencols - E.line_numbers_width - 1);
                            E.cx = E.screencols - E.line_numbers_width - 1;
                        }
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'b':  /* Word movement backward */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    int filecol = E.coloff + E.cx;
                    
                    if (filecol > 0) {
                        filecol--;
                        /* Skip whitespace */
                        while (filecol > 0 && isspace(row->chars[filecol])) {
                            filecol--;
                        }
                        /* Skip current word */
                        while (filecol > 0 && (isalnum(row->chars[filecol]) || row->chars[filecol] == '_')) {
                            filecol--;
                        }
                        if (filecol > 0 && !(isalnum(row->chars[filecol]) || row->chars[filecol] == '_')) {
                            filecol++;
                        }
                        
                        /* Update cursor position */
                        if (filecol >= E.coloff) {
                            E.cx = filecol - E.coloff;
                        } else {
                            E.coloff = filecol;
                            E.cx = 0;
                        }
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case '0':  /* Line start */
            E.cx = 0;
            E.coloff = 0;
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case '$':  /* Line end */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    if (row->size > 0) {
                        int target_col = row->size - 1;
                        if (target_col < E.screencols - E.line_numbers_width) {
                            E.cx = target_col;
                            E.coloff = 0;
                        } else {
                            E.coloff = target_col - (E.screencols - E.line_numbers_width - 1);
                            E.cx = E.screencols - E.line_numbers_width - 1;
                        }
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        /* ==== Advanced Movement Commands ==== */
        case 'G':  /* Go to line (or last line if no count) */
            xcodex_go_to_last_line();
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case 'g':  /* Handle gg command */
            if (E.command_len == 0) {
                E.command_buffer[0] = 'g';
                E.command_len = 1;
                E.command_buffer[1] = '\0';
                editorSetStatusMessage("-- VISUAL -- g");
                return;
            } else if (E.command_len == 1 && E.command_buffer[0] == 'g') {
                xcodex_go_to_first_line();
                E.command_len = 0;
                E.command_buffer[0] = '\0';
                /* Update selection end */
                E.visual_end_row = E.cy;
                if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                    E.visual_end_col = 0;
                } else {
                    E.visual_end_col = E.cx;
                }
            }
            break;
            
        case 'e':  /* Move to end of word */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    int filecol = E.coloff + E.cx;
                    
                    /* If at end of word, move to next word */
                    if (filecol < row->size && !xcodex_is_word_boundary(row->chars[filecol])) {
                        while (filecol < row->size && !xcodex_is_word_boundary(row->chars[filecol])) {
                            filecol++;
                        }
                        filecol--; /* Back up to last character of word */
                    } else {
                        /* Skip whitespace */
                        while (filecol < row->size && xcodex_is_word_boundary(row->chars[filecol])) {
                            filecol++;
                        }
                        /* Move to end of word */
                        while (filecol < row->size && !xcodex_is_word_boundary(row->chars[filecol])) {
                            filecol++;
                        }
                        if (filecol > 0) filecol--; /* Back up to last character of word */
                    }
                    
                    /* Update cursor position */
                    int effective_cols = E.screencols - E.line_numbers_width;
                    if (filecol >= E.coloff + effective_cols) {
                        E.coloff = filecol - effective_cols + 1;
                        E.cx = effective_cols - 1;
                    } else {
                        E.cx = filecol - E.coloff;
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        case '%':  /* Jump to matching bracket */
            {
                int filerow = E.rowoff + E.cy;
                if (filerow >= 0 && filerow < E.numrows) {
                    erow *row = &E.row[filerow];
                    int filecol = E.coloff + E.cx;
                    
                    if (filecol < row->size) {
                        char c = row->chars[filecol];
                        char match_char = 0;
                        int direction = 0;
                        
                        /* Determine matching character and direction */
                        switch (c) {
                            case '(': match_char = ')'; direction = 1; break;
                            case ')': match_char = '('; direction = -1; break;
                            case '[': match_char = ']'; direction = 1; break;
                            case ']': match_char = '['; direction = -1; break;
                            case '{': match_char = '}'; direction = 1; break;
                            case '}': match_char = '{'; direction = -1; break;
                            default: 
                                editorSetStatusMessage("No matching bracket found");
                                break;
                        }
                        
                        if (match_char) {
                            int level = 1;
                            int search_row = filerow;
                            int search_col = filecol + direction;
                            
                            /* Search for matching bracket */
                            while (level > 0 && search_row >= 0 && search_row < E.numrows) {
                                erow *search_row_ptr = &E.row[search_row];
                                
                                while (level > 0 && search_col >= 0 && search_col < search_row_ptr->size) {
                                    char search_c = search_row_ptr->chars[search_col];
                                    if (search_c == c) {
                                        level++;
                                    } else if (search_c == match_char) {
                                        level--;
                                    }
                                    
                                    if (level == 0) {
                                        /* Found matching bracket */
                                        E.cy = 0;
                                        E.rowoff = search_row;
                                        E.cx = search_col - E.coloff;
                                        E.coloff = 0;
                                        
                                        if (search_row < E.screenrows) {
                                            E.cy = search_row;
                                            E.rowoff = 0;
                                        }
                                        
                                        int effective_cols = E.screencols - E.line_numbers_width;
                                        if (search_col >= effective_cols) {
                                            E.coloff = search_col - effective_cols + 1;
                                            E.cx = effective_cols - 1;
                                        } else {
                                            E.cx = search_col;
                                            E.coloff = 0;
                                        }
                                        break;
                                    }
                                    search_col += direction;
                                }
                                
                                if (level > 0) {
                                    search_row += direction;
                                    search_col = (direction > 0) ? 0 : (search_row >= 0 && search_row < E.numrows ? E.row[search_row].size - 1 : 0);
                                }
                            }
                            
                            if (level > 0) {
                                editorSetStatusMessage("No matching bracket found");
                            }
                        }
                    }
                }
            }
            /* Update selection end */
            E.visual_end_row = E.cy;
            if (E.mode == XCODEX_MODE_VISUAL_LINE) {
                E.visual_end_col = 0;
            } else {
                E.visual_end_col = E.cx;
            }
            break;
            
        /* Actions on selection */
        case 'x':
        case 'd':
            /* Cut (delete and yank) */
            {
                int size, is_line_mode;
                char *selected_text = xcodex_get_selected_text(&size, &is_line_mode);
                
                if (selected_text) {
                    xcodex_yank_text(selected_text, size, is_line_mode);
                    xcodex_delete_selected_text();
                    free(selected_text);
                    
                    if (is_line_mode) {
                        editorSetStatusMessage("Cut %d lines", 
                            E.visual_end_row - E.visual_start_row + 1);
                    } else {
                        editorSetStatusMessage("Cut %d characters", size);
                    }
                } else {
                    editorSetStatusMessage("Nothing to cut");
                }
            }
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            break;
            
        case 'y':
            /* Yank (copy) */
            {
                int size, is_line_mode;
                char *selected_text = xcodex_get_selected_text(&size, &is_line_mode);
                
                if (selected_text) {
                    xcodex_yank_text(selected_text, size, is_line_mode);
                    free(selected_text);
                    
                    if (is_line_mode) {
                        editorSetStatusMessage("Yanked %d lines", 
                            E.visual_end_row - E.visual_start_row + 1);
                    } else {
                        editorSetStatusMessage("Yanked %d characters", size);
                    }
                } else {
                    editorSetStatusMessage("Nothing to yank");
                }
            }
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            break;
            
        case 'c':
            /* Change (cut and enter insert mode) */
            {
                int size, is_line_mode;
                char *selected_text = xcodex_get_selected_text(&size, &is_line_mode);
                
                if (selected_text) {
                    xcodex_yank_text(selected_text, size, is_line_mode);
                    xcodex_delete_selected_text();
                    free(selected_text);
                    
                    if (is_line_mode) {
                        editorSetStatusMessage("Changed %d lines", 
                            E.visual_end_row - E.visual_start_row + 1);
                    } else {
                        editorSetStatusMessage("Changed %d characters", size);
                    }
                } else {
                    editorSetStatusMessage("Nothing to change");
                }
            }
            xcodex_set_mode(XCODEX_MODE_INSERT);
            break;

        case 'u':  /* Undo */
            xcodex_undo();
            break;
            
        default:
            break;
    }
}

/* Process keys in COMMAND mode */
void xcodex_process_command_mode(int c) {
    switch (c) {
        case ESC:  /* Exit to normal mode */
        case CTRL_C:
            E.command_len = 0;
            E.command_buffer[0] = '\0';
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            break;
            
        case ENTER:  /* Execute command */
            E.command_buffer[E.command_len] = '\0';
            xcodex_execute_command(E.command_buffer);
            E.command_len = 0;
            E.command_buffer[0] = '\0';
            xcodex_set_mode(XCODEX_MODE_NORMAL);
            break;
            
        case BACKSPACE:  /* Delete character */
        case CTRL_H:
            if (E.command_len > 0) {
                E.command_len--;
                E.command_buffer[E.command_len] = '\0';
                editorSetStatusMessage(":%s", E.command_buffer);
            } else {
                xcodex_set_mode(XCODEX_MODE_NORMAL);
            }
            break;
            
        default:
            if (isprint(c) && E.command_len < sizeof(E.command_buffer) - 1) {
                E.command_buffer[E.command_len++] = c;
                E.command_buffer[E.command_len] = '\0';
                editorSetStatusMessage(":%s", E.command_buffer);
            }
            break;
    }
}

/* Execute command mode commands */
void xcodex_execute_command(char *command) {
    if (!command) return;
    
    if (strcmp(command, "q") == 0) {
        if (E.dirty) {
            editorSetStatusMessage("No write since last change (use :q! to override)");
            return;
        }
        E.quit_requested = 1;
    } else if (strcmp(command, "q!") == 0) {
        E.quit_requested = 1;
    } else if (strcmp(command, "w") == 0) {
        if (E.filename) {
            editorSave();
        } else {
            editorSetStatusMessage("No file name");
        }
    } else if (strcmp(command, "wq") == 0) {
        if (E.filename) {
            editorSave();
            E.quit_requested = 1;
        } else {
            editorSetStatusMessage("No file name");
        }
    }
#ifdef XCODEX_ENABLE_LUA
    else if (strncmp(command, "plugin ", 7) == 0) {
        char *plugin_path = command + 7;
        if (xcodex_lua_load_plugin(plugin_path) == 0) {
            editorSetStatusMessage("Plugin loaded successfully");
        }
    } else if (strcmp(command, "plugins") == 0) {
        if (g_plugin_manager.count == 0) {
            editorSetStatusMessage("No plugins loaded");
        } else {
            editorSetStatusMessage("%d plugin(s) loaded", g_plugin_manager.count);
        }
    } else if (strncmp(command, "plugindir ", 10) == 0) {
        char *plugin_dir = command + 10;
        int loaded = xcodex_lua_load_plugins_from_dir(plugin_dir);
        if (loaded > 0) {
            editorSetStatusMessage("Loaded %d plugins from %s", loaded, plugin_dir);
        } else {
            editorSetStatusMessage("No plugins found in %s", plugin_dir);
        }
    }
#endif
#ifdef XCODEX_ENABLE_COMPLETION
    else if (strcmp(command, "complete") == 0) {
        xcodex_completion_trigger();
        editorSetStatusMessage("Completion triggered");
    }
#endif
    else if (strcmp(command, "help") == 0) {
        editorSetStatusMessage("Commands: q w wq [line#]"
#ifdef XCODEX_ENABLE_LUA
                            " | plugin [file] plugins plugindir [dir]"
#endif
#ifdef XCODEX_ENABLE_COMPLETION  
                            " | complete"
#endif
                            " | Ctrl+N=complete");
    } else if (strlen(command) > 0 && command[0] >= '1' && command[0] <= '9') {
        int line = atoi(command);
        if (line > 0) {
            xcodex_go_to_line(line);
        }
    } else if (strlen(command) > 0) {
        editorSetStatusMessage("Unknown command: %s", command);
    } else {
        editorSetStatusMessage("Empty command");
    }
}

int editorFileWasModified(void) {
    return E.dirty;
}

void updateWindowSize(void) {
    if (getWindowSize(STDIN_FILENO,STDOUT_FILENO,
                    &E.screenrows,&E.screencols) == -1) {
        perror("Unable to query the screen for size (columns / rows)");
        exit(1);
    }
    E.screenrows -= 2; /* Get room for status bar. */
}

#if XCODEX_WINDOWS
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch(dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        editorAtExit();
        return TRUE;
    default:
        return FALSE;
    }
}

void handleSigWinCh() {
    updateWindowSize();
    if (E.cy > E.screenrows) E.cy = E.screenrows - 1;
    int effective_screencols = E.screencols - E.line_numbers_width;
    if (E.cx > effective_screencols) E.cx = effective_screencols - 1;
    editorRefreshScreen();
}
#endif

#if XCODEX_POSIX
void handleSigWinCh(int unused __attribute__((unused))) {
    updateWindowSize();
    if (E.cy > E.screenrows) E.cy = E.screenrows - 1;
    int effective_screencols = E.screencols - E.line_numbers_width;
    if (E.cx > effective_screencols) E.cx = effective_screencols - 1;
    editorRefreshScreen();
}
#endif

void initEditor(void) {
    /* Initialize configuration system first - ensure it's always loaded */
    config_init(&xcodex_config);
    config_load_defaults(&xcodex_config, "xcodex");
    
    /* Try to load configuration file from user's home directory first */
    char config_path[512];
    const char *home = getenv("HOME");
    if (!home) {
#ifdef _WIN32
        home = getenv("USERPROFILE");
#endif
    }
    
    int config_loaded = 0;
    if (home) {
#ifdef _WIN32
        snprintf(config_path, sizeof(config_path), "%s\\%s", home, XCODEX_CONFIG_FILE);
#else
        snprintf(config_path, sizeof(config_path), "%s/%s", home, XCODEX_CONFIG_FILE);
#endif
        
        /* Try to load from home directory first */
        if (access(config_path, F_OK) == 0) {
            if (config_load_file(&xcodex_config, config_path) == 0) {
                config_loaded = 1;
            }
        }
    }
    
    /* If not loaded from home directory, try current directory */
    if (!config_loaded) {
        if (config_load_file(&xcodex_config, XCODEX_CONFIG_FILE) != 0) {
            /* Configuration file doesn't exist anywhere, create default one in home directory */
            if (home) {
#ifdef _WIN32
                snprintf(config_path, sizeof(config_path), "%s\\%s", home, XCODEX_CONFIG_FILE);
#else
                snprintf(config_path, sizeof(config_path), "%s/%s", home, XCODEX_CONFIG_FILE);
#endif
                if (config_save_file(&xcodex_config, config_path) == 0) {
                    editorSetStatusMessage("Created default configuration file: %s", config_path);
                }
            } else {
                /* Fallback to current directory if no home found */
                if (config_save_file(&xcodex_config, XCODEX_CONFIG_FILE) == 0) {
                    editorSetStatusMessage("Created default configuration file: %s", XCODEX_CONFIG_FILE);
                }
            }
        }
    }
    
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.syntax = NULL;
    E.show_line_numbers = 1;        /* Line numbers ON by default */
    E.line_numbers_width = 0;
    
    /* Apply configuration settings using enhanced typed system */
    E.show_line_numbers = config_get_bool(&xcodex_config, "line_numbers", 1);
    
    /* Initialize global configuration settings */
    global_syntax_highlighting = config_get_bool(&xcodex_config, "syntax_highlighting", 1);
    global_show_status_bar = config_get_bool(&xcodex_config, "show_status_bar", 1);
    global_search_highlight = config_get_bool(&xcodex_config, "search_highlight", 1);
    global_auto_indent = config_get_bool(&xcodex_config, "auto_indent", 1);
    global_modal_editing = config_get_bool(&xcodex_config, "modal_editing", 1);
    
    /* Numeric settings with bounds checking */
    int tab_size = config_get_int(&xcodex_config, "tab_size", 4);
    if (tab_size >= 1 && tab_size <= 16) {
        global_tab_size = tab_size;
    } else {
        global_tab_size = 4; /* fallback to default */
    }
    
    /* Set undo system capacity based on configuration */
    int undo_levels = config_get_int(&xcodex_config, "undo_levels", 100);
    if (undo_levels >= 1 && undo_levels <= 100) {
        E.undo_capacity = undo_levels;
    } else {
        E.undo_capacity = 100; /* fallback to maximum */
    }
    
    /* Theme application with enhanced validation and mapping */
    const char *theme_name = config_get(&xcodex_config, "theme");
    if (theme_name) {
        // Use helper function for theme name-to-index mapping
        int theme_index = xcodex_get_theme_index(theme_name);
        
        if (theme_index >= 0) {
            current_theme = theme_index;
            editorSetStatusMessage("✅ Applied theme: %s (index %d)", theme_name, theme_index);
        } else {
            // Enhanced error message with complete theme list
            printf("⚠️  Unknown theme '%s'. Available themes:\n", theme_name);
            for (int i = 0; i < NUM_THEMES; i++) {
                printf("  %d: %s%s\n", i, themes[i].name, (i == 0) ? " (default)" : "");
            }
            current_theme = 0; // Fallback to xcodex_dark (index 0)
            editorSetStatusMessage("🔄 Fallback to default theme: %s", themes[0].name);
        }
    } else {
        // No theme configured, use default
        current_theme = 0; // Default to xcodex_dark (index 0)
        editorSetStatusMessage("🎨 Using default theme: %s", themes[0].name);
    }
    
    /* Apply the background color for the current theme - critical for theme application */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        editorSetBackgroundColor(themes[current_theme].bg_color);
        /* Force clear screen to apply background */
        char clear_buf[16];
        int len = snprintf(clear_buf, sizeof(clear_buf), "\x1b[2J\x1b[H");
        if (len > 0 && len < sizeof(clear_buf)) {
            xcodex_write(STDOUT_FILENO, clear_buf, len);
        }
    }
    
    /* Initialize modal system */
    xcodex_init_modal_system();
    
    /* Initialize undo system */
    xcodex_init_undo_system();
    
    updateWindowSize();
    editorUpdateLineNumberWidth();  /* Initialize line number width */
    
    /* Platform-specific initialization */
#if XCODEX_WINDOWS
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    /* Enable UTF-8 output */
    SetConsoleOutputCP(CP_UTF8);
#endif

#if XCODEX_POSIX
    signal(SIGWINCH, handleSigWinCh);
    signal(SIGPIPE, SIG_IGN);
#endif
    
    /* Ensure background color is applied again after all initialization */
    if (current_theme >= 0 && current_theme < NUM_THEMES) {
        editorSetBackgroundColor(themes[current_theme].bg_color);
    }
    
    /* Initialize plugin system */
#ifdef XCODEX_ENABLE_LUA
    if (xcodex_lua_init() != 0) {
        editorSetStatusMessage("Failed to initialize Lua plugin system");
        sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
    } else {
        /* Auto-load plugins from plugins directory */
        if (xcodex_lua_auto_load_plugins() != 0) {
            editorSetStatusMessage("Warning: Some plugins failed to load");
            sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
        }
    }
#endif
    
    /* Initialize completion system */
#ifdef XCODEX_ENABLE_COMPLETION
    if (xcodex_completion_init() != 0) {
        editorSetStatusMessage("Failed to initialize completion system");
        sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
    } else {
        editorSetStatusMessage("Completion system initialized with auto-trigger enabled");
        sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
    }
#endif

    /* Initialize LSP system */
#ifdef XCODEX_ENABLE_LSP
    if (xcodex_lsp_init() != 0) {
        editorSetStatusMessage("Failed to initialize LSP system");
        sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
    } else {
        editorSetStatusMessage("LSP system initialized with auto-start enabled");
        sleep(1); /* Sleep to allow the message to be displayed and the user can see it*/
    }
#endif
}

int xcodex_main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,"Usage: xcodex <filename>\n");
        return 1;
    }

    initEditor();
    editorSelectSyntaxHighlight(argv[1]);
    editorOpen(argv[1]);
    
    /* Try to enable raw mode with better error handling */
    if (enableRawMode(STDIN_FILENO) == -1) {
        fprintf(stderr, "Failed to enable raw mode\n");
        return 0;
    }
    
    editorSetStatusMessage(
        "Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | Ctrl-T = theme | Ctrl-N = line num | Ctrl-R = reload config");
    
    while(!E.quit_requested) {
        editorRefreshScreen();
        editorProcessKeypress(STDIN_FILENO);
        
#if XCODEX_WINDOWS
        /* Handle window resize on Windows */
        CONSOLE_SCREEN_BUFFER_INFO new_csbi;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &new_csbi)) {
            int new_cols = new_csbi.srWindow.Right - new_csbi.srWindow.Left + 1;
            int new_rows = new_csbi.srWindow.Bottom - new_csbi.srWindow.Top + 1;
            if (new_cols != E.screencols || new_rows != E.screenrows + 2) {
                handleSigWinCh();
            }
        }
#endif
    }
    
    /* Clean up terminal state before exiting */
    editorAtExit();
    
    return 1;
}

#if XCODEX_WINDOWS || XCODEX_POSIX

int XcodexMain(int argc, char **argv) {
    return xcodex_main(argc, argv);
}

#else

int XcodexMain(int argc, char **argv) {
    printf("XCodex editor is not supported on this platform.\n");
    return 1;
}

#endif
