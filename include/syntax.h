#ifndef XCODEX_SYNTAX_H
#define XCODEX_SYNTAX_H

/* Syntax highlight types */
#define HL_NORMAL 0
#define HL_NONPRINT 1
#define HL_COMMENT 2   /* Single line comment. */
#define HL_MLCOMMENT 3 /* Multi-line comment. */
#define HL_KEYWORD1 4
#define HL_KEYWORD2 5
#define HL_KEYWORD3 6
#define HL_STRING 7
#define HL_NUMBER 8
#define HL_MATCH 9     /* Search match. */
#define HL_FUNCTION 10
#define HL_OPERATOR 11
#define HL_PREPROCESSOR 12
#define HL_BRACKET 13
#define HL_CONSTANT 14
#define HL_VARIABLE 15
#define HL_ERROR 16 /* Error in syntax highlight. */

#define HL_HIGHLIGHT_STRINGS (1<<0)
#define HL_HIGHLIGHT_NUMBERS (1<<1)

struct editorSyntax {
    char **filematch;
    char **keywords;
    char singleline_comment_start[3];
    char multiline_comment_start[8];
    char multiline_comment_end[8];
    int flags;
};

/* Syntax highlighting database */
extern struct editorSyntax HLDB[];
extern const int HLDB_ENTRIES;

#endif