#include <ncurses.h>
#include <string.h>

#define NUM_TABS 3

void draw_menu(WINDOW *menu_win, int highlight);

int main() {
  WINDOW *menu_win;
  int highlight = 0;
  int choice = 0;
  int c;

  // Initialize ncurses
  initscr();
  clear();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  // Create menu window
  menu_win = newwin(10, 40, 1, 1);
  keypad(menu_win, TRUE);

  // Main loop
  while (1) {
    draw_menu(menu_win, highlight);
    c = wgetch(menu_win);

    switch (c) {
    case KEY_LEFT:
      if (highlight > 0)
        --highlight;
      break;
    case KEY_RIGHT:
      if (highlight < NUM_TABS - 1)
        ++highlight;
      break;
    case 10: // Enter key
      choice = highlight;
      break;
    case 'q':
      goto exit_loop;
    }

    if (choice != 0) { // User made a choice
      mvprintw(12, 2, "You chose tab %d    ", choice + 1);
      choice = 0;
      refresh();
    }
  }

exit_loop:
  endwin();
  return 0;
}

void draw_menu(WINDOW *menu_win, int highlight) {
  int x, y, i;
  char *tabs[NUM_TABS] = {"Tab 1", "Tab 2", "Tab 3"};

  x = 2;
  y = 2;
  box(menu_win, 0, 0);
  for (i = 0; i < NUM_TABS; ++i) {
    if (highlight == i) {
      wattron(menu_win, A_REVERSE);
      mvwprintw(menu_win, y, x, "%s", tabs[i]);
      wattroff(menu_win, A_REVERSE);
    } else {
      mvwprintw(menu_win, y, x, "%s", tabs[i]);
    }
    x += strlen(tabs[i]) + 2;
  }
  wrefresh(menu_win);
}
