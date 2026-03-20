#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "modules/disk.h"
#include "ui.h"


static void get_drive_type_str(UINT type, char *out, int out_size) {
    switch (type) {
        case DRIVE_FIXED:
            strncpy_s(out, out_size, "Disco Local", _TRUNCATE);
            break;
        case DRIVE_REMOVABLE:
            strncpy_s(out, out_size, "Removível", _TRUNCATE);
            break;
        case DRIVE_REMOTE:
            strncpy_s(out, out_size, "Rede", _TRUNCATE);
            break;
        case DRIVE_CDROM:
            strncpy_s(out, out_size, "CD/DVD", _TRUNCATE);
            break;
        case DRIVE_RAMDISK:
            strncpy_s(out, out_size, "Ram Disk", _TRUNCATE);
            break;
        default:
            strncpy_s(out, out_size, "Desconhecido", _TRUNCATE);
            break;
    }
}

static void load_volumes(VolumeList *vlist) {
    vlist->count = 0;
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < 26; i++) {
        if (!((drives >> i) & 1)) continue;

        char path[4];

        path[0] = 'A' + i;
        path[1] = ':';
        path[2] = '\\';
        path[3] = '\0';

        VolumeInfo *v = &vlist->volumes[vlist->count++];
        v->letter = 'A' + i;
        v->accessible = 0;

        UINT drive_type = GetDriveTypeA(path);
        get_drive_type_str(drive_type, v->drive_type, sizeof(v->drive_type));

        GetVolumeInformationA(path, v->label, sizeof(v->label),NULL,NULL,NULL, v->fs_type, sizeof(v->fs_type));

        ULONGLONG free_bytes, total_bytes, total_free;
        if (GetDiskFreeSpaceExA(path, (PULARGE_INTEGER)&free_bytes,(PULARGE_INTEGER)&total_bytes,(PULARGE_INTEGER)&total_free)) {
            v->free = free_bytes;
            v->total = total_bytes;
            v->accessible = 1;
        }
    }
}


// Formatacao dos bytes em texto legivel
static void format_bytes(ULONGLONG bytes, char *out, int out_size) {
    if      (bytes >= 1099511627776ULL) // 1 TB
        snprintf(out, out_size, "%.1f TB", (double)bytes / 1099511627776.0);
    else if (bytes >= 1073741824ULL)    // 1 GB
        snprintf(out, out_size, "%.1f GB", (double)bytes / 1073741824.0);
    else if (bytes >= 1048576ULL)       // 1 MB
        snprintf(out, out_size, "%.1f MB", (double)bytes / 1048576.0);
    else
        snprintf(out, out_size, "%llu KB", bytes / 1024ULL);
}

// Barra visual para mostrar uso do disco
static void print_usage_bar(ULONGLONG used, ULONGLONG total) {
    if (total == 0) return;

    int bar_width = 30;
    int filled = (int) ((double) used / (double) total * bar_width);

    double pct = (double) used / (double) total * 100.0;
    if (pct < 70.0) {
        ui_set_color(10,0);
    }else if (pct < 90.0) {
        ui_set_color(14,0);
    }else {
        ui_set_color(12,0);
    }

    printf("  [");
    for (int i = 0; i < bar_width; i++)
        printf("%c", i < filled ? '#' : '-');
    printf("] %.1f%%\n", pct);

    ui_reset_color();
}

