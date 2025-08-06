#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/* Global configuration instances */
config_t xshell_config = {0};
config_t xcodex_config = {0};

/* Initialize configuration structure */
int config_init(config_t *config) {
    if (!config) return -1;
    
    config->pairs = malloc(sizeof(config_pair_t) * 16);
    if (!config->pairs) return -1;
    
    config->count = 0;
    config->capacity = 16;
    return 0;
}

/* Free configuration structure */
void config_free(config_t *config) {
    if (!config) return;
    
    for (int i = 0; i < config->count; i++) {
        free(config->pairs[i].key);
        free(config->pairs[i].value);
        free(config->pairs[i].description);
        free(config->pairs[i].default_value);
    }
    free(config->pairs);
    config->pairs = NULL;
    config->count = 0;
    config->capacity = 0;
}

/* Trim whitespace from a string */
static char *trim_whitespace(char *str) {
    char *end;
    
    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    /* Write new null terminator */
    end[1] = '\0';
    
    return str;
}

/* Load configuration from key-value file */
int config_load_file(config_t *config, const char *filename) {
    if (!config || !filename) return -1;
    
    FILE *file = fopen(filename, "r");
    if (!file) return -1;
    
    char line[1024];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines and comments */
        char *trimmed = trim_whitespace(line);
        if (!*trimmed || *trimmed == '#') continue;
        
        /* Find equals sign */
        char *equals = strchr(trimmed, '=');
        if (!equals) {
            printf("Warning: Invalid syntax in %s line %d: %s\n", filename, line_num, trimmed);
            continue;
        }
        
        /* Split key and value */
        *equals = '\0';
        char *key = trim_whitespace(trimmed);
        char *value = trim_whitespace(equals + 1);
        
        if (*key && *value) {
            /* Check if key already exists and is readonly before trying to set it */
            int is_readonly = 0;
            for (int i = 0; i < config->count; i++) {
                if (strcmp(config->pairs[i].key, key) == 0) {
                    if (config->pairs[i].is_readonly) {
                        is_readonly = 1;
                        break;
                    }
                }
            }
            
            if (!is_readonly) {
                config_set(config, key, value);
            }
            /* Silently skip read-only keys when loading from file */
        }
    }
    
    fclose(file);
    return 0;
}

/* Validate configuration value based on type */
int config_validate_value(const char *value, config_type_t type) {
    if (!value) return 0;
    
    switch (type) {
        case CONFIG_TYPE_STRING:
            return 1; // Any string is valid
            
        case CONFIG_TYPE_INT: {
            char *endptr;
            strtol(value, &endptr, 10);
            return *endptr == '\0'; // Valid if entire string was consumed
        }
        
        case CONFIG_TYPE_BOOL:
            return (strcmp(value, "true") == 0 || strcmp(value, "false") == 0 ||
                    strcmp(value, "1") == 0 || strcmp(value, "0") == 0 ||
                    strcmp(value, "yes") == 0 || strcmp(value, "no") == 0);
                    
        case CONFIG_TYPE_FLOAT: {
            char *endptr;
            strtof(value, &endptr);
            return *endptr == '\0'; // Valid if entire string was consumed
        }
        
        default:
            return 0;
    }
}

/* Save configuration to key-value file */
int config_save_file(config_t *config, const char *filename) {
    if (!config || !filename) return -1;
    
    FILE *file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "# Configuration file\n");
    fprintf(file, "# Format: key=value\n\n");
    
    for (int i = 0; i < config->count; i++) {
        fprintf(file, "%s=%s\n", config->pairs[i].key, config->pairs[i].value);
    }
    
    fclose(file);
    return 0;
}

/* Set configuration value */
int config_set(config_t *config, const char *key, const char *value) {
    return config_set_with_type(config, key, value, CONFIG_TYPE_STRING, NULL);
}

