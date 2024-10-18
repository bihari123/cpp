#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 100
#define MAX_LINE_LENGTH 80

void draw_scrollable_block(WINDOW *win, char **content, int num_lines,
                           int start_line, int height, int width) {
  int y;
  for (y = 0; y < height && y + start_line < num_lines; y++) {
    mvwprintw(win, y, 0, "%.*s", width, content[y + start_line]);
    wclrtoeol(win);
  }
  for (; y < height; y++) {
    wmove(win, y, 0);
    wclrtoeol(win);
  }
  box(win, 0, 0);
  wrefresh(win);
}

int main() {
  WINDOW *win;
  char *content[MAX_LINES];
  int num_lines = 0, start_line = 0, ch;
  int max_y, max_x, height, width;

  // Initialize content
  for (int i = 0; i < MAX_LINES; i++) {
    content[i] = malloc(MAX_LINE_LENGTH * sizeof(char));
    snprintf(content[i], MAX_LINE_LENGTH,
             "Line %d: This is some sample text for the scrollable block.",
             i + 1);
    num_lines++;
  }

  // Initialize ncurses
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  // Get screen dimensions
  getmaxyx(stdscr, max_y, max_x);
  height = max_y - 4;
  width = max_x - 4;

  // Create window
  win = newwin(height, width, 2, 2);

  // Main loop
  while ((ch = getch()) != 'q') {
    switch (ch) {
    case KEY_UP:
      if (start_line > 0)
        start_line--;
      break;
    case KEY_DOWN:
      if (start_line < num_lines - height)
        start_line++;
      break;
    case KEY_PPAGE:
      start_line -= height;
      if (start_line < 0)
        start_line = 0;
      break;
    case KEY_NPAGE:
      start_line += height;
      if (start_line > num_lines - height)
        start_line = num_lines - height;
      if (start_line < 0)
        start_line = 0;
      break;
    }
    draw_scrollable_block(win, content, num_lines, start_line, height, width);
  }

  // Clean up
  endwin();
  for (int i = 0; i < num_lines; i++) {
    free(content[i]);
  }

  return 0;
}
