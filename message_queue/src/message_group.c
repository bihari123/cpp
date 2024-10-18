#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_NAME "/test_queue"
#define MAX_SIZE 1024
#define MSG_STOP "exit"
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 4
#define NUM_GROUPS 3
#define MSGS_PER_PRODUCER 10
#define MAX_GROUP_NAME 20
#define MAX_RETRIES 5
#define RETRY_DELAY_US 100000 // 100 ms

const char *GROUP_NAMES[NUM_GROUPS] = {"analytics", "logging", "notifications"};

typedef struct {
  char group[MAX_GROUP_NAME];
  char text[MAX_SIZE - MAX_GROUP_NAME];
} Message;

int send_with_retry(mqd_t mq, const char *msg_ptr, size_t msg_len,
                    unsigned int msg_prio) {
  int retries = 0;
  while (retries < MAX_RETRIES) {
    if (mq_send(mq, msg_ptr, msg_len, msg_prio) != -1) {
      return 0; // Success
    }
    if (errno != EAGAIN) {
      perror("mq_send");
      return -1; // Unrecoverable error
    }
    usleep(RETRY_DELAY_US);
    retries++;
  }
  fprintf(stderr, "Failed to send message after %d retries\n", MAX_RETRIES);
  return -1;
}

void producer(int id, mqd_t mq) {
  Message msg;
  srand(time(NULL) ^ (id << 16));

  for (int i = 0; i < MSGS_PER_PRODUCER; i++) {
    int group_index = rand() % NUM_GROUPS;
    strncpy(msg.group, GROUP_NAMES[group_index], MAX_GROUP_NAME);
    snprintf(msg.text, sizeof(msg.text),
             "Message from producer %d (Group %s): %d", id, msg.group, i);
    if (send_with_retry(mq, (char *)&msg, sizeof(Message), 0) == -1) {
      fprintf(stderr, "Producer %d failed to send message\n", id);
    }
    usleep(rand() % 1000000); // Sleep up to 1 second
  }
}

void consumer(int id, const char *group, int pipe_fd) {
  Message msg;
  int must_stop = 0;

  printf("Consumer %d started (Group %s)\n", id, group);

  while (!must_stop) {
    ssize_t bytes_read = read(pipe_fd, &msg, sizeof(Message));

    if (bytes_read > 0) {
      if (strcmp(msg.group, group) == 0 || strcmp(msg.group, "ALL") == 0) {
        if (!strncmp(msg.text, MSG_STOP, strlen(MSG_STOP))) {
          must_stop = 1;
          printf("Consumer %d (Group %s) received stop message\n", id, group);
        } else {
          printf("Consumer %d (Group %s) received: %s\n", id, group, msg.text);
        }
      }
    } else if (bytes_read == 0) {
      printf("Consumer %d (Group %s) pipe closed\n", id, group);
      break;
    } else {
      perror("read");
      break;
    }
  }

  close(pipe_fd);
}

void dispatcher(mqd_t mq, int consumer_pipes[NUM_CONSUMERS]) {
  Message msg;
  int must_stop = 0;
  int stop_messages_sent = 0;

  while (!must_stop || stop_messages_sent < NUM_CONSUMERS) {
    ssize_t bytes_read = mq_receive(mq, (char *)&msg, sizeof(Message), NULL);

    if (bytes_read >= 0) {
      for (int i = 0; i < NUM_CONSUMERS; i++) {
        if (write(consumer_pipes[i], &msg, sizeof(Message)) == -1) {
          perror("write");
        }
      }

      if (!strncmp(msg.text, MSG_STOP, strlen(MSG_STOP))) {
        must_stop = 1;
        stop_messages_sent++;
      }
    } else if (errno == EAGAIN) {
      // Queue is empty, but we haven't sent all stop messages yet
      if (must_stop) {
        Message stop_msg;
        strncpy(stop_msg.group, "ALL", MAX_GROUP_NAME);
        strncpy(stop_msg.text, MSG_STOP, sizeof(stop_msg.text));
        for (int i = 0; i < NUM_CONSUMERS; i++) {
          if (write(consumer_pipes[i], &stop_msg, sizeof(Message)) == -1) {
            perror("write stop message");
          }
        }
        stop_messages_sent = NUM_CONSUMERS;
      }
    } else {
      perror("mq_receive");
      break;
    }
  }

  // Close all consumer pipes
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    close(consumer_pipes[i]);
  }
}

int main() {
  mqd_t mq;
  struct mq_attr attr;
  pid_t pids[NUM_PRODUCERS + NUM_CONSUMERS + 1]; // +1 for dispatcher
  int consumer_pipes[NUM_CONSUMERS][2];

  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(Message);
  attr.mq_curmsgs = 0;

  mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
  if (mq == (mqd_t)-1) {
    perror("mq_open");
    exit(1);
  }

  // Create pipes for consumers
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    if (pipe(consumer_pipes[i]) == -1) {
      perror("pipe");
      exit(1);
    }
  }

  // Create consumer processes
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    const char *group = GROUP_NAMES[i % NUM_GROUPS];
    pids[i] = fork();
    if (pids[i] < 0) {
      perror("fork");
      exit(1);
    } else if (pids[i] == 0) {
      // Child (consumer) process
      close(consumer_pipes[i][1]); // Close write end
      consumer(i, group, consumer_pipes[i][0]);
      exit(0);
    }
    close(consumer_pipes[i][0]); // Close read end in parent
  }

  // Create dispatcher process
  pids[NUM_CONSUMERS] = fork();
  if (pids[NUM_CONSUMERS] < 0) {
    perror("fork");
    exit(1);
  } else if (pids[NUM_CONSUMERS] == 0) {
    // Child (dispatcher) process
    int dispatcher_pipes[NUM_CONSUMERS];
    for (int i = 0; i < NUM_CONSUMERS; i++) {
      dispatcher_pipes[i] = consumer_pipes[i][1];
    }
    dispatcher(mq, dispatcher_pipes);
    exit(0);
  }

  // Create producer processes
  for (int i = 0; i < NUM_PRODUCERS; i++) {
    pids[NUM_CONSUMERS + 1 + i] = fork();
    if (pids[NUM_CONSUMERS + 1 + i] < 0) {
      perror("fork");
      exit(1);
    } else if (pids[NUM_CONSUMERS + 1 + i] == 0) {
      // Child (producer) process
      producer(i, mq);
      exit(0);
    }
  }

  // Wait for all producers to finish
  for (int i = 0; i < NUM_PRODUCERS; i++) {
    waitpid(pids[NUM_CONSUMERS + 1 + i], NULL, 0);
  }

  // Send stop messages to each group
  Message stop_msg;
  strncpy(stop_msg.text, MSG_STOP, sizeof(stop_msg.text));
  for (int i = 0; i < NUM_GROUPS; i++) {
    strncpy(stop_msg.group, GROUP_NAMES[i], MAX_GROUP_NAME);
    if (send_with_retry(mq, (char *)&stop_msg, sizeof(Message), 0) == -1) {
      fprintf(stderr, "Failed to send stop message to group %s\n",
              GROUP_NAMES[i]);
    }
  }

  // Wait for dispatcher to finish
  waitpid(pids[NUM_CONSUMERS], NULL, 0);

  // Wait for all consumers to finish
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    waitpid(pids[i], NULL, 0);
  }

  // Cleanup
  mq_close(mq);
  mq_unlink(QUEUE_NAME);

  return 0;
}
