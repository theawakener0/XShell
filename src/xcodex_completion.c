#include "xcodex_completion.h"
#include "xcodex.h"
#include "syntax.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

completion_popup_t g_completion = {0};

/* Forward declarations */
void editorSetStatusMessage(const char *fmt, ...);

/* Initialize completion system */
int xcodex_completion_init(void) {
    g_completion.capacity = 100;
    g_completion.items = malloc(sizeof(completion_item_t) * g_completion.capacity);
    if (!g_completion.items) return -1;
    
    memset(g_completion.items, 0, sizeof(completion_item_t) * g_completion.capacity);
    g_completion.count = 0;
    g_completion.selected = 0;
    g_completion.visible = 0;
    g_completion.filter_prefix = NULL;
    
    return 0;
}

/* Cleanup completion system */
void xcodex_completion_cleanup(void) {
    if (g_completion.items) {
        for (int i = 0; i < g_completion.count; i++) {
            free(g_completion.items[i].text);
            free(g_completion.items[i].display_text);
            free(g_completion.items[i].detail);
        }
        free(g_completion.items);
        g_completion.items = NULL;
    }
    
    free(g_completion.filter_prefix);
    g_completion.filter_prefix = NULL;
    
    memset(&g_completion, 0, sizeof(g_completion));
}

/* Clear completion items */
static void clear_completion_items(void) {
    for (int i = 0; i < g_completion.count; i++) {
        free(g_completion.items[i].text);
        free(g_completion.items[i].display_text);
        free(g_completion.items[i].detail);
        memset(&g_completion.items[i], 0, sizeof(completion_item_t));
    }
    g_completion.count = 0;
    g_completion.selected = 0;
}

/* Add completion item */
static void add_completion_item(const char *text, const char *detail, int type) {
    if (!text || g_completion.count >= g_completion.capacity) {
        if (g_completion.count >= g_completion.capacity) {
            g_completion.capacity *= 2;
            completion_item_t *new_items = realloc(g_completion.items, 
                sizeof(completion_item_t) * g_completion.capacity);
            if (!new_items) return;
            g_completion.items = new_items;
            
            // Initialize new entries
            for (int i = g_completion.count; i < g_completion.capacity; i++) {
                memset(&g_completion.items[i], 0, sizeof(completion_item_t));
            }
        }
    }
    
    if (g_completion.count >= g_completion.capacity) return;
    
    completion_item_t *item = &g_completion.items[g_completion.count];
    item->text = strdup(text);
    item->display_text = strdup(text);
    item->detail = detail ? strdup(detail) : NULL;
    item->type = type;
    item->priority = type; // Simple priority based on type
    
    g_completion.count++;
}

/* Get current word prefix */
char* xcodex_get_current_word_prefix(void) {
    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    
    if (filerow < 0 || filerow >= E.numrows || !E.row) return strdup("");
    
    erow *row = &E.row[filerow];
    if (filecol > row->size) filecol = row->size;
    
    // Find start of current word
    int start = filecol;
    while (start > 0 && (isalnum(row->chars[start-1]) || row->chars[start-1] == '_')) {
        start--;
    }
    
    if (start == filecol) return strdup("");
    
    int len = filecol - start;
    if (len <= 0) return strdup("");
    
    char *prefix = malloc(len + 1);
    if (!prefix) return strdup("");
    
    memcpy(prefix, row->chars + start, len);
    prefix[len] = '\0';
    
    return prefix;
}

/* Compare function for sorting completions */
static int compare_completions(const void *a, const void *b) {
    const completion_item_t *item_a = (const completion_item_t *)a;
    const completion_item_t *item_b = (const completion_item_t *)b;
    
    // Sort by priority first, then alphabetically
    if (item_a->priority != item_b->priority) {
        return item_a->priority - item_b->priority;
    }
    
    return strcmp(item_a->text, item_b->text);
}

/* Filter completions based on prefix */
static void filter_completions(const char *prefix) {
    if (!prefix || strlen(prefix) == 0) return;
    
    int prefix_len = strlen(prefix);
    int write_index = 0;
    
    for (int read_index = 0; read_index < g_completion.count; read_index++) {
        completion_item_t *item = &g_completion.items[read_index];
        
        if (strncmp(item->text, prefix, prefix_len) == 0) {
            if (write_index != read_index) {
                g_completion.items[write_index] = g_completion.items[read_index];
                memset(&g_completion.items[read_index], 0, sizeof(completion_item_t));
            }
            write_index++;
        } else {
            // Free the filtered out item
            free(item->text);
            free(item->display_text);
            free(item->detail);
            memset(item, 0, sizeof(completion_item_t));
        }
    }
    
    g_completion.count = write_index;
    if (g_completion.selected >= g_completion.count) {
        g_completion.selected = g_completion.count > 0 ? g_completion.count - 1 : 0;
    }
}

