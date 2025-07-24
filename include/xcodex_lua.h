#ifndef XCODEX_LUA_H
#define XCODEX_LUA_H

#include <stddef.h>  /* For NULL definition */

#ifdef XCODEX_ENABLE_LUA

/* Conditional Lua includes - only if available */
#ifdef HAVE_LUA
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
    #define LUA_AVAILABLE 1
#else
    /* Dummy Lua types for compilation without Lua */
    typedef struct lua_State lua_State;
    typedef int (*lua_CFunction) (lua_State *L);  /* Add lua_CFunction definition */
    #define LUA_AVAILABLE 0
    /* Define minimal Lua constants/functions as stubs */
    #define LUA_OK 0
    static inline lua_State* luaL_newstate(void) { return NULL; }
    static inline void luaL_openlibs(lua_State *L) { (void)L; }
    static inline int luaL_dofile(lua_State *L, const char *filename) { (void)L; (void)filename; return -1; }
    static inline void lua_close(lua_State *L) { (void)L; }
    static inline void lua_getglobal(lua_State *L, const char *name) { (void)L; (void)name; }
    static inline int lua_pcall(lua_State *L, int nargs, int nresults, int msgh) { (void)L; (void)nargs; (void)nresults; (void)msgh; return -1; }
    static inline const char* lua_tostring(lua_State *L, int index) { (void)L; (void)index; return ""; }
    static inline void lua_pop(lua_State *L, int n) { (void)L; (void)n; }
    static inline int lua_type(lua_State *L, int index) { (void)L; (void)index; return 0; }
    static inline void lua_pushstring(lua_State *L, const char *s) { (void)L; (void)s; }
    static inline void lua_pushinteger(lua_State *L, int n) { (void)L; (void)n; }
    /* Additional stub functions for compatibility */
    static inline int luaL_error(lua_State *L, const char *fmt, ...) { (void)L; (void)fmt; return 0; }
    static inline void lua_createtable(lua_State *L, int narr, int nrec) { (void)L; (void)narr; (void)nrec; }
    static inline void lua_setfield(lua_State *L, int idx, const char *k) { (void)L; (void)idx; (void)k; }
    static inline int luaL_checkinteger(lua_State *L, int arg) { (void)L; (void)arg; return 0; }
    static inline const char* luaL_checkstring(lua_State *L, int arg) { (void)L; (void)arg; return ""; }
    static inline void lua_pushnil(lua_State *L) { (void)L; }
    static inline void lua_newtable(lua_State *L) { (void)L; }
    static inline void lua_pushcfunction(lua_State *L, lua_CFunction f) { (void)L; (void)f; }
    static inline void lua_setglobal(lua_State *L, const char *name) { (void)L; (void)name; }
    static inline int luaL_loadfile(lua_State *L, const char *filename) { (void)L; (void)filename; return -1; }
    static inline int lua_istable(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
    static inline void lua_getfield(lua_State *L, int idx, const char *k) { (void)L; (void)idx; (void)k; }
    static inline int lua_isfunction(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
    static inline int luaL_dostring(lua_State *L, const char *str) { (void)L; (void)str; return -1; }
#endif

/* Plugin system structures */
typedef struct {
    char *name;
    char *version;
    char *author;
    char *description;
    char *path;
    lua_State *L;
    int loaded;
    int enabled;
} xcodex_plugin_t;

typedef struct {
    xcodex_plugin_t *plugins;
    int count;
    int capacity;
    lua_State *global_L;
} xcodex_plugin_manager_t;

/* Plugin API function signatures */
typedef struct {
    const char *name;
    lua_CFunction func;
} xcodex_lua_reg_t;

/* Function declarations */
int xcodex_lua_init(void);
void xcodex_lua_cleanup(void);
int xcodex_lua_load_plugin(const char *plugin_path);
int xcodex_lua_unload_plugin(const char *plugin_name);
int xcodex_lua_reload_plugin(const char *plugin_name);
void xcodex_lua_call_hook(const char *hook_name, const char *args);
int xcodex_lua_execute_command(const char *command);
int xcodex_lua_load_plugins_from_dir(const char *plugin_dir);

/* Plugin hooks */
#define XCODEX_HOOK_ON_LOAD         "on_load"
#define XCODEX_HOOK_ON_SAVE         "on_save"
#define XCODEX_HOOK_ON_CHAR_INSERT  "on_char_insert"
#define XCODEX_HOOK_ON_CHAR_DELETE  "on_char_delete"
#define XCODEX_HOOK_ON_LINE_INSERT  "on_line_insert"
#define XCODEX_HOOK_ON_LINE_DELETE  "on_line_delete"
#define XCODEX_HOOK_ON_MODE_CHANGE  "on_mode_change"
#define XCODEX_HOOK_ON_CURSOR_MOVE  "on_cursor_move"
#define XCODEX_HOOK_ON_SYNTAX_CHANGE "on_syntax_change"
#define XCODEX_HOOK_ON_FILE_OPEN    "on_file_open"
#define XCODEX_HOOK_ON_EDITOR_INIT  "on_editor_init"

/* Auto-loading functions */
int xcodex_lua_auto_load_plugins(void);
int xcodex_lua_load_plugin_directory(const char *dir_path);
int xcodex_lua_setup_default_plugins(void);

/* Plugin manager */
extern xcodex_plugin_manager_t g_plugin_manager;

#endif /* XCODEX_ENABLE_LUA */

#endif
