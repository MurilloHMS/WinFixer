#include <stdio.h>
#include <conio.h>
#include "ui.h"

int main(void) {
    const char *items[] = {
        "Gerenciador de Aplicativos",
        "Sair"
    };

    while (1) {
        int choice = ui_run_menu("Menu Principal", items, 2, NULL);
        if (choice == -1 || choice == 1) break;

        ui_clear();
        ui_print_header("Modulo selecionado");
        printf("  Voce escolheu: %s\n", items[choice]); // ? \n dentro de string, não \\n
        ui_print_footer("[ENTER] Voltar");

        _getch();
    }
    return 0;
}
