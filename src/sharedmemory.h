#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__

#include <semaphore.h>

typedef enum Signals { RESET, CLOSE } Signals;

typedef struct SharedMemory {
  int body_num;

  size_t x_offset;
  size_t y_offset;

  sem_t signal[2];
} SharedMemory;

SharedMemory* create_shared_memory(char *filename, int body_num);
SharedMemory* attach_shared_memory(char *filename);
int detach_shared_memory(SharedMemory *shm);
int destroy_shared_memory(SharedMemory *shm, char *filename);

#endif