/* Trigger completion */
void xcodex_completion_trigger(void) {
    clear_completion_items();
    g_completion.trigger_line = E.rowoff + E.cy;
    g_completion.trigger_col = E.coloff + E.cx;
    
    char *prefix = xcodex_get_current_word_prefix();
    
    // Update filter prefix
    free(g_completion.filter_prefix);
    g_completion.filter_prefix = strdup(prefix);
    
    /* Add keywords for current language */
    if (E.syntax && E.syntax->keywords) {
        for (int i = 0; E.syntax->keywords[i]; i++) {
            char *keyword = E.syntax->keywords[i];
            int klen = strlen(keyword);
            
            // Remove type indicators (| and ||)
            while (klen > 0 && keyword[klen-1] == '|') klen--;
            
            if (klen > 0 && strncmp(keyword, prefix, strlen(prefix)) == 0) {
                char clean_keyword[256];
                if (klen < 256) {
                    strncpy(clean_keyword, keyword, klen);
                    clean_keyword[klen] = '\0';
                    add_completion_item(clean_keyword, "keyword", 0);
                }
            }
        }
    }
    
    /* Add words from current buffer */
    for (int i = 0; i < E.numrows && E.row; i++) {
        erow *row = &E.row[i];
        if (!row->chars) continue;
        
        char *line = row->chars;
        int pos = 0;
        
        while (pos < row->size) {
            if (isalpha(line[pos]) || line[pos] == '_') {
                int word_start = pos;
                while (pos < row->size && (isalnum(line[pos]) || line[pos] == '_')) {
                    pos++;
                }
                
                int word_len = pos - word_start;
                if (word_len > strlen(prefix) && 
                    strncmp(line + word_start, prefix, strlen(prefix)) == 0) {
                    
                    char word[256];
                    if (word_len < 256) {
                        strncpy(word, line + word_start, word_len);
                        word[word_len] = '\0';
                        
                        // Check if already added
                        int already_added = 0;
                        for (int j = 0; j < g_completion.count; j++) {
                            if (g_completion.items[j].text && 
                                strcmp(g_completion.items[j].text, word) == 0) {
                                already_added = 1;
                                break;
                            }
                        }
                        
                        if (!already_added) {
                            add_completion_item(word, "identifier", 2);
                        }
                    }
                }
            } else {
                pos++;
            }
        }
    }
    
    if (g_completion.count > 0) {
        // Sort completions
        qsort(g_completion.items, g_completion.count, sizeof(completion_item_t), compare_completions);
        g_completion.visible = 1;
        g_completion.selected = 0;
    }
    
    free(prefix);
}

/* Accept selected completion */
void xcodex_completion_accept(void) {
    if (!g_completion.visible || g_completion.count == 0 || 
        g_completion.selected < 0 || g_completion.selected >= g_completion.count) {
        return;
    }
    
    completion_item_t *item = &g_completion.items[g_completion.selected];
    if (!item->text) return;
    
    char *prefix = xcodex_get_current_word_prefix();
    int prefix_len = strlen(prefix);
    
    // Delete the prefix
    for (int i = 0; i < prefix_len; i++) {
        editorDelChar();
    }
    
    // Insert the completion
    const char *text = item->text;
    for (int i = 0; text[i]; i++) {
        editorInsertChar(text[i]);
    }
    
    free(prefix);
    xcodex_completion_hide();
}

/* Draw completion popup */
void xcodex_completion_draw(struct abuf *ab) {
    if (!g_completion.visible || g_completion.count == 0 || !ab) return;
    
    // Calculate popup position
    int popup_row = E.cy + 1;
    int popup_col = E.cx + (E.show_line_numbers ? E.line_numbers_width : 0);
    
    // Adjust if popup would go off screen
    if (popup_row >= E.screenrows - 5) {
        popup_row = E.cy - g_completion.count - 1;
        if (popup_row < 0) popup_row = 0;
    }
    
    int max_width = 30;
    int visible_items = g_completion.count;
    if (visible_items > 10) visible_items = 10;
    
    for (int i = 0; i < visible_items; i++) {
        int item_idx = i;
        if (item_idx >= g_completion.count) break;
        
        completion_item_t *item = &g_completion.items[item_idx];
        if (!item->text) continue;
        
        // Move cursor to popup position
        char cursor_pos[32];
        snprintf(cursor_pos, sizeof(cursor_pos), "\x1b[%d;%dH", 
                popup_row + i + 1, popup_col + 1);
        abAppend(ab, cursor_pos, strlen(cursor_pos));
        
        // Draw selection background
        if (item_idx == g_completion.selected) {
            abAppend(ab, "\x1b[48;5;237m", 10); // Dark gray background
        } else {
            abAppend(ab, "\x1b[48;5;235m", 10); // Darker background
        }
        
        // Draw completion item
        char display[64];
        snprintf(display, sizeof(display), " %-20s %s", 
                item->display_text ? item->display_text : item->text,
                item->detail ? item->detail : "");
        
        int display_len = strlen(display);
        if (display_len > max_width) display_len = max_width;
        
        abAppend(ab, display, display_len);
        
        // Pad with spaces to fill width
        for (int j = display_len; j < max_width; j++) {
            abAppend(ab, " ", 1);
        }
        
        abAppend(ab, "\x1b[0m", 4); // Reset formatting
    }
}

/* Navigate completion list */
void xcodex_completion_next(void) {
    if (!g_completion.visible || g_completion.count == 0) return;
    g_completion.selected = (g_completion.selected + 1) % g_completion.count;
}

void xcodex_completion_prev(void) {
    if (!g_completion.visible || g_completion.count == 0) return;
    g_completion.selected = (g_completion.selected - 1 + g_completion.count) % g_completion.count;
}

void xcodex_completion_hide(void) {
    g_completion.visible = 0;
}

int xcodex_completion_is_visible(void) {
    return g_completion.visible;
}

/* Update completion filter when user types */
void xcodex_completion_update_filter(const char *prefix) {
    if (!prefix) return;
    
    free(g_completion.filter_prefix);
    g_completion.filter_prefix = strdup(prefix);
    
    filter_completions(prefix);
}
