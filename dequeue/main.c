#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ENTRIES 60

typedef struct Node {
  int data;
  struct Node *prev;
  struct Node *next;
} Node;

typedef struct {
  Node *front;
  Node *rear;
  int size;
} Dequeue;

void initDequeue(Dequeue *dq) {
  dq->front = NULL;
  dq->rear = NULL;
  dq->size = 0;
}

bool isEmpty(Dequeue *dq) { return dq->size == 0; }

bool isFull(Dequeue *dq) { return dq->size == MAX_ENTRIES; }

Node *createNode(int data) {
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (newNode == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  newNode->data = data;
  newNode->prev = NULL;
  newNode->next = NULL;
  return newNode;
}

void pushRear(Dequeue *dq, int data) {
  if (isFull(dq)) {
    // Remove the oldest entry (from front) if we're at capacity
    Node *temp = dq->front;
    dq->front = dq->front->next;
    if (dq->front == NULL) {
      dq->rear = NULL;
    } else {
      dq->front->prev = NULL;
    }
    free(temp);
    dq->size--;
  }

  Node *newNode = createNode(data);
  if (isEmpty(dq)) {
    dq->front = dq->rear = newNode;
  } else {
    newNode->prev = dq->rear;
    dq->rear->next = newNode;
    dq->rear = newNode;
  }
  dq->size++;
}

int popFront(Dequeue *dq) {
  if (isEmpty(dq)) {
    fprintf(stderr, "Dequeue is empty\n");
    exit(1);
  }
  Node *temp = dq->front;
  int data = temp->data;
  dq->front = dq->front->next;
  if (dq->front == NULL) {
    dq->rear = NULL;
  } else {
    dq->front->prev = NULL;
  }
  free(temp);
  dq->size--;
  return data;
}

int peekRear(Dequeue *dq) {
  if (isEmpty(dq)) {
    fprintf(stderr, "Dequeue is empty\n");
    exit(1);
  }
  return dq->rear->data;
}

int size(Dequeue *dq) { return dq->size; }

void freeDequeue(Dequeue *dq) {
  while (!isEmpty(dq)) {
    popFront(dq);
  }
}

// Function to display all entries in the dequeue, newest first
void displayEntries(Dequeue *dq) {
  if (isEmpty(dq)) {
    printf("The table is empty.\n");
    return;
  }

  Node *current = dq->rear;
  int index = 1;
  printf("Latest %d entries in the table (newest on top):\n", dq->size);
  while (current != NULL) {
    printf("%d: %d\n", index++, current->data);
    current = current->prev;
  }
}

int main() {
  Dequeue dq;
  initDequeue(&dq);

  // Simulate adding entries to the table
  for (int i = 1; i <= 70; i++) {
    pushRear(&dq, i);

    // Display the state of the table every 10 entries
    if (i % 10 == 0 || i == 70) {
      printf("\nAfter adding %d entries:\n", i);
      displayEntries(&dq);
    }
  }

  freeDequeue(&dq);
  return 0;
}
