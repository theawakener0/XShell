#ifndef XCODEX_COMPLETION_H
#define XCODEX_COMPLETION_H

#include "xcodex_types.h"

typedef struct {
    char *text;
    char *display_text;
    char *detail;
    int type; // 0=keyword, 1=function, 2=variable, 3=class, etc.
    int priority;
} completion_item_t;

typedef struct {
    completion_item_t *items;
    int count;
    int capacity;
    int selected;
    int visible;
    int trigger_line;
    int trigger_col;
    char *filter_prefix;
} completion_popup_t;

/* Completion functions */
int xcodex_completion_init(void);
void xcodex_completion_cleanup(void);
void xcodex_completion_trigger(void);
void xcodex_completion_hide(void);
void xcodex_completion_next(void);
void xcodex_completion_prev(void);
void xcodex_completion_accept(void);
void xcodex_completion_update_filter(const char *prefix);
int xcodex_completion_is_visible(void);
void xcodex_completion_draw(struct abuf *ab);
char* xcodex_get_current_word_prefix(void);

/* Built-in completion sources */
void xcodex_completion_add_keywords(const char *language);
void xcodex_completion_add_from_buffer(void);

extern completion_popup_t g_completion;

#endif
