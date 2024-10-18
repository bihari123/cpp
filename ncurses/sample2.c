#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_BLOCKS 3
#define NUM_TABS 2
#define TABLE_ROWS 5
#define TABLE_COLS 3
#define BAR_HEIGHT 5
#define NUM_COLORS 5

typedef struct {
  WINDOW *win;
  int startx, starty;
  int width, height;
} Block;

typedef struct {
  WINDOW *win;
  char *title;
  Block blocks[NUM_BLOCKS];
  WINDOW *table_win;
} Tab;

WINDOW *content_win;

void init_ncurses() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  init_pair(2, COLOR_BLACK, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_GREEN);
  init_pair(4, COLOR_WHITE, COLOR_RED);
  init_pair(5, COLOR_BLACK, COLOR_YELLOW);
  // New color pairs for the bar graph
  init_pair(6, COLOR_RED, COLOR_RED);
  init_pair(7, COLOR_GREEN, COLOR_GREEN);
  init_pair(8, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(9, COLOR_BLUE, COLOR_BLUE);
  init_pair(10, COLOR_MAGENTA, COLOR_MAGENTA);
  clear();
  refresh();

  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  content_win = newwin(max_y - 3, max_x, 3, 0);
  wbkgd(content_win, COLOR_PAIR(1));
}

void draw_bar_graph(WINDOW *win) {
  int max_x, max_y;
  getmaxyx(win, max_y, max_x);

  const char *labels[] = {"A", "B", "C", "D", "E"};
  int values[5];
  int max_value = 0;

  // Generate random values and find the maximum
  for (int i = 0; i < 5; i++) {
    values[i] = rand() % (max_x - 15) + 1;
    if (values[i] > max_value)
      max_value = values[i];
  }

  // Draw bars
  for (int i = 0; i < 5; i++) {
    int bar_length = (values[i] * (max_x - 15)) / max_value;
    int color_pair = 6 + i; // Use color pairs 6-10 for the bars
    wattron(win, COLOR_PAIR(color_pair));
    mvwhline(win, max_y - 3 - i * 2, 2, ' ', bar_length);
    wattroff(win, COLOR_PAIR(color_pair));
    mvwprintw(win, max_y - 3 - i * 2, bar_length + 3, "%d", values[i]);
    mvwprintw(win, max_y - 3 - i * 2, 1, "%s", labels[i]);
  }

  // Draw legend
  mvwprintw(win, max_y - 1, 2, "Legend:");
  for (int i = 0; i < 5; i++) {
    wattron(win, COLOR_PAIR(6 + i));
    mvwprintw(win, max_y - 1, 11 + i * 8, "  ");
    wattroff(win, COLOR_PAIR(6 + i));
    mvwprintw(win, max_y - 1, 13 + i * 8, "%s", labels[i]);
  }
}

void draw_paragraph(WINDOW *win) {
  const char *text =
      "This is a sample paragraph to demonstrate text wrapping in ncurses. "
      "It will automatically wrap to the next line when it reaches the edge of "
      "the window. "
      "This allows for easy display of longer text content within a confined "
      "space.";

  int max_x, max_y, start_y = 2, start_x = 2;
  getmaxyx(win, max_y, max_x);

  for (const char *c = text; *c != '\0'; c++) {
    mvwaddch(win, start_y, start_x, *c);
    if (++start_x >= max_x - 2) {
      start_x = 2;
      if (++start_y >= max_y - 1)
        break;
    }
  }
}

void draw_list(WINDOW *win) {
  const char *items[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
  int max_y, max_x;
  getmaxyx(win, max_y, max_x);

  for (int i = 0; i < 5 && i < max_y - 2; i++) {
    mvwprintw(win, i + 2, 2, "%d. %s", i + 1, items[i]);
  }
}

void create_blocks(Block blocks[], int num_blocks, int tab_index) {
  int max_y, max_x;
  getmaxyx(content_win, max_y, max_x);

  int block_width = max_x;
  int block_height = max_y / num_blocks;

  for (int i = 0; i < num_blocks; i++) {
    blocks[i].width = block_width;
    blocks[i].height = block_height;
    blocks[i].startx = 0;
    blocks[i].starty = i * block_height;
    blocks[i].win = derwin(content_win, block_height, block_width,
                           blocks[i].starty, blocks[i].startx);
    if (blocks[i].win == NULL) {
      mvprintw(max_y - 1, 0, "Failed to create window for block %d", i + 1);
      refresh();
      return;
    }
    box(blocks[i].win, 0, 0);
    wbkgd(blocks[i].win, COLOR_PAIR(1));
  }
}

void create_table(Tab *tab) {
  int max_y, max_x;
  getmaxyx(content_win, max_y, max_x);

  tab->table_win = derwin(content_win, max_y, max_x, 0, 0);
  if (tab->table_win == NULL) {
    mvprintw(max_y - 1, 0, "Failed to create table window");
    refresh();
    return;
  }

  box(tab->table_win, 0, 0);
  wbkgd(tab->table_win, COLOR_PAIR(1));
}

void draw_table_headers(WINDOW *win) {
  int max_x, max_y;
  getmaxyx(win, max_y, max_x);

  mvwprintw(win, 1, 2, "Column 1");
  mvwprintw(win, 1, 20, "Column 2");
  mvwprintw(win, 1, 38, "Column 3");
  mvwchgat(win, 1, 1, max_x - 2, A_BOLD, 5, NULL);
}

void update_table(Tab *tab) {
  werase(tab->table_win);
  box(tab->table_win, 0, 0);
  draw_table_headers(tab->table_win);

  for (int i = 0; i < TABLE_ROWS; i++) {
    mvwprintw(tab->table_win, i + 3, 2, "%8d", rand() % 100);
    mvwprintw(tab->table_win, i + 3, 20, "%8d", rand() % 1000);
    mvwprintw(tab->table_win, i + 3, 38, "%8d", rand() % 10000);
  }
  wnoutrefresh(tab->table_win);
}

void create_tabs(Tab tabs[], int num_tabs) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int tab_width = max_x / num_tabs;

  for (int i = 0; i < num_tabs; i++) {
    tabs[i].win = newwin(3, tab_width, 0, i * tab_width);
    if (tabs[i].win == NULL) {
      mvprintw(max_y - 1, 0, "Failed to create tab window %d", i + 1);
      refresh();
      return;
    }
    tabs[i].title = malloc(20 * sizeof(char));
    snprintf(tabs[i].title, 20, "Tab %d", i + 1);

    if (i == 0) {
      create_blocks(tabs[i].blocks, NUM_BLOCKS, i);
    } else {
      create_table(&tabs[i]);
    }
  }
}

