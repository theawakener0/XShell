/* 
 * XCodex LSP (Language Server Protocol) Implementation
 * Provides intelligent code completion, diagnostics, and navigation
 */

#include "xcodex_types.h"

#ifdef XCODEX_ENABLE_LSP

#include "xcodex_lsp.h"
/* Note: JSON parsing will be implemented with manual string handling for now */

/* Platform-specific includes */
#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #include <io.h>
    #include <fcntl.h>
    #include <direct.h>  /* For _getcwd on Windows */
    /* Define POSIX-like constants for Windows */
    #ifndef WIFEXITED
    #define WIFEXITED(status) 1
    #endif
    #ifndef WEXITSTATUS
    #define WEXITSTATUS(status) (status)
    #endif
    #define getcwd _getcwd
    #define getpid _getpid
    #define write _write
    #define close _close
#endif

#ifndef _WIN32
    #include <sys/wait.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <unistd.h>  /* For getcwd, write, close, getpid */
#endif

#include <errno.h>

/* Global LSP client instance */
static lsp_client_t g_lsp_client = {0};

/* JSON-RPC message formatting - simple string-based implementation */
static char* format_json_rpc_request(int id, const char *method, const char *params) {
    char *json_content;
    size_t content_len;
    
    if (params && strlen(params) > 0) {
        content_len = snprintf(NULL, 0, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"id\":%d,\"params\":%s}", 
                            method, id, params);
        json_content = malloc(content_len + 1);
        snprintf(json_content, content_len + 1, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"id\":%d,\"params\":%s}", 
                method, id, params);
    } else {
        content_len = snprintf(NULL, 0, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"id\":%d}", 
                                method, id);
        json_content = malloc(content_len + 1);
        snprintf(json_content, content_len + 1, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"id\":%d}", 
                method, id);
    }
    
    char *result = malloc(content_len + 50);
    sprintf(result, "Content-Length: %zu\r\n\r\n%s", content_len, json_content);
    
    free(json_content);
    return result;
}

static char* format_json_rpc_notification(const char *method, const char *params) {
    char *json_content;
    size_t content_len;
    
    if (params && strlen(params) > 0) {
        content_len = snprintf(NULL, 0, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":%s}", 
                                method, params);
        json_content = malloc(content_len + 1);
        snprintf(json_content, content_len + 1, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":%s}", 
                method, params);
    } else {
        content_len = snprintf(NULL, 0, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\"}", method);
        json_content = malloc(content_len + 1);
        snprintf(json_content, content_len + 1, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\"}", method);
    }
    
    char *result = malloc(content_len + 50);
    sprintf(result, "Content-Length: %zu\r\n\r\n%s", content_len, json_content);
    
    free(json_content);
    return result;
}


/* Initialize LSP client */
int xcodex_lsp_init(void) {
    memset(&g_lsp_client, 0, sizeof(lsp_client_t));
    g_lsp_client.server_capacity = 10;
    g_lsp_client.servers = malloc(sizeof(lsp_server_t) * g_lsp_client.server_capacity);
    g_lsp_client.auto_start = 1;
    g_lsp_client.completion_enabled = 1;
    g_lsp_client.workspace_root = getcwd(NULL, 0);
    
    if (!g_lsp_client.servers) {
        return -1;
    }
    
    /* Load default server configurations */
    xcodex_lsp_load_default_servers();
    
    return 0;
}

/* Cleanup LSP client */
void xcodex_lsp_cleanup(void) {
    /* Stop all running servers */
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        if (g_lsp_client.servers[i].running) {
            xcodex_lsp_stop_server(g_lsp_client.servers[i].config->name);
        }
    }
    
    /* Free server configurations */
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        lsp_server_config_t *config = g_lsp_client.servers[i].config;
        if (config) {
            free(config->name);
            free(config->command);
            for (int j = 0; j < config->arg_count; j++) {
                free(config->args[j]);
            }
            free(config->args);
            for (int j = 0; j < config->pattern_count; j++) {
                free(config->file_patterns[j]);
            }
            free(config->file_patterns);
            free(config);
        }
        free(g_lsp_client.servers[i].root_uri);
    }
    
    free(g_lsp_client.servers);
    free(g_lsp_client.workspace_root);
    memset(&g_lsp_client, 0, sizeof(lsp_client_t));
}

