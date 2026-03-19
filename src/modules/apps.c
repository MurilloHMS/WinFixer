#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "modules/apps.h"
#include "ui.h"

// Windows guarda informações de todos os apps instalados no Registro "um banco de dados interno  do sistema"
static const struct {
    HKEY root;
    const char *path;
} REG_KEYS[] = {
    { HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" }, // apps 64-bit instalados para todos os usuários
    { HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"}, // apps 32-bit rodando em sistema 64-bit
    { HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" } // apps instalados só para o usuário
};

#define NUM_REG_KEYS 3

// Funcao para ler string do registro
static int reg_read_str(HKEY hKey, const char *value, char *out, DWORD out_size) {
    DWORD type = REG_SZ;
    DWORD size = out_size;
    return RegQueryValueExA(hKey, value, NULL, &type, (LPBYTE) out, &size) == ERROR_SUCCESS;
}

// Funcao para carregar os apps instalados
static void load_installed_apps(AppList *list) {
    list->count = 0;

    // k = Key
    for (int k = 0; k < NUM_REG_KEYS; k++) {
        HKEY hUninstall;
        if (RegOpenKeyExA(REG_KEYS[k].root, REG_KEYS[k].path, 0, KEY_READ, &hUninstall) != ERROR_SUCCESS)
            continue;

        char subKey_name[MAX_NAME_LEN];
        DWORD subKey_len;
        DWORD index = 0;

        while (list->count < MAX_APPS) {
            subKey_len = sizeof(subKey_name);
            LONG result = RegEnumKeyExA(hUninstall, index++, subKey_name, &subKey_len, NULL, NULL, NULL, NULL);

            if (result == ERROR_NO_MORE_ITEMS) break;

            if (result != ERROR_SUCCESS) continue;

            HKEY hApp;

            if (RegOpenKeyExA(hUninstall, subKey_name, 0, KEY_READ, &hApp) !=   ERROR_SUCCESS) continue;

            char display_name[MAX_NAME_LEN] = {0};
            char uninstall_cmd[MAX_CMD_LEN] = {0};
            char version[MAX_VERSION_LEN] = {0};
            char system_comp[8] = {0};


            reg_read_str(hApp, "DisplayName", display_name, sizeof(display_name));
            reg_read_str(hApp, "UninstallString", uninstall_cmd, sizeof(uninstall_cmd));
            reg_read_str(hApp, "DisplayVersion", version, sizeof(version));
            reg_read_str(hApp, "SystemComponent", system_comp, sizeof(system_comp));

            RegCloseKey(hApp);

            if (strlen(display_name) == 0) continue;
            if (strlen(uninstall_cmd) == 0) continue;
            if (system_comp[0] == '1') continue;

            int dup = 0;
            for (int i = 0; i < list->count; i++) {
                if (_stricmp(list->entries[i].name, display_name) == 0) {
                    dup = 1;
                    break;
                }
            }
            if (dup) continue;

            AppEntry *entry = &list->entries[list->count++];
            strncpy_s(entry->name, MAX_NAME_LEN, display_name, _TRUNCATE);
            strncpy_s(entry->version, MAX_VERSION_LEN, version, _TRUNCATE);
            strncpy_s(entry->uninstall_cmd, MAX_CMD_LEN, uninstall_cmd, _TRUNCATE);
            entry->is_msi = (strstr(uninstall_cmd, "msiexec") != NULL) || strstr(uninstall_cmd, "MsiExec") != NULL;
        }
        RegCloseKey(hUninstall);
    }
}

static void sort_apps(AppList *list) {
    for (int i = 0; i < list->count; i++) {
        AppEntry key = list->entries[i];
        int j = i - 1;
        while ( j >= 0 && _stricmp(list->entries[j].name, key.name) > 0) {
            list->entries[j + 1] = list->entries[j];
            j--;
        }
        list->entries[j + 1] = key;
    }
}

static int uninstall_app(const AppEntry *app) {
    char cmd[MAX_CMD_LEN + 64] = {0};

    if (app->is_msi) {
        const char *guid_start = strchr(app->uninstall_cmd, '{');
        const char *guid_end = strchr(app->uninstall_cmd, '}');
        if (guid_start && guid_end) {
            char guid[64] = {0};
            size_t len = (size_t)(guid_end - guid_start) + 1;
            strncpy_s(guid, sizeof(guid), guid_start, len);
            snprintf(cmd, sizeof(cmd), "msiexec.exe /x %s /qb /norestart", guid);
        }
    }

    if (strlen(cmd) == 0)
        strncpy_s(cmd, sizeof(cmd), app->uninstall_cmd, _TRUNCATE);

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    BOOL ok = CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ok) return 0;

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (exit_code == 0 || exit_code == 1605 || exit_code == 1614); // 1605 e 1614 = já removido
}

void menu_apps(void) {
    ui_clear();
    ui_print_header("Gerenciador de Aplicativos");
    ui_set_color(14, 0);
    printf("  Lendo registro do Windows, aguarde...\n");
    ui_reset_color();

    AppList list;
    load_installed_apps(&list);
    sort_apps(&list);

    if (list.count == 0) {
        ui_clear();
        ui_print_header("Gerenciador de Aplicativos");
        printf("  Nenhum aplicativo encontrado.\n");
        ui_print_footer("[ENTER] Voltar");
        _getch();
        return;
    }

    const char **labels = (const char **) malloc(list.count * sizeof(char *));
    char (*bufs)[320] = malloc(list.count * sizeof(*bufs));
    int *mask = (int *) calloc(list.count, sizeof(int));

    if (!labels || !bufs || !mask) {
        printf("  Erro de alocação de memória.\n");
        free(labels);
        free(bufs);
        free(mask);
        return;
    }

    for (int i = 0; i < list.count; i++) {
        if (strlen(list.entries[i].version) > 0)
            snprintf(bufs[i], 320, "%-44.44s  v%s", list.entries[i].name, list.entries[i].version);
        else
            snprintf(bufs[i], 320, "%s", list.entries[i].name);

        labels[i] = bufs[i];
    }

    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "Aplicativos Instalados (%d)", list.count);

    int confirmed = ui_run_checklist(title_buf, labels, list.count, mask, NULL);
    if (!confirmed) goto cleanup;

    int marked = 0;
    for (int i = 0; i < list.count; i++) if (mask[i]) marked++;

    if (marked == 0) {
        ui_clear();
        ui_print_header("Nenhum aplicativo selecionado.");
        printf("  Marque pelo menos um aplicativo com ESPACO.\n");
        ui_print_footer("[ENTER] Voltar");
        _getch();
        goto cleanup;
    }

    ui_clear();
    ui_print_header("Confirmar desinstalacao");
    ui_set_color(12,0);
    printf("  Os seguintes aplicativos serao desisnstalados:\n\n");
    ui_reset_color();
    for (int i = 0; i < list.count; i++)
        if (mask[i]) printf("    -%s\n", list.entries[i].name);

    ui_set_color(14, 0);
    printf("\n  Confirmar? [S] sim   [N] nao\n");
    ui_reset_color();

    int c = _getch();
    if (c != 's' && c != 'S') goto cleanup;

    for (int i = 0; i < list.count; i++) {
        if (!mask[i]) continue;

        ui_clear();
        ui_print_header("Desinstalando...");
        ui_set_color(14,0);
        printf("  Removendo: %s\n\n", list.entries[i].name);
        ui_reset_color();

        int ok = uninstall_app(&list.entries[i]);

        if (ok) {
            ui_set_color(10,0);
            printf("  [OK] Removido com sucesso.\n");
        }else {
            ui_set_color(12,0);
            printf("  [FAIL] Falha ao desinstalar.\n");
        }

        ui_reset_color();
        Sleep(1500);

    }

    ui_print_footer("[ENTER] Voltar ao menu");
    _getch();

cleanup:
    free(labels);
    free(bufs);
    free(mask);
}