#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BLOCKS 3

typedef struct {
  WINDOW *win;
  int startx, starty;
  int width, height;
} Block;

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
  clear();   // Clear the screen
  refresh(); // Refresh the screen to ensure it's clear
}

void create_blocks(Block blocks[], int num_blocks) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int block_width = max_x / num_blocks;
  int block_height = max_y - 4; // Leave space for the tab

  for (int i = 0; i < num_blocks; i++) {
    blocks[i].width = block_width;
    blocks[i].height = block_height;
    blocks[i].startx = i * block_width;
    blocks[i].starty = 3; // Start below the tab
    blocks[i].win =
        newwin(block_height, block_width, blocks[i].starty, blocks[i].startx);
    if (blocks[i].win == NULL) {
      mvprintw(max_y - 1, 0, "Failed to create window for block %d", i + 1);
      refresh();
      return;
    }
    box(blocks[i].win, 0, 0);
    wbkgd(blocks[i].win, COLOR_PAIR(1));
    mvwprintw(blocks[i].win, 1, 1, "Block %d", i + 1);
    wnoutrefresh(blocks[i].win); // Stage the window for refresh
  }
}

void create_tab(WINDOW **tab_win) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  *tab_win = newwin(3, max_x, 0, 0);
  if (*tab_win == NULL) {
    mvprintw(max_y - 1, 0, "Failed to create tab window");
    refresh();
    return;
  }
  box(*tab_win, 0, 0);
  wbkgd(*tab_win, COLOR_PAIR(2));
  mvwprintw(*tab_win, 1, 2, "Tab");
  wnoutrefresh(*tab_win); // Stage the window for refresh
}

void highlight_block(Block *block) {
  wbkgd(block->win, COLOR_PAIR(3));
  wnoutrefresh(block->win);
}

void unhighlight_block(Block *block) {
  wbkgd(block->win, COLOR_PAIR(1));
  wnoutrefresh(block->win);
}

int main() {
  init_ncurses();

  Block blocks[NUM_BLOCKS];
  WINDOW *tab_win = NULL;

  create_tab(&tab_win);
  create_blocks(blocks, NUM_BLOCKS);

  int current_block = 0;
  highlight_block(&blocks[current_block]);

  mvprintw(LINES - 1, 0, "Use arrow keys to move, 'q' to quit");
  refresh(); // Refresh the main window

  doupdate(); // Update the screen with all changes

  int ch;
  while ((ch = getch()) != 'q') {
    switch (ch) {
    case KEY_LEFT:
      if (current_block > 0) {
        unhighlight_block(&blocks[current_block]);
        current_block--;
        highlight_block(&blocks[current_block]);
      }
      break;
    case KEY_RIGHT:
      if (current_block < NUM_BLOCKS - 1) {
        unhighlight_block(&blocks[current_block]);
        current_block++;
        highlight_block(&blocks[current_block]);
      }
      break;
    }
    doupdate(); // Update the screen after each action
  }

  endwin();
  printf("Program finished. Thank you for using the ncurses blocks demo!\n");
  return 0;
}