/* Set configuration value with type and description */
int config_set_with_type(config_t *config, const char *key, const char *value, config_type_t type, const char *description) {
    if (!config || !key || !value) return -1;
    
    /* Validate value based on type */
    if (!config_validate_value(value, type)) {
        printf("Warning: Invalid value '%s' for type %d\n", value, type);
        return -1;
    }
    
    /* Check if key already exists */
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            /* Check if it's readonly */
            if (config->pairs[i].is_readonly) {
                printf("Error: Configuration key '%s' is read-only\n", key);
                return -1;
            }
            
            free(config->pairs[i].value);
            config->pairs[i].value = strdup(value);
            config->pairs[i].type = type;
            
            /* Update description if provided */
            if (description) {
                free(config->pairs[i].description);
                config->pairs[i].description = strdup(description);
            }
            return 0;
        }
    }
    
    /* Expand capacity if needed */
    if (config->count >= config->capacity) {
        config->capacity *= 2;
        config->pairs = realloc(config->pairs, sizeof(config_pair_t) * config->capacity);
        if (!config->pairs) return -1;
    }
    
    /* Add new pair */
    config->pairs[config->count].key = strdup(key);
    config->pairs[config->count].value = strdup(value);
    config->pairs[config->count].type = type;
    config->pairs[config->count].description = description ? strdup(description) : NULL;
    config->pairs[config->count].default_value = NULL;
    config->pairs[config->count].is_readonly = 0;
    config->count++;
    
    return 0;
}

/* Get configuration value */
const char *config_get(config_t *config, const char *key) {
    if (!config || !key) return NULL;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].value;
        }
    }
    
    return NULL;
}

/* Get configuration value with default */
const char *config_get_default(config_t *config, const char *key, const char *default_value) {
    const char *value = config_get(config, key);
    return value ? value : default_value;
}

/* Get configuration value as integer */
int config_get_int(config_t *config, const char *key, int default_value) {
    const char *value = config_get(config, key);
    if (!value) return default_value;
    
    char *endptr;
    long result = strtol(value, &endptr, 10);
    return (*endptr == '\0') ? (int)result : default_value;
}

/* Get configuration value as boolean */
int config_get_bool(config_t *config, const char *key, int default_value) {
    const char *value = config_get(config, key);
    if (!value) return default_value;
    
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) {
        return 1;
    } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0) {
        return 0;
    }
    
    return default_value;
}

/* Get configuration value as float */
float config_get_float(config_t *config, const char *key, float default_value) {
    const char *value = config_get(config, key);
    if (!value) return default_value;
    
    char *endptr;
    float result = strtof(value, &endptr);
    return (*endptr == '\0') ? result : default_value;
}

/* Remove configuration value */
int config_remove(config_t *config, const char *key) {
    if (!config || !key) return -1;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            /* Check if it's readonly */
            if (config->pairs[i].is_readonly) {
                printf("Error: Configuration key '%s' is read-only and cannot be removed\n", key);
                return -1;
            }
            
            free(config->pairs[i].key);
            free(config->pairs[i].value);
            free(config->pairs[i].description);
            free(config->pairs[i].default_value);
            
            /* Move remaining elements */
            for (int j = i; j < config->count - 1; j++) {
                config->pairs[j] = config->pairs[j + 1];
            }
            config->count--;
            return 0;
        }
    }
    
    return -1;
}

/* Print all configuration values */
void config_print_all(config_t *config) {
    if (!config) return;
    
    if (config->count == 0) {
        printf("No configuration values set.\n");
        return;
    }
    
    printf("Configuration values:\n");
    for (int i = 0; i < config->count; i++) {
        printf("  %s = %s\n", config->pairs[i].key, config->pairs[i].value);
    }
}

