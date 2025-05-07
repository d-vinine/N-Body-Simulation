#define _POSIX_C_SOURCE 200112L
#include "sharedmemory.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

SharedMemory *create_shared_memory(char *filename, int body_num) {
  size_t shm_size = sizeof(SharedMemory) + 2 * body_num * sizeof(float);

  int fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
  if (fd < 0) {
    perror("shm_open failed");
    return NULL;
  }

  if (ftruncate(fd, shm_size) == -1) {
    perror("ftruncate failed");
    close(fd);
    return NULL;
  }

  SharedMemory *shm =
      mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm == MAP_FAILED) {
    perror("mmap failed");
    close(fd);
    return NULL;
  }

  close(fd);

  shm->body_num = body_num;
  shm->x_offset = sizeof(SharedMemory);
  shm->y_offset = sizeof(SharedMemory) + body_num * sizeof(float);

  if (sem_init(&shm->signal[RESET], 1, 0) != 0) {
    perror("sem_init(RESET) failed");
    munmap(shm, shm_size);
    return NULL;
  }

  if (sem_init(&shm->signal[CLOSE], 1, 0) != 0) {
    perror("sem_init(CLOSE) failed");
    sem_destroy(&shm->signal[RESET]);
    munmap(shm, shm_size);
    return NULL;
  }

  return shm;
}

SharedMemory *attach_shared_memory(char *filename) {
  int fd = shm_open(filename, O_RDWR, 0666);
  if (fd < 0) {
    perror("shm_open failed");
    return NULL;
  }

  SharedMemory *temp = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (temp == MAP_FAILED) {
    perror("mmap failed");
    close(fd);
    return NULL;
  }

  size_t shm_size = sizeof(SharedMemory) + 2 * temp->body_num * sizeof(float);
  munmap(temp, sizeof(SharedMemory));

  SharedMemory *shm =
      mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);

  if (shm == MAP_FAILED) {
    perror("mmap failed");
    return NULL;
  }

  return shm;
}

int detach_shared_memory(SharedMemory *shm) {
  if (!shm)
    return -1;

  size_t shm_size = sizeof(SharedMemory) + 2 * shm->body_num * sizeof(float);
  return munmap(shm, shm_size);
}

int destroy_shared_memory(SharedMemory *shm, char *filename) {
  if (!shm || !filename)
    return -1;

  size_t shm_size = sizeof(SharedMemory) + 2 * shm->body_num * sizeof(float);

  sem_destroy(&shm->signal[RESET]);
  sem_destroy(&shm->signal[CLOSE]);

  if (munmap(shm, shm_size)) {
    perror("munmap failed");
    return -1;
  }

  if (shm_unlink(filename)) {
    perror("shm_unlink failed");
    return -1;
  }

  return 0;
}