// Informacao dos volumes
static void show_disk_info(void) {
    ui_clear();
    ui_print_header("Informacoes dos Volumes");

    VolumeList vlist;
    load_volumes(&vlist);

    if (vlist.count == 0) {
        printf("  Nenhum volume encontrado.\n");
        ui_print_footer("[ENTER] Voltar");
        _getch();
        return;
    }

    for (int i = 0; i < vlist.count; i++) {
        VolumeInfo *v = &vlist.volumes[i];

        ui_set_color(11,0);
        printf("  %c:  ", v->letter);

        ui_set_color(7,0);

        if (strlen(v->label) > 0) {
            printf("%-20s", v->label);
        }else {
            printf("%-20s", "(sem nome)");
        }

        printf("  %-12s  %s\n", v->fs_type, v->drive_type);

        if (v->accessible) {
            char free_str[16], total_str[16];
            format_bytes(v->free, free_str, sizeof(free_str));
            format_bytes(v->total, total_str, sizeof(total_str));

            printf("      Livre: %-10s    Total: %s\n", free_str, total_str);
            print_usage_bar(v->free, v->total);
        }else {
            ui_set_color(8,0);
            printf("      (sem acesso)\n");
            ui_reset_color();
        }
        printf("\n");
    }

    ui_print_footer("[ENTER] Voltar");
    _getch();
}

static void run_repair_command(const char *title, const char *program, const char *args) {
    ui_clear();
    ui_print_header(title);
    ui_set_color(14,0);
    printf("  Iniciando: %s %s\n", program, args);
    printf("  O programa irá abrir outro terminal para executar\n");
    printf("  Aguarde o comando concluir para continuar");
    ui_reset_color();

    SHELLEXECUTEINFOA sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "runas";
    sei.lpFile = program;
    sei.lpParameters = args;
    sei.nShow = SW_SHOW;

    if (!ShellExecuteExA(&sei)) {
        ui_set_color(12,0);
        printf("\n  [ERRO] nao foi possivel iniciar. Tente novamente como Administrador.\n");
        ui_reset_color();
        ui_print_footer("[ENTER] Voltar");
        _getch();
        return;
    }

    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);

    ui_set_color(10,0);
    printf("\n  [OK] Operacao concluida.\n");
    ui_reset_color();
    ui_print_footer("[ENTER] Voltar");
    _getch();
}

static void run_sfc(void) {
    run_repair_command(
        "Verificacao de Arquivos do Sistema (SFC)",
        "cmd.exe",
        "/k sfc /scannow"

    );
}

static void run_chkdsk(void) {
    ui_clear();
    ui_print_header("CHKDSK - Verificacao de Disco");

    VolumeList vlist;
    load_volumes(&vlist);

    const char *items[MAX_VOLUMES];
    char        labels[MAX_VOLUMES][16];
    int         indices[MAX_VOLUMES];
    int         item_count = 0;

    for (int i = 0; i < vlist.count; i++) {
        if (strcmp(vlist.volumes[i].drive_type, "Disco Local") != 0) continue;
        snprintf(labels[item_count], sizeof(labels[0]),
                 "Drive %c:", vlist.volumes[i].letter);
        items[item_count]   = labels[item_count];
        indices[item_count] = i;
        item_count++;
    }

    if (item_count == 0) {
        printf("  Nenhum disco local encontrado.\n");
        ui_print_footer("[ENTER] Voltar");
        _getch();
        return;
    }

    int choice = ui_run_menu("Selecione o drive", items, item_count, NULL);
    if (choice == -1) return;

    char args[32];
    snprintf(args, sizeof(args), "/k chkdsk %c: /f /r",
             vlist.volumes[indices[choice]].letter);

    run_repair_command("CHKDSK", "cmd.exe", args);
}

static void run_cleanmgr(void) {
    run_repair_command(
        "Limpeza de Disco",
        "cleanmgr.exe",
        "/d C:"
    );
}

void menu_disk(void) {
    const char *items[] = {
        "Informacoes dos Volumes",
        "Verificar Arquivos do Sistema (SFC)",
        "Verificar e Reparar Disco (CHKDSK)",
        "Limpeza de Disco",
        "Voltar"
    };

    while (1) {
        int choice = ui_run_menu("Analise de Disco", items, 5, NULL);
        if (choice == -1 || choice == 4) return;

        switch (choice) {
            case 0: show_disk_info(); break;
            case 1: run_sfc();        break;
            case 2: run_chkdsk();     break;
            case 3: run_cleanmgr();   break;
        }
    }
}
