#ifdef XCODEX_ENABLE_LUA

#include "xcodex_lua.h"
#include "xcodex_types.h"
#include "xcodex.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if XCODEX_POSIX
#include <dirent.h>
#include <sys/stat.h>
#endif

#if XCODEX_WINDOWS
#include <windows.h>
#endif

xcodex_plugin_manager_t g_plugin_manager = {0};

/* Forward declaration */
void editorSetStatusMessage(const char *fmt, ...);

/* Lua API functions that plugins can call */

/* Safety check function */
static int check_editor_state(lua_State *L) {
    if (!E.row && E.numrows > 0) {
        luaL_error(L, "Editor in invalid state: rows is NULL but numrows > 0");
        return 0;
    }
    return 1;
}

static int lua_xcodex_get_current_line(lua_State *L) {
    if (!check_editor_state(L)) return 0;
    
    int filerow = E.rowoff + E.cy;
    if (filerow >= 0 && filerow < E.numrows && E.row) {
        lua_pushstring(L, E.row[filerow].chars ? E.row[filerow].chars : "");
    } else {
        lua_pushstring(L, "");
    }
    return 1;
}

static int lua_xcodex_get_cursor_pos(lua_State *L) {
    lua_createtable(L, 0, 2);
    lua_pushinteger(L, E.rowoff + E.cy + 1);  // 1-based line number
    lua_setfield(L, -2, "line");
    lua_pushinteger(L, E.coloff + E.cx + 1);  // 1-based column
    lua_setfield(L, -2, "col");
    return 1;
}

static int lua_xcodex_set_cursor_pos(lua_State *L) {
    if (!check_editor_state(L)) return 0;
    
    int line = luaL_checkinteger(L, 1) - 1;  // Convert to 0-based
    int col = luaL_checkinteger(L, 2) - 1;
    
    // Validate bounds
    if (line < 0) line = 0;
    if (col < 0) col = 0;
    
    if (line < E.numrows) {
        E.cy = 0;
        E.rowoff = line;
        E.cx = col;
        E.coloff = 0;
        
        if (line < E.screenrows) {
            E.cy = line;
            E.rowoff = 0;
        }
        
        // Adjust column if necessary
        if (line < E.numrows && E.row) {
            erow *row = &E.row[line];
            if (col > row->size) col = row->size;
            
            int effective_cols = E.screencols - E.line_numbers_width;
            if (effective_cols < 1) effective_cols = 1;
            
            if (col >= effective_cols) {
                E.coloff = col - effective_cols + 1;
                E.cx = effective_cols - 1;
            } else {
                E.cx = col;
                E.coloff = 0;
            }
        }
    }
    return 0;
}

static int lua_xcodex_insert_text(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    if (!text) return 0;
    
    size_t len = strlen(text);
    
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\n') {
            editorInsertNewline();
        } else {
            editorInsertChar(text[i]);
        }
    }
    return 0;
}

static int lua_xcodex_get_file_content(lua_State *L) {
    int buflen;
    char *content = editorRowsToString(&buflen);
    if (content) {
        lua_pushstring(L, content);
        free(content);
    } else {
        lua_pushstring(L, "");
    }
    return 1;
}

static int lua_xcodex_set_status_message(lua_State *L) {
    const char *msg = luaL_checkstring(L, 1);
    if (msg) {
        editorSetStatusMessage("%s", msg);
    }
    return 0;
}