/* Print all configuration values with type information */
void config_print_with_types(config_t *config) {
    if (!config) return;
    
    if (config->count == 0) {
        printf("No configuration values set.\n");
        return;
    }
    
    const char *type_names[] = {"string", "int", "bool", "float"};
    
    printf("Configuration values (with types):\n");
    printf("%-20s %-10s %-10s %s\n", "Key", "Type", "Value", "Description");
    printf("%-20s %-10s %-10s %s\n", "---", "----", "-----", "-----------");
    
    for (int i = 0; i < config->count; i++) {
        const char *type_name = (config->pairs[i].type < 4) ? type_names[config->pairs[i].type] : "unknown";
        printf("%-20s %-10s %-10s %s%s\n", 
               config->pairs[i].key, 
               type_name,
               config->pairs[i].value,
               config->pairs[i].description ? config->pairs[i].description : "",
               config->pairs[i].is_readonly ? " (readonly)" : "");
    }
}

/* Load default configuration values */
int config_load_defaults(config_t *config, const char *type) {
    if (!config || !type) return -1;
    
    if (strcmp(type, "xshell") == 0) {
        config_set_with_type(config, "prompt", "xsh@{user}:{cwd}:{history}> ", CONFIG_TYPE_STRING, "Shell prompt format (supports {user}, {cwd}, {history} placeholders)");
        config_set_with_type(config, "prompt_style", "enhanced", CONFIG_TYPE_STRING, "Prompt style: 'simple', 'enhanced', or 'custom'");
        config_set_with_type(config, "history_size", "100", CONFIG_TYPE_INT, "Maximum number of history entries (XSH_HISTORY_SIZE)");
        config_set_with_type(config, "auto_complete", "true", CONFIG_TYPE_BOOL, "Enable tab auto-completion for commands");
        config_set_with_type(config, "case_sensitive", "false", CONFIG_TYPE_BOOL, "Case sensitive command matching");
        config_set_with_type(config, "color_output", "true", CONFIG_TYPE_BOOL, "Enable colored output in prompt");
        config_set_with_type(config, "theme", "default", CONFIG_TYPE_STRING, "Color theme (currently only affects prompt colors)");
        config_set_with_type(config, "save_history", "true", CONFIG_TYPE_BOOL, "Save command history (future feature)");
        config_set_with_type(config, "startup_banner", "true", CONFIG_TYPE_BOOL, "Show ASCII art banner on startup");
        
        /* Set system information as readonly */
        config_set_with_type(config, "version", XSHELL_VERSION, CONFIG_TYPE_STRING, "XShell version");
        /* Mark version as readonly */
        for (int i = 0; i < config->count; i++) {
            if (strcmp(config->pairs[i].key, "version") == 0) {
                config->pairs[i].is_readonly = 1;
                break;
            }
        }
        
    } else if (strcmp(type, "xcodex") == 0) {
        config_set_with_type(config, "line_numbers", "true", CONFIG_TYPE_BOOL, "Show line numbers in editor");
        config_set_with_type(config, "syntax_highlighting", "true", CONFIG_TYPE_BOOL, "Enable syntax highlighting");
        config_set_with_type(config, "theme", "xcodex_dark", CONFIG_TYPE_STRING, "Editor color theme");
        config_set_with_type(config, "modal_editing", "true", CONFIG_TYPE_BOOL, "Enable vim-like modal editing");
        config_set_with_type(config, "undo_levels", "100", CONFIG_TYPE_INT, "Number of undo levels (UNDO_STACK_SIZE)");
        config_set_with_type(config, "search_highlight", "true", CONFIG_TYPE_BOOL, "Highlight search matches");
        config_set_with_type(config, "auto_indent", "true", CONFIG_TYPE_BOOL, "Enable automatic indentation (future feature)");
        config_set_with_type(config, "tab_size", "4", CONFIG_TYPE_INT, "Tab size in spaces (display only)");
        config_set_with_type(config, "show_status_bar", "true", CONFIG_TYPE_BOOL, "Show status bar with file info and mode");
    }
    
    return 0;
}