void draw_tabs(Tab tabs[], int num_tabs, int current_tab) {
  for (int i = 0; i < num_tabs; i++) {
    if (i == current_tab) {
      wbkgd(tabs[i].win, COLOR_PAIR(4));
    } else {
      wbkgd(tabs[i].win, COLOR_PAIR(2));
    }
    werase(tabs[i].win);
    box(tabs[i].win, 0, 0);
    mvwprintw(tabs[i].win, 1, 2, "%s", tabs[i].title);
    wnoutrefresh(tabs[i].win);
  }
}

void show_tab(Tab *tab, int tab_index, int current_block) {
  werase(content_win);
  if (tab_index == 0) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
      if (i == current_block) {
        wbkgd(tab->blocks[i].win, COLOR_PAIR(3));
      } else {
        wbkgd(tab->blocks[i].win, COLOR_PAIR(1));
      }
      werase(tab->blocks[i].win);
      box(tab->blocks[i].win, 0, 0);
      switch (i) {
      case 0:
        mvwprintw(tab->blocks[i].win, 1, 2, "Bar Graph");
        draw_bar_graph(tab->blocks[i].win);
        break;
      case 1:
        mvwprintw(tab->blocks[i].win, 1, 2, "Paragraph");
        draw_paragraph(tab->blocks[i].win);
        break;
      case 2:
        mvwprintw(tab->blocks[i].win, 1, 2, "List");
        draw_list(tab->blocks[i].win);
        break;
      }
      wnoutrefresh(tab->blocks[i].win);
    }
  } else {
    update_table(tab);
  }
  wnoutrefresh(content_win);
}

void redraw_screen(Tab tabs[], int num_tabs, int current_tab,
                   int current_block) {
  clear();
  refresh();
  draw_tabs(tabs, num_tabs, current_tab);
  show_tab(&tabs[current_tab], current_tab, current_block);
  mvprintw(LINES - 1, 0,
           "Use arrow keys to move, TAB/SHIFT+TAB to switch tabs, 'q' to quit");
  refresh();
}

int main() {
  init_ncurses();
  srand(time(NULL));

  Tab tabs[NUM_TABS];
  create_tabs(tabs, NUM_TABS);

  int current_tab = 0;
  int current_block = 0;

  redraw_screen(tabs, NUM_TABS, current_tab, current_block);

  int ch;
  time_t last_update = time(NULL);
  while ((ch = getch()) != 'q') {
    switch (ch) {
    case KEY_UP:
      if (current_tab == 0 && current_block > 0) {
        current_block--;
      }
      break;
    case KEY_DOWN:
      if (current_tab == 0 && current_block < NUM_BLOCKS - 1) {
        current_block++;
      }
      break;
    case 9: // TAB key
      current_tab = (current_tab + 1) % NUM_TABS;
      current_block = 0; // Reset block selection when switching tabs
      break;
    case 353: // SHIFT+TAB key
      current_tab = (current_tab - 1 + NUM_TABS) % NUM_TABS;
      current_block = 0; // Reset block selection when switching tabs
      break;
    }

    redraw_screen(tabs, NUM_TABS, current_tab, current_block);

    if (current_tab == 1 && time(NULL) - last_update >= 1) {
      update_table(&tabs[current_tab]);
      last_update = time(NULL);
    }

    doupdate();
    usleep(10000);
  }

  endwin();
  for (int i = 0; i < NUM_TABS; i++) {
    free(tabs[i].title);
  }
  printf("Program finished. Thank you for using the ncurses blocks demo!\n");
  return 0;
}