static int lua_xcodex_get_filename(lua_State *L) {
    if (E.filename) {
        lua_pushstring(L, E.filename);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_xcodex_get_mode(lua_State *L) {
    lua_pushstring(L, xcodex_get_mode_name(E.mode));
    return 1;
}

/* Registry of Lua API functions */
static const xcodex_lua_reg_t xcodex_lua_api[] = {
    {"get_current_line", lua_xcodex_get_current_line},
    {"get_cursor_pos", lua_xcodex_get_cursor_pos},
    {"set_cursor_pos", lua_xcodex_set_cursor_pos},
    {"insert_text", lua_xcodex_insert_text},
    {"get_file_content", lua_xcodex_get_file_content},
    {"set_status_message", lua_xcodex_set_status_message},
    {"get_filename", lua_xcodex_get_filename},
    {"get_mode", lua_xcodex_get_mode},
    {NULL, NULL}
};

/* Initialize Lua plugin system */
int xcodex_lua_init(void) {
    g_plugin_manager.global_L = luaL_newstate();
    if (!g_plugin_manager.global_L) {
        return -1;
    }
    
    luaL_openlibs(g_plugin_manager.global_L);
    
    /* Register XCodex API functions */
    lua_newtable(g_plugin_manager.global_L);
    for (const xcodex_lua_reg_t *reg = xcodex_lua_api; reg->name; reg++) {
        lua_pushcfunction(g_plugin_manager.global_L, reg->func);
        lua_setfield(g_plugin_manager.global_L, -2, reg->name);
    }
    lua_setglobal(g_plugin_manager.global_L, "xcodex");
    
    /* Initialize plugin array */
    g_plugin_manager.capacity = 10;
    g_plugin_manager.plugins = malloc(sizeof(xcodex_plugin_t) * g_plugin_manager.capacity);
    if (!g_plugin_manager.plugins) {
        lua_close(g_plugin_manager.global_L);
        g_plugin_manager.global_L = NULL;
        return -1;
    }
    
    memset(g_plugin_manager.plugins, 0, sizeof(xcodex_plugin_t) * g_plugin_manager.capacity);
    g_plugin_manager.count = 0;
    
    return 0;
}

/* Load a plugin from file */
int xcodex_lua_load_plugin(const char *plugin_path) {
    if (!plugin_path || !g_plugin_manager.plugins) {
        return -1;
    }
    
    if (g_plugin_manager.count >= g_plugin_manager.capacity) {
        g_plugin_manager.capacity *= 2;
        xcodex_plugin_t *new_plugins = realloc(g_plugin_manager.plugins,
            sizeof(xcodex_plugin_t) * g_plugin_manager.capacity);
        if (!new_plugins) return -1;
        
        g_plugin_manager.plugins = new_plugins;
        
        // Initialize new entries
        for (int i = g_plugin_manager.count; i < g_plugin_manager.capacity; i++) {
            memset(&g_plugin_manager.plugins[i], 0, sizeof(xcodex_plugin_t));
        }
    }
    
    xcodex_plugin_t *plugin = &g_plugin_manager.plugins[g_plugin_manager.count];
    plugin->L = luaL_newstate();
    if (!plugin->L) return -1;
    
    luaL_openlibs(plugin->L);
    
    /* Copy API functions to plugin state */
    lua_newtable(plugin->L);
    for (const xcodex_lua_reg_t *reg = xcodex_lua_api; reg->name; reg++) {
        lua_pushcfunction(plugin->L, reg->func);
        lua_setfield(plugin->L, -2, reg->name);
    }
    lua_setglobal(plugin->L, "xcodex");
    
    /* Load and execute plugin file */
    if (luaL_loadfile(plugin->L, plugin_path) != LUA_OK) {
        const char *error = lua_tostring(plugin->L, -1);
        editorSetStatusMessage("Plugin load error: %s", error ? error : "Unknown error");
        lua_close(plugin->L);
        plugin->L = NULL;
        return -1;
    }
    
    if (lua_pcall(plugin->L, 0, 0, 0) != LUA_OK) {
        const char *error = lua_tostring(plugin->L, -1);
        editorSetStatusMessage("Plugin exec error: %s", error ? error : "Unknown error");
        lua_close(plugin->L);
        plugin->L = NULL;
        return -1;
    }
    
    /* Get plugin metadata */
    lua_getglobal(plugin->L, "plugin_info");
    if (lua_istable(plugin->L, -1)) {
        lua_getfield(plugin->L, -1, "name");
        const char *name = lua_tostring(plugin->L, -1);
        plugin->name = name ? strdup(name) : strdup("Unknown");
        lua_pop(plugin->L, 1);
        
        lua_getfield(plugin->L, -1, "version");
        const char *version = lua_tostring(plugin->L, -1);
        plugin->version = version ? strdup(version) : strdup("1.0");
        lua_pop(plugin->L, 1);
        
        lua_getfield(plugin->L, -1, "author");
        const char *author = lua_tostring(plugin->L, -1);
        plugin->author = author ? strdup(author) : strdup("Unknown");
        lua_pop(plugin->L, 1);
        
        lua_getfield(plugin->L, -1, "description");
        const char *description = lua_tostring(plugin->L, -1);
        plugin->description = description ? strdup(description) : strdup("No description");
        lua_pop(plugin->L, 1);
    } else {
        plugin->name = strdup("Unknown");
        plugin->version = strdup("1.0");
        plugin->author = strdup("Unknown");
        plugin->description = strdup("No description");
    }
    lua_pop(plugin->L, 1);
    
    plugin->path = strdup(plugin_path);
    plugin->loaded = 1;
    plugin->enabled = 1;
    
    g_plugin_manager.count++;
    
    /* Call on_load hook if it exists */
    lua_getglobal(plugin->L, XCODEX_HOOK_ON_LOAD);
    if (lua_isfunction(plugin->L, -1)) {
        lua_pcall(plugin->L, 0, 0, 0);
    } else {
        lua_pop(plugin->L, 1);
    }
    
    editorSetStatusMessage("Loaded plugin: %s v%s", plugin->name, plugin->version);
    return 0;
}

/* Call a hook in all loaded plugins */
void xcodex_lua_call_hook(const char *hook_name, const char *args) {
    if (!hook_name || !g_plugin_manager.plugins) return;
    
    for (int i = 0; i < g_plugin_manager.count; i++) {
        xcodex_plugin_t *plugin = &g_plugin_manager.plugins[i];
        if (!plugin->loaded || !plugin->enabled || !plugin->L) continue;
        
        lua_getglobal(plugin->L, hook_name);
        if (lua_isfunction(plugin->L, -1)) {
            if (args) {
                lua_pushstring(plugin->L, args);
                if (lua_pcall(plugin->L, 1, 0, 0) != LUA_OK) {
                    // Silently ignore errors in hooks
                    lua_pop(plugin->L, 1);
                }
            } else {
                if (lua_pcall(plugin->L, 0, 0, 0) != LUA_OK) {
                    // Silently ignore errors in hooks
                    lua_pop(plugin->L, 1);
                }
            }
        } else {
            lua_pop(plugin->L, 1);
        }
    }
}

/* Load plugins from directory */
int xcodex_lua_load_plugins_from_dir(const char *plugin_dir) {
    if (!plugin_dir) return -1;
    
    int loaded_count = 0;
    
#if XCODEX_WINDOWS
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*.lua", plugin_dir);
    
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(search_path, &find_data);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s\\%s", plugin_dir, find_data.cFileName);
                
                if (xcodex_lua_load_plugin(full_path) == 0) {
                    loaded_count++;
                }
            }
        } while (FindNextFile(hFind, &find_data));
        
        FindClose(hFind);
    }
