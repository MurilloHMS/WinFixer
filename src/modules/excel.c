#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "modules/excel.h"
#include "ui.h"

#define TEMP_DIR "temp_xlsm"
#define TEMP_ZIP "temp.zip"
#define VBA_PATH "temp_xlsm/xl/vbaProject.bin"

static void run_command(const char *cmd) {
    int result = system(cmd);
    if (result != 0) {
        printf("command error: %s\n", cmd);
        printf("Codigo de retorno: %d\n", result);
    }
}

static void extract_xlsm(const char *input) {
    char command[512];

    printf("Preparando arquivo...\n");

    sprintf(command,
            "powershell -Command \""
            "if (Test-Path '%s') { Remove-Item '%s' }; "
            "Copy-Item '%s' '%s'; "
            "Rename-Item '%s' '%s'\"",
            TEMP_ZIP,
            TEMP_ZIP,
            input,
            TEMP_ZIP,
            TEMP_ZIP,
            TEMP_ZIP);

    run_command(command);

    printf("Obtendo dados...\n");

    sprintf(command,
            "powershell -Command \""
            "if (Test-Path '%s') { Remove-Item '%s' -Recurse -Force }; "
            "Expand-Archive -Force '%s' '%s'\"",
            TEMP_DIR,
            TEMP_DIR,
            TEMP_ZIP,
            TEMP_DIR);

    run_command(command);
}

static void patch_vba(void) {
    FILE *f = fopen(VBA_PATH, "rb+");
    if (!f) {
        printf("Erro ao abrir %s\n", VBA_PATH);
        return;
    }

    printf("Removendo credenciais...\n");

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    unsigned char *buffer = (unsigned char *)malloc(size);
    if (!buffer) {
        printf("Erro de memoria");
        fclose(f);
        return;
    }

    size_t read = fread(buffer, 1, size, f);
    if (read != (size_t)size) {
        printf("Erro lendo arquivo (lido %zu de %ld)\n", read, size);
    }

    int count = 0;

    for (long i = 0; i < size -2; i++) {
        if (buffer[i] == 'D' && buffer[i+1] == 'P' && buffer[i+2] == 'B') {
            buffer[i+2] = 'X';
            count++;
        }
    }

    rewind(f);
    fwrite(buffer, 1, size, f);

    free(buffer);
    fclose(f);

    printf("Credenciais removidas com sucesso!\n");
}

static void compress_back(const char *output) {
    char command[512];

    printf("Recriando excel...\n");

    sprintf(command,
            "powershell -Command \""
            "if (Test-Path '%s.zip') { Remove-Item '%s.zip' }; "
            "Compress-Archive -Force '%s/*' '%s.zip'; "
            "if (Test-Path '%s') { Remove-Item '%s' -Recurse -Force }; "
            "Rename-Item '%s.zip' '%s'\"",
            output,
            output,
            TEMP_DIR,
            output,
            TEMP_DIR,
            TEMP_DIR,
            output,
            output);

    run_command(command);
}

static void cleanup_temp(void) {
    char command[512];
    printf("Limpando arquivos...");

    sprintf(command,
            "powershell -Command \""
            "if (Test-Path '%s') { Remove-Item '%s' -Recurse -Force }; "
            "if (Test-Path '%s') { Remove-Item '%s' }\"",
            TEMP_DIR,
            TEMP_DIR,
            TEMP_ZIP,
            TEMP_ZIP);

    run_command(command);

}

static void excel_patch_file(const char *input, const char *output) {
    extract_xlsm(input);
    patch_vba();
    compress_back(output);
    cleanup_temp();
}

static int list_excel_files(char files[][MAX_PATH], int max_files) {
    WIN32_FIND_DATAA ffd;
    HANDLE hFind;

    int count = 0;

    hFind = FindFirstFileA("*.xlsx", &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                strncpy_s(files[count], MAX_PATH, ffd.cFileName, _TRUNCATE);
                count++;
                if (count >= max_files) break;
            }
        } while (FindNextFileA(hFind, &ffd));
        FindClose(hFind);
    }

    hFind = FindFirstFileA("*.xlsm", &ffd);
    if (hFind != INVALID_HANDLE_VALUE && count < max_files) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                strncpy_s(files[count], MAX_PATH, ffd.cFileName, _TRUNCATE);
                count++;
                if (count >= max_files) break;
            }
        } while (FindNextFileA(hFind, &ffd));
        FindClose(hFind);
    }

    return count;
}

void menu_excel(void) {
    char files[256][MAX_PATH];
    int  file_count = list_excel_files(files, 256);

    if (file_count == 0) {
        ui_clear();
        ui_print_header("Ferramentas Excel");
        printf("  Nenhum arquivo .xlsx ou .xlsm encontrado no diretorio atual.\n");
        printf("  Diretorio atual:\n");
        char cwd[MAX_PATH];
        if (GetCurrentDirectoryA(MAX_PATH, cwd))
            printf("    %s\n", cwd);
        ui_print_footer("[ENTER] Voltar");
        _getch();
        return;
    }

    const char *items[256];
    for (int i = 0; i < file_count; i++)
        items[i] = files[i];

    while (1) {
        int choice = ui_run_menu(
            "Arquivos Excel no Diretorio Atual",
            items,
            file_count,
            "[cima/baixo] Navegar  [ENTER] Selecionar  [ESC] Voltar"
        );

        if (choice == -1) return;

        const char *input = files[choice];

        char output[MAX_PATH];
        snprintf(output, sizeof(output), "mod_%s", input);

        ui_clear();
        ui_print_header("Processando Arquivo Excel");
        printf("  Arquivo de entrada: %s\n", input);
        printf("  Arquivo de saida:   %s\n\n", output);

        excel_patch_file(input, output);

        ui_print_footer("[ENTER] Voltar ao menu de arquivos");
        _getch();
    }
}