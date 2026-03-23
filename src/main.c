#include <stdio.h>
#include <conio.h>
#include "ui.h"
#include "modules/apps.h"
#include "modules/disk.h"
#include "modules/excel.h"
#include "modules/sysinfo.h"

int main(void) {
    const char *items[] = {
        "Gerenciador de Aplicativos",
        "Analise de Disco",
        "Informacoes do Sistema",
        "Ferramentas Excel",
        "Sair"
    };

    while (1) {
        int choice = ui_run_menu("Menu Principal", items, 5, NULL);
        if (choice == -1 || choice == 4) break;

        switch (choice) {
            case 0: menu_apps(); break;
            case 1: menu_disk(); break;
            case 2: menu_sysinfo(); break;
            case 3: menu_excel(); break;
        }
    }

    ui_clear();
    ui_set_color(11,0);
    printf("\n  WinFixer encerrado.\n");
    ui_reset_color();
    _getch();

    return 0;
}
