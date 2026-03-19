#ifndef WINFIXER_APPS_H
#define WINFIXER_APPS_H

#define MAX_APPS 512
#define MAX_NAME_LEN 256
#define MAX_CMD_LEN 512
#define MAX_VERSION_LEN 64

typedef struct {
    char name[MAX_NAME_LEN];
    char version[MAX_VERSION_LEN];
    char uninstall_cmd[MAX_CMD_LEN];
    int is_msi;
} AppEntry;

typedef struct {
    AppEntry entries[MAX_APPS];
    int count;
} AppList;

void menu_apps(void);

#endif