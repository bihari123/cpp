#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_BLOCKS 3
#define NUM_TABS 2
#define TABLE_ROWS 5
#define TABLE_COLS 3

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
  clear();
  refresh();

  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  content_win = newwin(max_y - 3, max_x, 3, 0);
  wbkgd(content_win, COLOR_PAIR(1));
}

void create_blocks(Block blocks[], int num_blocks, int tab_index) {
  int max_y, max_x;
  getmaxyx(content_win, max_y, max_x);

  int block_width = max_x / num_blocks;
  int block_height = max_y;

  for (int i = 0; i < num_blocks; i++) {
    blocks[i].width = block_width;
    blocks[i].height = block_height;
    blocks[i].startx = i * block_width;
    blocks[i].starty = 0;
    blocks[i].win = derwin(content_win, block_height, block_width,
                           blocks[i].starty, blocks[i].startx);
    if (blocks[i].win == NULL) {
      mvprintw(max_y - 1, 0, "Failed to create window for block %d", i + 1);
      refresh();
      return;
    }
    box(blocks[i].win, 0, 0);
    wbkgd(blocks[i].win, COLOR_PAIR(1));
    mvwprintw(blocks[i].win, 1, 1, "Tab %d Block %d", tab_index + 1, i + 1);
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

void highlight_block(Block *block) {
  wbkgd(block->win, COLOR_PAIR(3));
  box(block->win, 0, 0);
  wnoutrefresh(block->win);
}

void unhighlight_block(Block *block) {
  wbkgd(block->win, COLOR_PAIR(1));
  box(block->win, 0, 0);
  wnoutrefresh(block->win);
}

void show_tab(Tab *tab, int tab_index, int current_block) {
  werase(content_win);
  if (tab_index == 0) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
      if (i == current_block) {
        highlight_block(&tab->blocks[i]);
      } else {
        unhighlight_block(&tab->blocks[i]);
      }
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
    case KEY_LEFT:
      if (current_tab == 0 && current_block > 0) {
        current_block--;
      }
      break;
    case KEY_RIGHT:
      if (current_tab == 0 && current_block < NUM_BLOCKS - 1) {
        current_block++;
      }
      break;
    case 9: // TAB key
      current_tab = (current_tab + 1) % NUM_TABS;
      break;
    case 353: // SHIFT+TAB key
      current_tab = (current_tab - 1 + NUM_TABS) % NUM_TABS;
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
