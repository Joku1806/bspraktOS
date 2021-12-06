#ifndef THREAD_H
#define THREAD_H

typedef enum {
  ready = 0,
  waiting = 1,
  running = 2,
  finished = 3,
} thread_status;

void thread_cleanup();

#endif