#include <stdio.h>
#include <stdlib.h>

#define MAX_ENTRIES 50

typedef struct {
  int data[MAX_ENTRIES];
  int head;
  int tail;
  int count;
} CircularBuffer;

void initBuffer(CircularBuffer *buffer) {
  buffer->head = 0;
  buffer->tail = 0;
  buffer->count = 0;
}

void addEntry(CircularBuffer *buffer, int value) {
  if (buffer->count == MAX_ENTRIES) {
    buffer->tail = (buffer->tail + 1) % MAX_ENTRIES;
  } else {
    buffer->count++;
  }

  buffer->data[buffer->head] = value;
  buffer->head = (buffer->head + 1) % MAX_ENTRIES;
}

void printBuffer(CircularBuffer *buffer) {
  int i;
  int index;

  printf("Latest 50 entries (most recent at the top):\n");
  for (i = 0; i < buffer->count; i++) {
    index = (buffer->head - 1 - i + MAX_ENTRIES) % MAX_ENTRIES;
    printf("%d\n", buffer->data[index]);
  }
  printf("\n");
}

int main() {
  CircularBuffer buffer;
  initBuffer(&buffer);

  // Example: Add 60 entries
  for (int i = 1; i <= 60; i++) {
    addEntry(&buffer, i);
  }

  printBuffer(&buffer);

  return 0;
}
