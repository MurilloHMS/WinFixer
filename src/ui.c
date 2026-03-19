#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include "ui.h"

static HANDLE hConsole; // Handle - Identificador para um recurso do windows

static void init_console(void) {
    if (!hConsole) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // pede ao windows o HANDLE da saída padrão  do console
    }
}
/*
  Cores
    0=preto  1=azul  2=verde  3=ciano  4=vermelho  7=branco
    8=cinza  9=azul claro  10=verde claro  11=ciano claro  12=vermelho claro  14=amarelo
 */
void ui_set_color(int fg, int bg) {
    init_console();
    SetConsoleTextAttribute(hConsole, (WORD) ((bg << 4) | fg)); // windows representa cor do texto e fundo em um único número de 8 bits (bg << 4) desloca os bits do fundo 4 posições para esquerda e | fg coloca os bits do texto nos 4 bits da direita com OR bit a bit (WORD) cast para tipo que SetConsoleTextAttribute espera
}

void ui_reset_color(void) {
    ui_set_color(7,0);
}

void ui_clear(void) {
    system("cls"); // Sistem executa um comando do terminal
}

void ui_hide_cursor(void) {
    init_console();
    CONSOLE_CURSOR_INFO ci = {1, FALSE};
    SetConsoleCursorInfo(hConsole, &ci);
}

void ui_show_cursor(void) {
    init_console();
    CONSOLE_CURSOR_INFO ci = {1, TRUE}; // CONSOLE_CURSOR_INFO é uma struct do Windows com dois campos: tamanho do cursor (1) e se está visível (TRUE/FALSE)
    SetConsoleCursorInfo(hConsole, &ci);
}

void ui_print_header(const char *title) {
    ui_set_color(11, 0);
    printf("\n  +--------------------------------------------------+\n");
    printf("  |  WinFixer  >>  %-32s|\n", title); // %-32s formata a string com 32 caracteres de largura (-) mantém a borda | sempre na mesma coluna, independente do tamanho do title
    printf("  +--------------------------------------------------+\n\n");
    ui_reset_color();
}

void ui_print_footer(const char *hint) {
    ui_set_color(8,0);
    printf("\n  %s\n", hint);
    ui_reset_color();
}

int ui_run_menu(const char *title, const char **items, int count, const char *footer) {
    int selected = 0;
    int key;

    while (1) {
        ui_clear();
        ui_hide_cursor();
        ui_print_header(title);

        for (int i = 0; i < count; i++) {
            if (i == selected) {
                ui_set_color(0, 11);
                printf("  > %-48s\n", items[i]);
            } else {
                ui_reset_color();
                printf("    %-48s\n", items[i]);
            }
        }

        ui_print_footer(footer ? footer : "[cima/baixo] Navegar    [ENTER] Selecionar    [ESC] Voltar");
        ui_reset_color();

        key = _getch();

        if (key == 0 || key == 224) {
            key = _getch();
            if (key == KEY_UP && selected > 0) selected--;
            if (key == KEY_DOWN && selected < count - 1) selected++;
        } else if (key == KEY_ENTER) {
            ui_show_cursor();
            return selected;
        } else if (key == KEY_ESC) {
            ui_show_cursor();
            return -1;
        }
    }
}