/* Load configuration file with fallback to home directory and current directory */
int config_load_with_fallback(config_t *config, const char *config_file) {
    if (!config || !config_file) return -1;
    
    char home_path[512];
    const char *home = getenv("HOME");
    if (!home) {
#ifdef _WIN32
        home = getenv("USERPROFILE");
#endif
    }
    
    /* Try home directory first */
    if (home) {
#ifdef _WIN32
        snprintf(home_path, sizeof(home_path), "%s\\%s", home, config_file);
#else
        snprintf(home_path, sizeof(home_path), "%s/%s", home, config_file);
#endif
        
        if (access(home_path, F_OK) == 0) {
            int result = config_load_file(config, home_path);
            if (result == 0) {
                printf("Loaded configuration from %s\n", home_path);
            } else {
                fprintf(stderr, "Warning: Failed to load configuration from %s\n", home_path);
            }
            return result;
        }
    }
    
    /* Try current directory as fallback */
    if (access(config_file, F_OK) == 0) {
        int result = config_load_file(config, config_file);
        if (result == 0) {
            printf("Loaded configuration from current directory: %s\n", config_file);
        } else {
            fprintf(stderr, "Warning: Failed to load configuration from current directory: %s\n", config_file);
        }
        return result;
    }
    
    /* File not found in either location */
    return -1;
}

/* Load all configuration files */
int config_load_all_files(void) {
    int result = 0;
    
    /* Initialize configurations */
    if (config_init(&xshell_config) != 0 || config_init(&xcodex_config) != 0) {
        fprintf(stderr, "Error: Failed to initialize configuration structures\n");
        return -1;
    }
    
    /* Load default values first */
    if (config_load_defaults(&xshell_config, "xshell") != 0 || 
        config_load_defaults(&xcodex_config, "xcodex") != 0) {
        fprintf(stderr, "Error: Failed to load default configuration values\n");
        return -1;
    }
    
    /* Try to load from home directory */
    char home_path[512];
    const char *home = getenv("HOME");
    if (!home) {
#ifdef _WIN32
        home = getenv("USERPROFILE");
#endif
    }
    
    if (home) {
        /* Load XShell config from user home directory, fallback to current directory */
#ifdef _WIN32
        snprintf(home_path, sizeof(home_path), "%s\\%s", home, XSHELL_CONFIG_FILE);
#else
        snprintf(home_path, sizeof(home_path), "%s/%s", home, XSHELL_CONFIG_FILE);
#endif
        
        if (access(home_path, F_OK) == 0) {
            /* Config exists in home directory - load it (this will override defaults with user settings) */
            if (config_load_file(&xshell_config, home_path) == 0) {
                printf("Loaded XShell config from %s\n", home_path);
            } else {
                fprintf(stderr, "Warning: Failed to load XShell config from %s\n", home_path);
            }
        } else if (access(XSHELL_CONFIG_FILE, F_OK) == 0) {
            /* Try current directory as fallback */
            if (config_load_file(&xshell_config, XSHELL_CONFIG_FILE) == 0) {
                printf("Loaded XShell config from current directory: %s\n", XSHELL_CONFIG_FILE);
            } else {
                fprintf(stderr, "Warning: Failed to load XShell config from current directory\n");
            }
        } else {
            /* Config doesn't exist anywhere, create it in home directory with defaults */
            printf("Creating default XShell config at %s\n", home_path);
            if (config_save_file(&xshell_config, home_path) != 0) {
                fprintf(stderr, "Warning: Failed to create XShell config file at %s\n", home_path);
                result = -1;
            }
        }
        
        /* Load XCodex config from user home directory, fallback to current directory */
#ifdef _WIN32
        snprintf(home_path, sizeof(home_path), "%s\\%s", home, XCODEX_CONFIG_FILE);
#else
        snprintf(home_path, sizeof(home_path), "%s/%s", home, XCODEX_CONFIG_FILE);
#endif
        
        if (access(home_path, F_OK) == 0) {
            /* Config exists in home directory - load it (this will override defaults with user settings) */
            if (config_load_file(&xcodex_config, home_path) == 0) {
                printf("Loaded XCodex config from %s\n", home_path);
            } else {
                fprintf(stderr, "Warning: Failed to load XCodex config from %s\n", home_path);
            }
        } else if (access(XCODEX_CONFIG_FILE, F_OK) == 0) {
            /* Try current directory as fallback */
            if (config_load_file(&xcodex_config, XCODEX_CONFIG_FILE) == 0) {
                printf("Loaded XCodex config from current directory: %s\n", XCODEX_CONFIG_FILE);
            } else {
                fprintf(stderr, "Warning: Failed to load XCodex config from current directory\n");
            }
        } else {
            /* Config doesn't exist anywhere, create it in home directory with defaults */
            printf("Creating default XCodex config at %s\n", home_path);
            if (config_save_file(&xcodex_config, home_path) != 0) {
                fprintf(stderr, "Warning: Failed to create XCodex config file at %s\n", home_path);
                result = -1;
            }
        }
    } else {
        fprintf(stderr, "Warning: Could not determine user home directory for configuration files\n");
        /* Try current directory as last resort */
        if (access(XSHELL_CONFIG_FILE, F_OK) == 0) {
            if (config_load_file(&xshell_config, XSHELL_CONFIG_FILE) == 0) {
                printf("Loaded XShell config from current directory: %s\n", XSHELL_CONFIG_FILE);
            } else {
                fprintf(stderr, "Warning: Failed to load XShell config from current directory\n");
            }
        } else {
            printf("Using default XShell configuration (no config file found)\n");
        }
        
        if (access(XCODEX_CONFIG_FILE, F_OK) == 0) {
            if (config_load_file(&xcodex_config, XCODEX_CONFIG_FILE) == 0) {
                printf("Loaded XCodex config from current directory: %s\n", XCODEX_CONFIG_FILE);
            } else {
                fprintf(stderr, "Warning: Failed to load XCodex config from current directory\n");
            }
        } else {
            printf("Using default XCodex configuration (no config file found)\n");
        }
        result = -1;
    }
    
    return result;
}