/* Add server configuration */
int xcodex_lsp_add_server_config(const char *name, const char *command, 
                                 char **args, int arg_count,
                                 char **file_patterns, int pattern_count) {
    if (g_lsp_client.server_count >= g_lsp_client.server_capacity) {
        /* Resize array */
        g_lsp_client.server_capacity *= 2;
        g_lsp_client.servers = realloc(g_lsp_client.servers, 
                                       sizeof(lsp_server_t) * g_lsp_client.server_capacity);
        if (!g_lsp_client.servers) {
            return -1;
        }
    }
    
    lsp_server_t *server = &g_lsp_client.servers[g_lsp_client.server_count];
    memset(server, 0, sizeof(lsp_server_t));
    
    server->config = malloc(sizeof(lsp_server_config_t));
    if (!server->config) {
        return -1;
    }
    
    lsp_server_config_t *config = server->config;
    config->name = strdup(name);
    config->command = strdup(command);
    config->args = malloc(sizeof(char*) * arg_count);
    config->arg_count = arg_count;
    
    for (int i = 0; i < arg_count; i++) {
        config->args[i] = strdup(args[i]);
    }
    
    config->file_patterns = malloc(sizeof(char*) * pattern_count);
    config->pattern_count = pattern_count;
    
    for (int i = 0; i < pattern_count; i++) {
        config->file_patterns[i] = strdup(file_patterns[i]);
    }
    
    config->enabled = 1;
    server->next_request_id = 1;
    
    g_lsp_client.server_count++;
    return 0;
}

/* Start LSP server */
int xcodex_lsp_start_server(const char *server_name) {
    lsp_server_t *server = NULL;
    
    /* Find server by name */
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        if (strcmp(g_lsp_client.servers[i].config->name, server_name) == 0) {
            server = &g_lsp_client.servers[i];
            break;
        }
    }
    
    if (!server || server->running) {
        return -1;
    }

#ifdef _WIN32
    /* Windows implementation using CreateProcess */
    HANDLE hChildStdinRd, hChildStdinWr;
    HANDLE hChildStdoutRd, hChildStdoutWr;
    HANDLE hChildStderrRd, hChildStderrWr;
    SECURITY_ATTRIBUTES saAttr;
    
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    /* Create pipes */
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0) ||
        !CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0) ||
        !CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0)) {
        return -1;
    }
    
    /* Ensure the read handle to the pipe for STDOUT/STDERR is not inherited */
    SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStderrRd, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
    
    /* Build command line */
    char cmdline[1024] = {0};
    snprintf(cmdline, sizeof(cmdline), "%s", server->config->command);
    for (int i = 0; i < server->config->arg_count; i++) {
        strncat(cmdline, " ", sizeof(cmdline) - strlen(cmdline) - 1);
        strncat(cmdline, server->config->args[i], sizeof(cmdline) - strlen(cmdline) - 1);
    }
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hChildStderrWr;
    si.hStdOutput = hChildStdoutWr;
    si.hStdInput = hChildStdinRd;
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hChildStdinRd);
        CloseHandle(hChildStdinWr);
        CloseHandle(hChildStdoutRd);
        CloseHandle(hChildStdoutWr);
        CloseHandle(hChildStderrRd);
        CloseHandle(hChildStderrWr);
        return -1;
    }
    
    /* Close handles not needed in parent */
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
    CloseHandle(hChildStderrWr);
    CloseHandle(pi.hThread);
    
    /* Store handles in Windows-compatible fields */
    server->process_handle = pi.hProcess;
    server->stdin_handle = hChildStdinWr;
    server->stdout_handle = hChildStdoutRd;
    server->stderr_handle = hChildStderrRd;
    server->running = 1;
    
#ifndef _WIN32
    /* POSIX implementation */
    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
    
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    }
    
    if (pid == 0) {
        /* Child process - LSP server */
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        
        /* Prepare arguments */
        char **exec_args = malloc(sizeof(char*) * (server->config->arg_count + 2));
        exec_args[0] = server->config->command;
        for (int i = 0; i < server->config->arg_count; i++) {
            exec_args[i + 1] = server->config->args[i];
        }
        exec_args[server->config->arg_count + 1] = NULL;
        
        execvp(server->config->command, exec_args);
        exit(1);
    }
    
    /* Parent process */
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
    
    server->pid = pid;
    server->stdin_fd = stdin_pipe[1];
    server->stdout_fd = stdout_pipe[0];
    server->stderr_fd = stderr_pipe[0];
    server->running = 1;
    
    /* Make pipes non-blocking */
    fcntl(server->stdout_fd, F_SETFL, O_NONBLOCK);
    fcntl(server->stderr_fd, F_SETFL, O_NONBLOCK);
