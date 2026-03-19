#ifndef WINFIXER_UI_H
#define WINFIXER_UI_H

#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_ENTER 13
#define KEY_ESC   27
#define KEY_SPACE 32

void ui_set_color(int fg, int bg);
void ui_reset_color(void);
void ui_clear(void);
void ui_hide_cursor(void);
void ui_show_cursor(void);
void ui_print_header(const char *title);
void ui_print_footer(const char *hint);
int  ui_run_menu(const char *title, const char **items, int count, const char *footer);
int  ui_run_checklist(const char *title, const char **items, int count, int *selected_mask, const char *footer);

#endif
