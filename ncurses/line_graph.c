#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_X 80
#define MAX_Y 24
#define TOTAL_DATA_POINTS 120
#define DISPLAYED_POINTS 20
#define UPDATE_DELAY 500000 // 0.1 seconds in microseconds
#define NUM_LINES 3
#define LEGEND_WIDTH 22
#define LEGEND_HEIGHT 5
#define CHART_WIDTH (MAX_X - LEGEND_WIDTH - 3)

#define COLOR_AXIS 1
#define COLOR_LINE1 2
#define COLOR_LINE2 3
#define COLOR_LINE3 4

typedef struct {
  int data[TOTAL_DATA_POINTS];
  int color;
  char name[20];
  char line_char;
  chtype point_char;
} DataLine;

void initialize_colors() {
  start_color();
  init_pair(COLOR_AXIS, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_LINE1, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_LINE2, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_LINE3, COLOR_YELLOW, COLOR_BLACK);
}

void draw_axes(WINDOW *win, int height, int width) {
  wattron(win, COLOR_PAIR(COLOR_AXIS));
  mvwhline(win, height - 2, 1, ACS_HLINE, width - 2);
  mvwaddch(win, height - 2, width - 1, ACS_RARROW);
  mvwvline(win, 0, 0, ACS_VLINE, height - 2);
  mvwaddch(win, 0, 0, ACS_UARROW);
  wattroff(win, COLOR_PAIR(COLOR_AXIS));
}

void draw_point(WINDOW *win, int x, int y, int color, chtype ch) {
  wattron(win, COLOR_PAIR(color));
  mvwaddch(win, MAX_Y - y - 2, x, ch);
  wattroff(win, COLOR_PAIR(color));
}

void draw_line(WINDOW *win, int x1, int y1, int x2, int y2, int color,
               char ch) {
  wattron(win, COLOR_PAIR(color));
  int dx = x2 - x1;
  int dy = y2 - y1;
  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

  float x_inc = dx / (float)steps;
  float y_inc = dy / (float)steps;

  float x = x1;
  float y = y1;

  for (int i = 0; i <= steps; i++) {
    mvwaddch(win, MAX_Y - (int)(y + 0.5) - 2, (int)(x + 0.5), ch);
    x += x_inc;
    y += y_inc;
  }
  wattroff(win, COLOR_PAIR(color));
}

void draw_legend(WINDOW *win, DataLine lines[], int num_lines) {
  int start_y = 1;
  int start_x = CHART_WIDTH + 2;

  wattron(win, COLOR_PAIR(COLOR_AXIS));
  mvwvline(win, start_y, start_x, ACS_VLINE, LEGEND_HEIGHT);
  mvwvline(win, start_y, start_x + LEGEND_WIDTH, ACS_VLINE, LEGEND_HEIGHT);
  mvwhline(win, start_y, start_x, ACS_HLINE, LEGEND_WIDTH);
  mvwhline(win, start_y + LEGEND_HEIGHT, start_x, ACS_HLINE, LEGEND_WIDTH);
  mvwaddch(win, start_y, start_x, ACS_ULCORNER);
  mvwaddch(win, start_y, start_x + LEGEND_WIDTH, ACS_URCORNER);
  mvwaddch(win, start_y + LEGEND_HEIGHT, start_x, ACS_LLCORNER);
  mvwaddch(win, start_y + LEGEND_HEIGHT, start_x + LEGEND_WIDTH, ACS_LRCORNER);
  wattroff(win, COLOR_PAIR(COLOR_AXIS));

  mvwprintw(win, start_y + 1, start_x + 2, "Legend");

  for (int i = 0; i < num_lines; i++) {
    wattron(win, COLOR_PAIR(lines[i].color));
    mvwprintw(win, start_y + i + 2, start_x + 2, "%s: ", lines[i].name);
    waddch(win, lines[i].point_char);
    waddch(win, lines[i].line_char);
    waddch(win, lines[i].line_char);
    wattroff(win, COLOR_PAIR(lines[i].color));
  }
}

int generate_data_point() { return rand() % (MAX_Y - 4) + 1; }

void shift_data(DataLine *line) {
  for (int i = 0; i < TOTAL_DATA_POINTS - 1; i++) {
    line->data[i] = line->data[i + 1];
  }
}

int main() {
  DataLine lines[NUM_LINES] = {
      {{0}, COLOR_LINE1, "Temperature", '*', ACS_DIAMOND},
      {{0}, COLOR_LINE2, "Humidity", '+', ACS_PLUS},
      {{0}, COLOR_LINE3, "Pressure", 'o', ACS_RTEE}};

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);

  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  initialize_colors();

  srand(time(NULL));

  for (int i = 0; i < NUM_LINES; i++) {
    for (int j = 0; j < TOTAL_DATA_POINTS; j++) {
      lines[i].data[j] = generate_data_point();
    }
  }

  WINDOW *win = newwin(MAX_Y, MAX_X, 0, 0);

  while (1) {
    werase(win);
    box(win, 0, 0);

    draw_axes(win, MAX_Y, CHART_WIDTH);
    draw_legend(win, lines, NUM_LINES);

    for (int line = 0; line < NUM_LINES; line++) {
      for (int i = 0; i < DISPLAYED_POINTS - 1; i++) {
        int index = TOTAL_DATA_POINTS - DISPLAYED_POINTS + i;
        int x1 = i * (CHART_WIDTH - 2) / (DISPLAYED_POINTS - 1) + 1;
        int y1 = lines[line].data[index];
        int x2 = (i + 1) * (CHART_WIDTH - 2) / (DISPLAYED_POINTS - 1) + 1;
        int y2 = lines[line].data[index + 1];

        draw_line(win, x1, y1, x2, y2, lines[line].color,
                  lines[line].line_char);
        draw_point(win, x1, y1, lines[line].color, lines[line].point_char);
      }
      // Draw the last point
      int last_x =
          (DISPLAYED_POINTS - 1) * (CHART_WIDTH - 2) / (DISPLAYED_POINTS - 1) +
          1;
      int last_y = lines[line].data[TOTAL_DATA_POINTS - 1];
      draw_point(win, last_x, last_y, lines[line].color,
                 lines[line].point_char);
    }

    wrefresh(win);

    for (int i = 0; i < NUM_LINES; i++) {
      shift_data(&lines[i]);
      lines[i].data[TOTAL_DATA_POINTS - 1] = generate_data_point();
    }

    if (getch() == 'q') {
      break;
    }

    usleep(UPDATE_DELAY);
  }

  endwin();
  return 0;
}