#else
    /* Platform not supported - just return error */
    return -1;
#endif
    
    /* Send initialize request */
    xcodex_lsp_send_initialize(server);
    
    return 0;
}

/* Stop LSP server */
int xcodex_lsp_stop_server(const char *server_name) {
    lsp_server_t *server = NULL;
    
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        if (strcmp(g_lsp_client.servers[i].config->name, server_name) == 0) {
            server = &g_lsp_client.servers[i];
            break;
        }
    }
    
    if (!server || !server->running) {
        return -1;
    }
    
    /* Send shutdown request */
    char *shutdown_msg = format_json_rpc_request(server->next_request_id++, LSP_SHUTDOWN, NULL);
    
#ifdef _WIN32
    DWORD written;
    WriteFile(server->stdin_handle, shutdown_msg, strlen(shutdown_msg), &written, NULL);
    free(shutdown_msg);
    
    /* Send exit notification */
    char *exit_msg = format_json_rpc_notification(LSP_EXIT, NULL);
    WriteFile(server->stdin_handle, exit_msg, strlen(exit_msg), &written, NULL);
    free(exit_msg);
    
    /* Close handles */
    CloseHandle(server->stdin_handle);
    CloseHandle(server->stdout_handle);
    CloseHandle(server->stderr_handle);
    
    /* Terminate process */
    TerminateProcess(server->process_handle, 0);
    WaitForSingleObject(server->process_handle, 5000);
    CloseHandle(server->process_handle);
#else
    write(server->stdin_fd, shutdown_msg, strlen(shutdown_msg));
    free(shutdown_msg);
    
    /* Send exit notification */
    char *exit_msg = format_json_rpc_notification(LSP_EXIT, NULL);
    write(server->stdin_fd, exit_msg, strlen(exit_msg));
    free(exit_msg);
    
    /* Close pipes */
    close(server->stdin_fd);
    close(server->stdout_fd);
    close(server->stderr_fd);
    
    /* Kill process if still running */
    kill(server->pid, SIGTERM);
    waitpid(server->pid, NULL, 0);
#endif
    
    server->running = 0;
    server->initialized = 0;
    
    return 0;
}

/* Get appropriate server for file */
lsp_server_t* xcodex_lsp_get_server_for_file(const char *filename) {
    if (!filename) return NULL;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return NULL;
    
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        lsp_server_t *server = &g_lsp_client.servers[i];
        if (!server->config->enabled || !server->running) continue;
        
        for (int j = 0; j < server->config->pattern_count; j++) {
            if (strstr(ext, server->config->file_patterns[j])) {
                return server;
            }
        }
    }
    
    return NULL;
}

/* Send initialize request */
int xcodex_lsp_send_initialize(lsp_server_t *server) {
    if (!server || !server->running) return -1;
    
    char params[1024];
    snprintf(params, sizeof(params),
        "{"
        "\"processId\": %d,"
        "\"rootUri\": \"file://%s\","
        "\"capabilities\": {"
            "\"textDocument\": {"
                "\"completion\": {\"dynamicRegistration\": true},"
                "\"hover\": {\"dynamicRegistration\": true},"
                "\"definition\": {\"dynamicRegistration\": true},"
                "\"references\": {\"dynamicRegistration\": true},"
                "\"documentSymbol\": {\"dynamicRegistration\": true},"
                "\"codeAction\": {\"dynamicRegistration\": true},"
                "\"formatting\": {\"dynamicRegistration\": true},"
                "\"rename\": {\"dynamicRegistration\": true},"
                "\"signatureHelp\": {\"dynamicRegistration\": true}"
            "},"
            "\"workspace\": {"
                "\"symbol\": {\"dynamicRegistration\": true},"
                "\"didChangeConfiguration\": {\"dynamicRegistration\": true}"
            "}"
        "}"
        "}",
        getpid(), g_lsp_client.workspace_root);
    
    char *msg = format_json_rpc_request(server->next_request_id++, LSP_INITIALIZE, params);
    
    /* Use Windows handles for communication */
#ifdef _WIN32
    DWORD written;
    BOOL result = WriteFile(server->stdin_handle, msg, strlen(msg), &written, NULL);
    free(msg);
    return (result && written > 0) ? 0 : -1;
#else
    int result = write(server->stdin_fd, msg, strlen(msg));
    free(msg);
    return result > 0 ? 0 : -1;
#endif
}