/* Create default configuration files */
int config_create_default_files(void) {
    int result = 0;
    char home_path[512];
    const char *home = getenv("HOME");
    
    if (!home) {
#ifdef _WIN32
        home = getenv("USERPROFILE");
#endif
    }
    
    if (!home) {
        printf("Error: Could not determine home directory\n");
        return -1;
    }
    
    /* Create XShell config file */
#ifdef _WIN32
    snprintf(home_path, sizeof(home_path), "%s\\%s", home, XSHELL_CONFIG_FILE);
#else
    snprintf(home_path, sizeof(home_path), "%s/%s", home, XSHELL_CONFIG_FILE);
#endif
    if (access(home_path, F_OK) != 0) {
        config_t temp_config;
        config_init(&temp_config);
        config_load_defaults(&temp_config, "xshell");
        
        if (config_save_file(&temp_config, home_path) == 0) {
            printf("Created default XShell config: %s\n", home_path);
        } else {
            printf("Error: Could not create XShell config file\n");
            result = -1;
        }
        
        config_free(&temp_config);
    } else {
        printf("XShell config already exists: %s\n", home_path);
    }
    
    /* Create XCodex config file */
#ifdef _WIN32
    snprintf(home_path, sizeof(home_path), "%s\\%s", home, XCODEX_CONFIG_FILE);
#else
    snprintf(home_path, sizeof(home_path), "%s/%s", home, XCODEX_CONFIG_FILE);
#endif
    if (access(home_path, F_OK) != 0) {
        config_t temp_config;
        config_init(&temp_config);
        config_load_defaults(&temp_config, "xcodex");
        
        if (config_save_file(&temp_config, home_path) == 0) {
            printf("Created default XCodex config: %s\n", home_path);
        } else {
            printf("Error: Could not create XCodex config file\n");
            result = -1;
        }
        
        config_free(&temp_config);
    } else {
        printf("XCodex config already exists: %s\n", home_path);
    }
    
    return result;
}

