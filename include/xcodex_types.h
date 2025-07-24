#ifndef XCODEX_TYPES_H
#define XCODEX_TYPES_H

#include <time.h>
#include "syntax.h"

/* Forward declarations */
struct abuf {
    char *b;
    int len;
};

/* This structure represents a single line of the file we are editing. */
typedef struct erow {
    int idx;            /* Row index within the file. */
    int size;           /* Size of the row, excluding the null term. */
    int rsize;          /* Size of the rendered row. */
    char *chars;        /* Row content. */
    char *render;       /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl;  /* Syntax highlight type for each character in render.*/
    int hl_oc;          /* Row had open comment at end in HL_MLCOMMENT state.*/
} erow;

/* Undo system structure */
struct undoEntry {
    int type;           /* Type of operation */
    int row;            /* Row number */
    int col;            /* Column number */
    char *data;         /* Data associated with operation */
    int data_len;       /* Length of data */
    int group_id;       /* Group ID for grouping related operations */
    int cursor_row;     /* Cursor position before operation */
    int cursor_col;     /* Cursor position before operation */
};

/* This structure contains the entire state of the editor. */
struct editorConfig {
    int cx, cy;           /* Cursor x and y position in characters */
    int rowoff;           /* Offset of row displayed. */
    int coloff;           /* Offset of column displayed. */
    int screenrows;       /* Number of rows that we can show */
    int screencols;       /* Number of cols that we can show */
    int numrows;          /* Number of rows */
    int rawmode;          /* Is terminal raw mode enabled? */
    erow *row;            /* Rows */
    int dirty;            /* File modified but not saved. */
    char *filename;       /* Currently open filename */
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    
    /* Modal system state */
    int mode;             /* Current mode */
    int motion_count;     /* Number prefix for motions */
    char command_buffer[256]; /* Buffer for multi-character commands */
    int command_len;      /* Length of current command */
    
    /* Visual mode state */
    int visual_start_row, visual_start_col;
    int visual_end_row, visual_end_col;
    int visual_line_mode;     /* 1 for line-wise, 0 for character-wise */
    
    /* Search state */
    char last_search[256];    /* Last search pattern */
    int search_direction;     /* 1 for forward, -1 for backward */
    
    /* Yank buffer */
    char *yank_buffer;
    int yank_buffer_size;
    int yank_is_line_mode;
    
    /* Theme and display settings */
    int show_line_numbers;
    int line_numbers_width;
    
    /* Undo system */
    struct undoEntry *undo_stack;  /* Stack of undo entries */
    int undo_count;                /* Number of undo entries */
    int undo_capacity;             /* Capacity of undo stack */
    int undo_group_id;             /* Current undo group ID */
    int xcodex_undo_in_progress;   /* Flag to indicate if undo is in progress */
    
    /* Exit flag */
    int quit_requested;
};

/* Global editor state - extern declaration */
extern struct editorConfig E;

/* Function declarations that are used across modules */
void editorSetStatusMessage(const char *fmt, ...);
void editorInsertChar(int c);
void editorDelChar(void);
void editorInsertNewline(void);
char* editorRowsToString(int *buflen);
void abAppend(struct abuf *ab, const char *s, int len);
const char* xcodex_get_mode_name(int mode);

#endif