/* Send text document open notification */
int xcodex_lsp_send_text_document_open(lsp_server_t *server, const char *uri, 
                                       const char *language, const char *content) {
    if (!server || !server->initialized || !uri || !content) return -1;
    
    char *escaped_content = malloc(strlen(content) * 2 + 1);
    int j = 0;
    for (int i = 0; content[i]; i++) {
        if (content[i] == '"' || content[i] == '\\') {
            escaped_content[j++] = '\\';
        }
        escaped_content[j++] = content[i];
    }
    escaped_content[j] = '\0';
    
    char *params = malloc(strlen(uri) + strlen(language) + strlen(escaped_content) + 256);
    sprintf(params,
        "{"
        "\"textDocument\": {"
            "\"uri\": \"%s\","
            "\"languageId\": \"%s\","
            "\"version\": 1,"
            "\"text\": \"%s\""
        "}"
        "}",
        uri, language, escaped_content);
    
    char *msg = format_json_rpc_notification(LSP_TEXT_DOCUMENT_OPEN, params);
    
#ifdef _WIN32
    DWORD written;
    BOOL result = WriteFile(server->stdin_handle, msg, strlen(msg), &written, NULL);
    free(escaped_content);
    free(params);
    free(msg);
    return (result && written > 0) ? 0 : -1;
#else
    int result = write(server->stdin_fd, msg, strlen(msg));
    
    free(escaped_content);
    free(params);
    free(msg);
    
    return result > 0 ? 0 : -1;
#endif
}

/* Convert file path to URI */
char* xcodex_lsp_file_to_uri(const char *filepath) {
    if (!filepath) return NULL;
    
    char *uri = malloc(strlen(filepath) + 8);
    sprintf(uri, "file://%s", filepath);
    return uri;
}

/* Load default LSP server configurations */
void xcodex_lsp_load_default_servers(void) {
    xcodex_lsp_register_clangd();
    xcodex_lsp_register_rust_analyzer();
    xcodex_lsp_register_pylsp();
    xcodex_lsp_register_typescript_language_server();
    xcodex_lsp_register_gopls();
}

/* Register clangd for C/C++ */
void xcodex_lsp_register_clangd(void) {
    char *args[] = {"--background-index", "--clang-tidy"};
    char *patterns[] = {".c", ".cpp", ".h", ".hpp", ".cc", ".cxx"};
    xcodex_lsp_add_server_config("clangd", "clangd", args, 2, patterns, 6);
}

/* Register rust-analyzer for Rust */
void xcodex_lsp_register_rust_analyzer(void) {
    char *args[] = {};
    char *patterns[] = {".rs"};
    xcodex_lsp_add_server_config("rust-analyzer", "rust-analyzer", args, 0, patterns, 1);
}

/* Register pylsp for Python */
void xcodex_lsp_register_pylsp(void) {
    char *args[] = {};
    char *patterns[] = {".py", ".pyw"};
    xcodex_lsp_add_server_config("pylsp", "pylsp", args, 0, patterns, 2);
}

/* Register TypeScript language server */
void xcodex_lsp_register_typescript_language_server(void) {
    char *args[] = {"--stdio"};
    char *patterns[] = {".ts", ".js", ".tsx", ".jsx"};
    xcodex_lsp_add_server_config("typescript-language-server", "typescript-language-server", 
                                 args, 1, patterns, 4);
}

/* Register gopls for Go */
void xcodex_lsp_register_gopls(void) {
    char *args[] = {};
    char *patterns[] = {".go"};
    xcodex_lsp_add_server_config("gopls", "gopls", args, 0, patterns, 1);
}

/* Check if server is running */
int xcodex_lsp_is_server_running(const char *server_name) {
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        if (strcmp(g_lsp_client.servers[i].config->name, server_name) == 0) {
            return g_lsp_client.servers[i].running;
        }
    }
    return 0;
}

/* Auto-start appropriate LSP server for file */
int xcodex_lsp_auto_start_for_file(const char *filename) {
    if (!g_lsp_client.auto_start || !filename) return -1;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return -1;
    
    for (int i = 0; i < g_lsp_client.server_count; i++) {
        lsp_server_t *server = &g_lsp_client.servers[i];
        if (!server->config->enabled || server->running) continue;
        
        for (int j = 0; j < server->config->pattern_count; j++) {
            if (strstr(ext, server->config->file_patterns[j])) {
                return xcodex_lsp_start_server(server->config->name);
            }
        }
    }
    
    return -1;
}

#endif

#endif /* XCODEX_ENABLE_LSP */
