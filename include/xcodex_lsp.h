#ifndef XCODEX_LSP_H
#define XCODEX_LSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform-specific includes */
#ifdef _WIN32
    #include <windows.h>
#endif

#ifndef _WIN32
    #include <unistd.h>
    #include <sys/types.h>
#endif

#ifdef XCODEX_ENABLE_LSP

/* LSP Method Types */
#define LSP_INITIALIZE              "initialize"
#define LSP_INITIALIZED             "initialized"
#define LSP_SHUTDOWN                "shutdown"
#define LSP_EXIT                    "exit"
#define LSP_TEXT_DOCUMENT_OPEN      "textDocument/didOpen"
#define LSP_TEXT_DOCUMENT_CHANGE    "textDocument/didChange"
#define LSP_TEXT_DOCUMENT_SAVE      "textDocument/didSave"
#define LSP_TEXT_DOCUMENT_CLOSE     "textDocument/didClose"
#define LSP_COMPLETION              "textDocument/completion"
#define LSP_HOVER                   "textDocument/hover"
#define LSP_GOTO_DEFINITION         "textDocument/definition"
#define LSP_FIND_REFERENCES         "textDocument/references"
#define LSP_DOCUMENT_SYMBOLS        "textDocument/documentSymbol"
#define LSP_WORKSPACE_SYMBOLS       "workspace/symbol"
#define LSP_CODE_ACTION             "textDocument/codeAction"
#define LSP_FORMATTING              "textDocument/formatting"
#define LSP_RENAME                  "textDocument/rename"
#define LSP_SIGNATURE_HELP          "textDocument/signatureHelp"

/* LSP Server Configuration */
typedef struct {
    char *name;                 /* Server name (e.g., "clangd") */
    char *command;              /* Server executable path */
    char **args;                /* Command line arguments */
    int arg_count;              /* Number of arguments */
    char **file_patterns;       /* File patterns this server handles */
    int pattern_count;          /* Number of file patterns */
    int enabled;                /* Whether server is enabled */
} lsp_server_config_t;

/* LSP Server Instance */
typedef struct {
    lsp_server_config_t *config;
    
#ifndef _WIN32
    int pid;                    /* Process ID */
    int stdin_fd;               /* Pipe to server stdin */
    int stdout_fd;              /* Pipe from server stdout */
    int stderr_fd;              /* Pipe from server stderr */
#endif

#ifdef _WIN32
    HANDLE process_handle;      /* Process handle */
    HANDLE stdin_handle;        /* Pipe to server stdin */
    HANDLE stdout_handle;       /* Pipe from server stdout */
    HANDLE stderr_handle;       /* Pipe from server stderr */
#endif
    
    int next_request_id;        /* Next JSON-RPC request ID */
    int initialized;            /* Whether server is initialized */
    int running;                /* Whether server process is running */
    char *root_uri;             /* Workspace root URI */
} lsp_server_t;

/* LSP Client Manager */
typedef struct {
    lsp_server_t *servers;      /* Array of LSP servers */
    int server_count;           /* Number of active servers */
    int server_capacity;        /* Server array capacity */
    char *workspace_root;       /* Current workspace root */
    int auto_start;             /* Whether to auto-start servers */
    int completion_enabled;     /* Whether LSP completion is enabled */
} lsp_client_t;

/* LSP Completion Item */
typedef struct {
    char *label;                /* Display text */
    char *detail;               /* Additional detail */
    char *documentation;        /* Documentation string */
    char *insert_text;          /* Text to insert */
    int kind;                   /* CompletionItemKind */
} lsp_completion_item_t;

/* LSP Diagnostic */
typedef struct {
    int line;                   /* Line number (0-based) */
    int character;              /* Character offset (0-based) */
    int end_line;               /* End line number */
    int end_character;          /* End character offset */
    int severity;               /* Diagnostic severity */
    char *message;              /* Diagnostic message */
    char *source;               /* Diagnostic source */
} lsp_diagnostic_t;

/* Function declarations */
int xcodex_lsp_init(void);
void xcodex_lsp_cleanup(void);
int xcodex_lsp_add_server_config(const char *name, const char *command, 
                                 char **args, int arg_count,
                                 char **file_patterns, int pattern_count);
int xcodex_lsp_start_server(const char *server_name);
int xcodex_lsp_stop_server(const char *server_name);
int xcodex_lsp_restart_server(const char *server_name);
lsp_server_t* xcodex_lsp_get_server_for_file(const char *filename);
int xcodex_lsp_send_initialize(lsp_server_t *server);
int xcodex_lsp_send_text_document_open(lsp_server_t *server, const char *uri, 
                                       const char *language, const char *content);
int xcodex_lsp_send_text_document_change(lsp_server_t *server, const char *uri, 
                                         const char *content);
int xcodex_lsp_send_text_document_save(lsp_server_t *server, const char *uri);
int xcodex_lsp_send_text_document_close(lsp_server_t *server, const char *uri);
int xcodex_lsp_request_completion(lsp_server_t *server, const char *uri, 
                                  int line, int character);
int xcodex_lsp_request_hover(lsp_server_t *server, const char *uri, 
                             int line, int character);
int xcodex_lsp_request_goto_definition(lsp_server_t *server, const char *uri, 
                                       int line, int character);
int xcodex_lsp_handle_server_response(lsp_server_t *server);
void xcodex_lsp_process_diagnostics(lsp_server_t *server, const char *response);
lsp_completion_item_t* xcodex_lsp_parse_completion_response(const char *response, 
                                                            int *item_count);
char* xcodex_lsp_file_to_uri(const char *filepath);
char* xcodex_lsp_uri_to_file(const char *uri);
void xcodex_lsp_free_completion_items(lsp_completion_item_t *items, int count);
int xcodex_lsp_is_server_running(const char *server_name);
void xcodex_lsp_load_default_servers(void);
int xcodex_lsp_auto_start_for_file(const char *filename);

/* Built-in LSP server configurations */
void xcodex_lsp_register_clangd(void);
void xcodex_lsp_register_rust_analyzer(void);
void xcodex_lsp_register_pylsp(void);
void xcodex_lsp_register_typescript_language_server(void);
void xcodex_lsp_register_gopls(void);
void xcodex_lsp_register_jdtls(void);

#endif /* XCODEX_ENABLE_LSP */

#endif /* XCODEX_LSP_H */
