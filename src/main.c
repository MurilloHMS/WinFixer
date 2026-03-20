#include <stdio.h>
#include <conio.h>
#include "ui.h"
#include "modules/apps.h"
#include "modules/disk.h"
#include "modules/sysinfo.h"

int main(void) {
    const char *items[] = {
        "Gerenciador de Aplicativos",
        "Analise de Disco",
        "Informacoes do Sistema",
        "Sair"
    };

    while (1) {
        int choice = ui_run_menu("Menu Principal", items, 4, NULL);
        if (choice == -1 || choice == 3) break;

        switch (choice) {
            case 0: menu_apps(); break;
            case 1: menu_disk(); break;
            case 2: menu_sysinfo(); break;
        }
    }

    ui_clear();
    ui_set_color(11,0);
    printf("\n  WinFixer encerrado.\n");
    ui_reset_color();
    _getch();

    return 0;
}
