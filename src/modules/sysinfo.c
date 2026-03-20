#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "modules/sysinfo.h"
#include "ui.h"
#include <winsock2.h>

static void show_windows_info(void) {
    ui_set_color(11, 0);
    printf("  [Sistema Operacional]\n");
    ui_reset_color();

    HKEY hKey;
    char product_name[128] = {0};
    char build_number[16] = {0};
    char edition[64] = {0};
    DWORD size;
    DWORD type = REG_SZ;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        size = sizeof(product_name);
        RegQueryValueExA(hKey, "ProductName",  NULL, &type, (LPBYTE)product_name, &size);

        size = sizeof(build_number);
        RegQueryValueExA(hKey, "CurrentBuildNumber", NULL, &type, (LPBYTE)build_number, &size);

        size = sizeof(edition);
        RegQueryValueExA(hKey, "EditionID", NULL, &type, (LPBYTE)edition, &size);

        RegCloseKey(hKey);
    }

    printf("  Nome:    %s\n", strlen(product_name) > 0 ? product_name : "Desconhecido");
    printf("  Edicao:  %s\n", strlen(edition) > 0 ? edition : "Desconhecido");
    printf("  Build:   %s\n\n", strlen(build_number) > 0 ? build_number : "Desconhecido");
}

static void show_cpu_info(void) {
    ui_set_color(11, 0);
    printf("  [Processador]\n");
    ui_reset_color();


    HKEY hkey;
    char cpu_name[256] = {0};
    DWORD size = sizeof(cpu_name);
    DWORD type = REG_SZ;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hkey) == ERROR_SUCCESS) {

        RegQueryValueExA(hkey, "ProcessorNameString", NULL, &type, (LPBYTE)cpu_name, &size);
        RegCloseKey(hkey);
    }

    SYSTEM_INFO si;
    GetSystemInfo ( &si );

    printf("  Modelo:  %s\n", strlen(cpu_name) > 0 ? cpu_name : "Desconhecido");
    printf("  Nucleos: %lu\n\n", si.dwNumberOfProcessors); // dwNumberOfProcessors = número de núcleos lógicos (threads) %lu = formato para unsigned long (DWORD)
}

static void show_memory_info(void) {
    ui_set_color(11, 0);
    printf("  [Memoria RAM]\n");
    ui_reset_color();

    MEMORYSTATUSEX mem = {0};
    mem.dwLength = sizeof(mem);

    GlobalMemoryStatusEx(&mem);

    double total_gb = (double)mem.ullTotalPhys / 1073741824.0;
    double free_gb  = (double)mem.ullAvailPhys / 1073741824.0;
    double used_gb  = total_gb - free_gb;
    double pct      = (used_gb / total_gb) * 100.0;

    printf("  Total:   %.1f GB\n", total_gb);
    printf("  Em uso:  %.1f GB (%.0f%%)\n", used_gb, pct);
    printf("  Livre:   %.1f GB\n", free_gb);

    int bar_width = 30;
    int filled    = (int)(pct / 100.0 * bar_width);

    if (pct < 70.0)  {
        ui_set_color(10, 0);
    }else if (pct < 90.0) {
        ui_set_color(14, 0);
    }else {
        ui_set_color(12, 0);
    }

    printf("  [");
    for (int i = 0; i < bar_width; i++)
        printf("%c", i < filled ? '#' : '-');
    printf("] %.0f%%\n\n", pct);

    ui_reset_color();
}

static void show_uptime(void) {
    ui_set_color(11, 0);
    printf("  [Tempo Ligado]\n");
    ui_reset_color();

    ULONGLONG ms      = GetTickCount64();
    ULONGLONG seconds = ms / 1000;
    ULONGLONG minutes = seconds / 60;
    ULONGLONG hours   = minutes / 60;
    ULONGLONG days    = hours / 24;

    printf("  Uptime:  %llud %lluh %llum %llus\n\n",
           days,
           hours   % 24,
           minutes % 60,
           seconds % 60);
}

static void show_network_basic(void) {
    ui_set_color(11, 0);
    printf("  [Rede]\n");
    ui_reset_color();

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    char hostname[256] = {0};
    gethostname(hostname, sizeof(hostname));
    printf("  Hostname: %s\n", hostname);

    struct hostent *host = gethostbyname(hostname);
    if (host && host->h_addr_list[0]) {
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[0], sizeof(addr));
        printf("  IP local: %s\n", inet_ntoa(addr));
    }

    printf("\n");
    WSACleanup();
}

void menu_sysinfo(void) {
    while (1) {
        int choice = ui_run_menu(
            "Informacoes do Sistema",
            (const char *[]){
                "Visao Geral Completa",
                "Sistema Operacional",
                "Processador",
                "Memoria RAM",
                "Tempo Ligado",
                "Rede",
                "Voltar"
            }, 7, NULL
        );

        if (choice == -1 || choice == 6) return;

        ui_clear();

        switch (choice) {
            case 0:
                ui_print_header("Visao Geral do Sistema");
                show_windows_info();
                show_cpu_info();
                show_memory_info();
                show_uptime();
                show_network_basic();
                break;
            case 1: ui_print_header("Sistema Operacional"); show_windows_info();    break;
            case 2: ui_print_header("Processador");         show_cpu_info();        break;
            case 3: ui_print_header("Memoria RAM");         show_memory_info();     break;
            case 4: ui_print_header("Tempo Ligado");        show_uptime();          break;
            case 5: ui_print_header("Rede");                show_network_basic();   break;
        }

        ui_print_footer("[ENTER] Voltar");
        _getch();
    }
}
