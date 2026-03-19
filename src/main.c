#include <stdio.h>
#include <conio.h>
#include "ui.h"
#include "modules/apps.h"

int main(void) {
    const char *items[] = {
        "Gerenciador de Aplicativos",
        "Sair"
    };

    while (1) {
        int choice = ui_run_menu("Menu Principal", items, 2, NULL);
        if (choice == -1 || choice == 1) break;

        switch (choice) {
            case 0: menu_apps(); break;
        }
    }

    ui_clear();
    ui_set_color(11,0);
    printf("\n  WinFixer encerrado.\n");
    ui_reset_color();
    _getch();

    return 0;
}
