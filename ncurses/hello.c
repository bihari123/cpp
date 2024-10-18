#include <ncurses.h>
#include <unistd.h>

int main() {
  // Initialize ncurses
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  // Get screen dimensions
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Calculate center position
  int start_y = max_y / 2;
  int start_x = (max_x - 13) / 2; // 13 is the length of "Hello, World!"

  // Clear the screen
  clear();

  // Display "Hello, World!" at the center of the screen
  mvprintw(start_y, start_x, "Hello, World!");
  refresh();

  // Wait for 3 seconds
  sleep(3);

  // Clean up and exit
  endwin();
  return 0;
}