/* Export configuration to INI format */
int config_export_ini(config_t *config, const char *filename) {
    if (!config || !filename) return -1;
    
    FILE *file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "; Configuration file exported in INI format\n");
    fprintf(file, "; Generated automatically - edit with care\n\n");
    
    for (int i = 0; i < config->count; i++) {
        if (config->pairs[i].description) {
            fprintf(file, "; %s\n", config->pairs[i].description);
        }
        if (config->pairs[i].is_readonly) {
            fprintf(file, "; [READONLY] ");
        }
        fprintf(file, "%s=%s\n", config->pairs[i].key, config->pairs[i].value);
        if (i < config->count - 1) fprintf(file, "\n");
    }
    
    fclose(file);
    return 0;
}

/* Export configuration to environment file format */
int config_export_env(config_t *config, const char *filename) {
    if (!config || !filename) return -1;
    
    FILE *file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "#!/bin/bash\n");
    fprintf(file, "# Configuration file exported as environment variables\n");
    fprintf(file, "# Source this file: source %s\n\n", filename);
    
    for (int i = 0; i < config->count; i++) {
        if (config->pairs[i].description) {
            fprintf(file, "# %s\n", config->pairs[i].description);
        }
        // Convert key to uppercase for environment variable convention
        char *env_key = strdup(config->pairs[i].key);
        for (char *p = env_key; *p; p++) {
            *p = toupper(*p);
        }
        
        fprintf(file, "export %s=\"%s\"\n", env_key, config->pairs[i].value);
        free(env_key);
        
        if (i < config->count - 1) fprintf(file, "\n");
    }
    
    fclose(file);
    return 0;
}

/* List available configuration keys for a type */
void config_list_available_keys(const char *type) {
    printf("Available configuration keys for %s:\n\n", type);
    
    if (strcmp(type, "xshell") == 0) {
        printf("%-20s %-10s %s\n", "Key", "Type", "Description");
        printf("%-20s %-10s %s\n", "---", "----", "-----------");
        printf("%-20s %-10s %s\n", "prompt", "string", "Shell prompt string");
        printf("%-20s %-10s %s\n", "history_size", "int", "Maximum history entries (currently 100)");
        printf("%-20s %-10s %s\n", "auto_complete", "bool", "Enable tab auto-completion for commands");
        printf("%-20s %-10s %s\n", "case_sensitive", "bool", "Case sensitive command matching");
        printf("%-20s %-10s %s\n", "color_output", "bool", "Enable colored output in prompt");
        printf("%-20s %-10s %s\n", "theme", "string", "Color theme (affects prompt colors)");
        printf("%-20s %-10s %s\n", "save_history", "bool", "Save command history (future feature)");
        printf("%-20s %-10s %s\n", "startup_banner", "bool", "Show ASCII art banner on startup");
        printf("%-20s %-10s %s\n", "version", "string", "XShell version (readonly)");
    } else if (strcmp(type, "xcodex") == 0) {
        printf("%-20s %-10s %s\n", "Key", "Type", "Description");
        printf("%-20s %-10s %s\n", "---", "----", "-----------");
        printf("%-20s %-10s %s\n", "line_numbers", "bool", "Show line numbers in editor");
        printf("%-20s %-10s %s\n", "syntax_highlighting", "bool", "Enable syntax highlighting");
        printf("%-20s %-10s %s\n", "theme", "string", "Editor color theme");
        printf("%-20s %-10s %s\n", "modal_editing", "bool", "Enable vim-like modal editing");
        printf("%-20s %-10s %s\n", "undo_levels", "int", "Number of undo levels (max 100)");
        printf("%-20s %-10s %s\n", "search_highlight", "bool", "Highlight search matches");
        printf("%-20s %-10s %s\n", "auto_indent", "bool", "Enable automatic indentation (future)");
        printf("%-20s %-10s %s\n", "tab_size", "int", "Tab size in spaces (display only)");
        printf("%-20s %-10s %s\n", "show_status_bar", "bool", "Show status bar with file info");
        
        printf("\nAvailable themes:\n");
        printf("  xcodex_dark, xcodex_light, gruvbox_dark\n");
        printf("  tokyo_night_dark, tokyo_night_light, tokyo_night_storm\n");
    }
    
    printf("\nValid values:\n");
    printf("  bool: true, false, 1, 0, yes, no\n");
    printf("  string: any text value\n");
    printf("  int: whole numbers (e.g., 1, 10, 1000)\n");
    printf("  float: decimal numbers (e.g., 1.0, 3.14, 30.5)\n");
    
    if (strcmp(type, "xshell") == 0) {
        printf("\nPrompt placeholders:\n");
        printf("  {user} - current username\n");
        printf("  {cwd} - current directory name\n");
        printf("  {history} - command history number\n");
        printf("\nPrompt styles:\n");
        printf("  simple - just 'xsh> '\n");
        printf("  enhanced - colored with user:dir:history\n");
        printf("  custom - use your own prompt format\n");
    };
}