#endif

#if XCODEX_POSIX
    DIR *dir = opendir(plugin_dir);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
                // Check if file ends with .lua
                size_t len = strlen(entry->d_name);
                if (len > 4 && strcmp(entry->d_name + len - 4, ".lua") == 0) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s/%s", plugin_dir, entry->d_name);
                    
                    if (xcodex_lua_load_plugin(full_path) == 0) {
                        loaded_count++;
                    }
                }
            }
        }
        closedir(dir);
    }
#endif
    
    if (loaded_count > 0) {
        editorSetStatusMessage("Loaded %d plugin(s) from %s", loaded_count, plugin_dir);
    }
    
    return loaded_count;
}

/* Cleanup Lua system */
void xcodex_lua_cleanup(void) {
    if (g_plugin_manager.plugins) {
        for (int i = 0; i < g_plugin_manager.count; i++) {
            xcodex_plugin_t *plugin = &g_plugin_manager.plugins[i];
            if (plugin->L) {
                lua_close(plugin->L);
                plugin->L = NULL;
            }
            free(plugin->name);
            free(plugin->version);
            free(plugin->author);
            free(plugin->description);
            free(plugin->path);
        }
        free(g_plugin_manager.plugins);
        g_plugin_manager.plugins = NULL;
    }
    
    if (g_plugin_manager.global_L) {
        lua_close(g_plugin_manager.global_L);
        g_plugin_manager.global_L = NULL;
    }
    
    memset(&g_plugin_manager, 0, sizeof(g_plugin_manager));
}

/* Auto-load plugins from default locations */
int xcodex_lua_auto_load_plugins(void) {
    int loaded = 0;
    
    /* Try to load from current directory's plugins folder */
    if (xcodex_lua_load_plugin_directory("plugins") > 0) {
        loaded = 1;
    }
    
    /* Try to load from ~/.xcodex/plugins */
    const char *home = getenv("HOME");
    if (home) {
        char plugin_path[512];
        snprintf(plugin_path, sizeof(plugin_path), "%s/.xcodex/plugins", home);
        if (xcodex_lua_load_plugin_directory(plugin_path) > 0) {
            loaded = 1;
        }
    }
    
    /* Try to load from system-wide location */
#if XCODEX_POSIX
    if (xcodex_lua_load_plugin_directory("/usr/share/xcodex/plugins") > 0) {
        loaded = 1;
    }
#elif XCODEX_WINDOWS
    if (xcodex_lua_load_plugin_directory("C:\\Program Files\\XShell\\plugins") > 0) {
        loaded = 1;
    }
#endif
    
    /* Set up default plugins if none were loaded */
    if (!loaded) {
        xcodex_lua_setup_default_plugins();
    }
    
    /* Call on_editor_init hook for all loaded plugins */
    xcodex_lua_call_hook(XCODEX_HOOK_ON_EDITOR_INIT, NULL);
    
    return loaded ? 0 : -1;
}

