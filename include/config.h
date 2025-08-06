#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Configuration value types */
typedef enum {
    CONFIG_TYPE_STRING = 0,
    CONFIG_TYPE_INT = 1,
    CONFIG_TYPE_BOOL = 2,
    CONFIG_TYPE_FLOAT = 3
} config_type_t;

/* Configuration key-value pair structure with type information */
typedef struct {
    char *key;
    char *value;
    config_type_t type;
    char *description;
    char *default_value;
    int is_readonly;
} config_pair_t;

/* Configuration storage structure */
typedef struct {
    config_pair_t *pairs;
    int count;
    int capacity;
} config_t;

/* Global configuration instances */
extern config_t xshell_config;
extern config_t xcodex_config;

/* Configuration function declarations */
int config_init(config_t *config);
void config_free(config_t *config);
int config_load_file(config_t *config, const char *filename);
int config_save_file(config_t *config, const char *filename);
int config_set(config_t *config, const char *key, const char *value);
int config_set_with_type(config_t *config, const char *key, const char *value, config_type_t type, const char *description);
const char *config_get(config_t *config, const char *key);
const char *config_get_default(config_t *config, const char *key, const char *default_value);
int config_get_int(config_t *config, const char *key, int default_value);
int config_get_bool(config_t *config, const char *key, int default_value);
float config_get_float(config_t *config, const char *key, float default_value);
int config_remove(config_t *config, const char *key);
void config_print_all(config_t *config);
void config_print_with_types(config_t *config);
int config_validate_value(const char *value, config_type_t type);
int config_load_defaults(config_t *config, const char *type);
int config_load_all_files(void);
int config_load_with_fallback(config_t *config, const char *config_file);
int config_create_default_files(void);
int config_export_ini(config_t *config, const char *filename);
int config_export_env(config_t *config, const char *filename);
int config_import_from_file(config_t *config, const char *filename);
void config_list_available_keys(const char *type);
int config_has_key(config_t *config, const char *key);
int config_get_type(config_t *config, const char *key);
const char *config_get_description(config_t *config, const char *key);
int config_set_readonly(config_t *config, const char *key, int readonly);
int config_is_readonly(config_t *config, const char *key);
void config_backup(config_t *config, const char *backup_file);
int config_restore(config_t *config, const char *backup_file);
void config_show_help(const char *type);

/* Default configuration paths */
#define XSHELL_CONFIG_FILE ".xshellrc"
#define XCODEX_CONFIG_FILE ".xcodexrc"
#define XSHELL_VERSION "1.0.0"

#endif /* CONFIG_H */