/* Check if configuration has a specific key */
int config_has_key(config_t *config, const char *key) {
    if (!config || !key) return 0;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Get the type of a configuration key */
int config_get_type(config_t *config, const char *key) {
    if (!config || !key) return -1;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].type;
        }
    }
    return -1;
}

/* Get the description of a configuration key */
const char *config_get_description(config_t *config, const char *key) {
    if (!config || !key) return NULL;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].description;
        }
    }
    return NULL;
}

/* Set readonly status of a configuration key */
int config_set_readonly(config_t *config, const char *key, int readonly) {
    if (!config || !key) return -1;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            config->pairs[i].is_readonly = readonly;
            return 0;
        }
    }
    return -1;
}

/* Check if a configuration key is readonly */
int config_is_readonly(config_t *config, const char *key) {
    if (!config || !key) return 0;
    
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].is_readonly;
        }
    }
    return 0;
}

/* Backup configuration to a file */
void config_backup(config_t *config, const char *backup_file) {
    if (!config || !backup_file) return;
    
    config_export_ini(config, backup_file);
    printf("Configuration backed up to %s\n", backup_file);
}

/* Restore configuration from a backup file */
int config_restore(config_t *config, const char *backup_file) {
    if (!config || !backup_file) return -1;
    
    if (access(backup_file, F_OK) != 0) {
        printf("Backup file %s not found\n", backup_file);
        return -1;
    }
    
    int result = config_load_file(config, backup_file);
    if (result == 0) {
        printf("Configuration restored from %s\n", backup_file);
    } else {
        printf("Failed to restore configuration from %s\n", backup_file);
    }
    return result;
}

/* Show comprehensive help for configuration */
void config_show_help(const char *type) {
    printf("=== %s Configuration Help ===\n\n", type);
    
    printf("Configuration file location:\n");
    printf("  ~/.%src (Unix/Linux/Mac)\n", type);
    printf("  .\\%src (Windows)\n\n", type);
    
    printf("Basic commands:\n");
    printf("  config get <key>              - Get value of a key\n");
    printf("  config set <key> <value>      - Set value of a key\n");
    printf("  config remove <key>           - Remove a key\n");
    printf("  config list                   - List all configuration\n");
    printf("  config defaults               - Show default values\n");
    printf("  config validate               - Validate all settings\n");
    printf("  config backup <file>          - Backup configuration\n");
    printf("  config restore <file>         - Restore from backup\n");
    printf("  config export ini <file>      - Export to INI format\n");
    printf("  config export env <file>      - Export as environment vars\n");
    printf("  config reset <key>            - Reset key to default\n");
    printf("  config help                   - Show this help\n\n");
    
    printf("File format example:\n");
    printf("# Comment lines start with #\n");
    printf("key=value\n");
    printf("another_key=another_value\n\n");
    
    config_list_available_keys(type);
}
