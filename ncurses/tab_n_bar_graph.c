#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_TABS 3
#define NUM_BARS 5
#define MAX_BAR_HEIGHT 5

void draw_menu(WINDOW *menu_win, int highlight);
void draw_bar_graph(WINDOW *menu_win);
void draw_tab_content(WINDOW *menu_win, int tab);

int bar_values[NUM_BARS];

int main() {
  WINDOW *menu_win;
  int highlight = 0;
  int choice = 0;
  int c;

  // Initialize random number generator
  srand(time(NULL));

  // Generate random values for bar graph
  for (int i = 0; i < NUM_BARS; i++) {
    bar_values[i] = rand() % (MAX_BAR_HEIGHT + 1);
  }

  // Initialize ncurses
  initscr();
  clear();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);

  // Create menu window
  menu_win = newwin(20, 50, 1, 1);
  keypad(menu_win, TRUE);

  // Main loop
  while (1) {
    draw_menu(menu_win, highlight);
    draw_tab_content(menu_win, highlight);
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
      mvprintw(22, 2, "You chose tab %d    ", choice + 1);
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
  char *tabs[NUM_TABS] = {"Bar Graph", "Tab 2", "Tab 3"};

  x = 2;
  y = 1;
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

void draw_bar_graph(WINDOW *menu_win) {
  int start_y = 5;
  int start_x = 5;

  mvwprintw(menu_win, start_y - 1, start_x, "Bar Graph:");

  for (int i = 0; i < NUM_BARS; i++) {
    for (int j = 0; j < MAX_BAR_HEIGHT; j++) {
      if (j < bar_values[i]) {
        mvwprintw(menu_win, start_y + MAX_BAR_HEIGHT - j - 1, start_x + i * 4,
                  "###");
      } else {
        mvwprintw(menu_win, start_y + MAX_BAR_HEIGHT - j - 1, start_x + i * 4,
                  "   ");
      }
    }
    mvwprintw(menu_win, start_y + MAX_BAR_HEIGHT, start_x + i * 4, "%3d",
              bar_values[i]);
  }
}

void draw_tab_content(WINDOW *menu_win, int tab) {
  // Clear the content area
  for (int i = 3; i < 19; i++) {
    mvwhline(menu_win, i, 1, ' ', 48);
  }

  // Draw content based on the selected tab
  switch (tab) {
  case 0:
    draw_bar_graph(menu_win);
    break;
  case 1:
    mvwprintw(menu_win, 10, 20, "Tab 2 Content");
    break;
  case 2:
    mvwprintw(menu_win, 10, 20, "Tab 3 Content");
    break;
  }

  wrefresh(menu_win);
}