/* Load plugins from a specific directory */
int xcodex_lua_load_plugin_directory(const char *dir_path) {
    if (!dir_path) return -1;
    
    int loaded_count = 0;
    
#if XCODEX_POSIX
    DIR *dir = opendir(dir_path);
    if (!dir) return -1;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".lua") == 0) {
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
                
                if (xcodex_lua_load_plugin(full_path) == 0) {
                    loaded_count++;
                }
            }
        }
    }
    closedir(dir);
#endif

#if XCODEX_WINDOWS
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*.lua", dir_path);
    
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFile(search_path, &find_data);
    
    if (find_handle != INVALID_HANDLE_VALUE) {
        do {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s\\%s", dir_path, find_data.cFileName);
            
            if (xcodex_lua_load_plugin(full_path) == 0) {
                loaded_count++;
            }
        } while (FindNextFile(find_handle, &find_data));
        
        FindClose(find_handle);
    }
#endif
    
    if (loaded_count > 0) {
        editorSetStatusMessage("Auto-loaded %d plugins from %s", loaded_count, dir_path);
    }
    
    return loaded_count;
}

/* Setup default built-in plugins if no external plugins are found */
int xcodex_lua_setup_default_plugins(void) {
    /* Create built-in auto-pairs functionality */
    const char *autopairs_plugin = 
        "plugin_info = {\n"
        "    name = \"Built-in Auto Pairs\",\n"
        "    version = \"1.0\",\n"
        "    author = \"XCodex\",\n"
        "    description = \"Built-in bracket and quote pairing\"\n"
        "}\n"
        "\n"
        "local pairs_map = {\n"
        "    ['('] = ')',\n"
        "    ['['] = ']',\n"
        "    ['{'] = '}',\n"
        "    ['\"'] = '\"',\n"
        "    [\"'\"] = \"'\"\n"
        "}\n"
        "\n"
        "function on_char_insert(char)\n"
        "    if pairs_map[char] then\n"
        "        local pos = xcodex.get_cursor_pos()\n"
        "        xcodex.insert_text(pairs_map[char])\n"
        "        xcodex.set_cursor_pos(pos.line, pos.col)\n"
        "    end\n"
        "end\n"
        "\n"
        "function on_load()\n"
        "    xcodex.set_status_message(\"Built-in Auto Pairs enabled\")\n"
        "end\n";
    
    /* Load the built-in plugin */
    lua_State *L = luaL_newstate();
    if (!L) return -1;
    
    luaL_openlibs(L);
    
    /* Register API functions */
    const xcodex_lua_reg_t api_functions[] = {
        {"get_current_line", lua_xcodex_get_current_line},
        {"get_cursor_pos", lua_xcodex_get_cursor_pos},
        {"set_cursor_pos", lua_xcodex_set_cursor_pos},
        {"insert_text", lua_xcodex_insert_text},
        {"get_file_content", lua_xcodex_get_file_content},
        {"set_status_message", lua_xcodex_set_status_message},
        {"get_filename", lua_xcodex_get_filename},
        {"get_mode", lua_xcodex_get_mode},
        {NULL, NULL}
    };
    
    lua_newtable(L);
    for (int i = 0; api_functions[i].name; i++) {
        lua_pushcfunction(L, api_functions[i].func);
        lua_setfield(L, -2, api_functions[i].name);
    }
    lua_setglobal(L, "xcodex");
    
    /* Execute the plugin code */
    if (luaL_dostring(L, autopairs_plugin) != LUA_OK) {
        const char *error = lua_tostring(L, -1);
        editorSetStatusMessage("Built-in plugin error: %s", error);
        lua_close(L);
        return -1;
    }
    
    /* Add to plugin manager */
    if (g_plugin_manager.count >= g_plugin_manager.capacity) {
        g_plugin_manager.capacity = g_plugin_manager.capacity ? g_plugin_manager.capacity * 2 : 4;
        g_plugin_manager.plugins = realloc(g_plugin_manager.plugins, 
                                           sizeof(xcodex_plugin_t) * g_plugin_manager.capacity);
    }
    
    xcodex_plugin_t *plugin = &g_plugin_manager.plugins[g_plugin_manager.count++];
    memset(plugin, 0, sizeof(xcodex_plugin_t));
    plugin->name = strdup("Built-in Auto Pairs");
    plugin->version = strdup("1.0");
    plugin->author = strdup("XCodex");
    plugin->description = strdup("Built-in bracket and quote pairing");
    plugin->path = strdup("built-in");
    plugin->L = L;
    plugin->loaded = 1;
    plugin->enabled = 1;
    
    /* Call on_load hook */
    lua_getglobal(L, "on_load");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            const char *error = lua_tostring(L, -1);
            editorSetStatusMessage("Plugin on_load error: %s", error);
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
    
    editorSetStatusMessage("Built-in plugins initialized");
    return 0;
}

#endif /* XCODEX_ENABLE_LUA */
