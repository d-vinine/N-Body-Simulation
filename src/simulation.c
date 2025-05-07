#define _POSIX_C_SOURCE 200112L
#include "simulation.h"
#include "sharedmemory.h"
#include <mm_malloc.h>

#include <immintrin.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define BODY_NUM 16000
#define PI 3.14159265359
void init_sim(Simulation *sim);

Simulation create_simulation(int body_num, float mass, float G, float eps,
                             float dt) {

  Simulation ret;

  ret.body_num = body_num;
  ret.G = G;
  ret.eps = eps;
  ret.dt = dt;

  ret.mass = mass;

  ret.x = calloc(body_num, sizeof(float));
  ret.y = calloc(body_num, sizeof(float));
  ret.v_x = calloc(body_num, sizeof(float));
  ret.v_y = calloc(body_num, sizeof(float));
  ret.a_x = calloc(body_num, sizeof(float));
  ret.a_y = calloc(body_num, sizeof(float));

  return ret;
}

void free_simulation(Simulation *sim) {
  free(sim->x);
  free(sim->y);
  free(sim->v_x);
  free(sim->v_y);
  free(sim->a_x);
  free(sim->a_y);
}

typedef struct ThreadArgs {
  Simulation *sim;
  int start;
  int end;
  float *a_x_local;
  float *a_y_local;
} ThreadArgs;

void *thread_calc_acc(void *t_args) {

  ThreadArgs *args = (ThreadArgs *)t_args;
  Simulation *sim = args->sim;

  for (int i = args->start; i < args->end; i++) {
    __m256 xi = _mm256_set1_ps(sim->x[i]);
    __m256 yi = _mm256_set1_ps(sim->y[i]);

    __m256 acc_xi = _mm256_setzero_ps();
    __m256 acc_yi = _mm256_setzero_ps();

    int j = i + 1;
    for (; j + 7 < sim->body_num; j += 8) {
      __m256 xj = _mm256_loadu_ps(&sim->x[j]);
      __m256 yj = _mm256_loadu_ps(&sim->y[j]);

      __m256 dx = _mm256_sub_ps(xj, xi);
      __m256 dy = _mm256_sub_ps(yj, yi);

      __m256 dist2 = _mm256_fmadd_ps(
          dx, dx, _mm256_fmadd_ps(dy, dy, _mm256_set1_ps(sim->eps * sim->eps)));
      __m256 inv_dist = _mm256_rsqrt_ps(dist2);

      __m256 a = _mm256_mul_ps(_mm256_set1_ps(sim->G * sim->mass),
                               _mm256_mul_ps(inv_dist, inv_dist));
      __m256 ax = _mm256_mul_ps(a, _mm256_mul_ps(dx, inv_dist));
      __m256 ay = _mm256_mul_ps(a, _mm256_mul_ps(dy, inv_dist));

      acc_xi = _mm256_add_ps(acc_xi, ax);
      acc_yi = _mm256_add_ps(acc_yi, ay);

      __m256 axj = _mm256_loadu_ps(&args->a_x_local[j]);
      __m256 ayj = _mm256_loadu_ps(&args->a_y_local[j]);
      axj = _mm256_sub_ps(axj, ax);
      ayj = _mm256_sub_ps(ayj, ay);
      _mm256_storeu_ps(&args->a_x_local[j], axj);
      _mm256_storeu_ps(&args->a_y_local[j], ayj);
    }

    float sum_xi = 0;
    float sum_yi = 0;

    __m128 low = _mm256_castps256_ps128(acc_xi);
    __m128 high = _mm256_extractf128_ps(acc_xi, 1);
    __m128 sum1 = _mm_add_ps(low, high);

    __m128 shuff1 = _mm_movehdup_ps(sum1);
    __m128 sum2 = _mm_add_ps(sum1, shuff1);
    __m128 shuff2 = _mm_movehl_ps(shuff1, sum2);
    __m128 sum3 = _mm_add_ps(sum2, shuff2);
    sum_xi = _mm_cvtss_f32(sum3);

    low = _mm256_castps256_ps128(acc_yi);
    high = _mm256_extractf128_ps(acc_yi, 1);
    sum1 = _mm_add_ps(low, high);

    shuff1 = _mm_movehdup_ps(sum1);
    sum2 = _mm_add_ps(sum1, shuff1);
    shuff2 = _mm_movehl_ps(shuff1, sum2);
    sum3 = _mm_add_ps(sum2, shuff2);
    sum_yi = _mm_cvtss_f32(sum3);

    for (; j < sim->body_num; j++) {
      float dx = sim->x[j] - sim->x[i];
      float dy = sim->y[j] - sim->y[i];
      float dist2 = dx * dx + dy * dy + sim->eps * sim->eps;
      float dist = sqrtf(dist2);

      float acc = sim->G * sim->mass / dist2;
      float acc_x = acc * dx / dist;
      float acc_y = acc * dy / dist;

      sum_xi += acc_x;
      sum_yi += acc_y;

      args->a_x_local[j] -= acc_x;
      args->a_y_local[j] -= acc_y;
    }

    args->a_x_local[i] += sum_xi;
    args->a_y_local[i] += sum_yi;
  }

  return NULL;
}

void initialize_leapfrog(Simulation *sim, int thread_num) {
  pthread_t pool[thread_num];
  ThreadArgs thread_args[thread_num];

  float *a_x_locals[thread_num];
  float *a_y_locals[thread_num];
  for (int t = 0; t < thread_num; t++) {
    a_x_locals[t] = calloc(sim->body_num, sizeof(float));
    a_y_locals[t] = calloc(sim->body_num, sizeof(float));
  }

  for (int i = 0; i < thread_num; i++) {
    int block = sim->body_num / thread_num;
    int left = sim->body_num % thread_num;

    int start = i * block + (i < left ? i : left);
    int end = start + block + (i < left ? 1 : 0);

    thread_args[i] = (ThreadArgs){
        .sim = sim,
        .start = start,
        .end = end,
        .a_x_local = a_x_locals[i],
        .a_y_local = a_y_locals[i],
    };
    pthread_create(&pool[i], NULL, thread_calc_acc, &thread_args[i]);
  }

  for (int i = 0; i < thread_num; i++) {
    pthread_join(pool[i], NULL);
  }

  for (int i = 0; i < sim->body_num; i++) {
    sim->a_x[i] = 0;
    sim->a_y[i] = 0;
    for (int j = 0; j < thread_num; j++) {
      sim->a_x[i] += a_x_locals[j][i];
      sim->a_y[i] += a_y_locals[j][i];
    }
  }

  for (int i = 0; i < sim->body_num; i++) {
    sim->v_x[i] -= 0.5 * sim->a_x[i] * sim->dt;
    sim->v_y[i] -= 0.5 * sim->a_y[i] * sim->dt;
  }

  for (int t = 0; t < thread_num; t++) {
    free(a_x_locals[t]);
    free(a_y_locals[t]);
  }
}

void step_simulation_parallel(Simulation *sim, int thread_num) {
  pthread_t pool[thread_num];
  ThreadArgs thread_args[thread_num];

  for (int i = 0; i < sim->body_num; i++) {
    sim->x[i] += sim->v_x[i] * sim->dt;
    sim->y[i] += sim->v_y[i] * sim->dt;
  }

  float *a_x_locals[thread_num];
  float *a_y_locals[thread_num];
  for (int t = 0; t < thread_num; t++) {
    a_x_locals[t] = calloc(sim->body_num, sizeof(float));
    a_y_locals[t] = calloc(sim->body_num, sizeof(float));
  }

  for (int i = 0; i < thread_num; i++) {
    int block = sim->body_num / thread_num;
    int left = sim->body_num % thread_num;

    int start = i * block + (i < left ? i : left);
    int end = start + block + (i < left ? 1 : 0);

    thread_args[i] = (ThreadArgs){
        .sim = sim,
        .start = start,
        .end = end,
        .a_x_local = a_x_locals[i],
        .a_y_local = a_y_locals[i],
    };
    pthread_create(&pool[i], NULL, thread_calc_acc, &thread_args[i]);
  }

  for (int i = 0; i < thread_num; i++) {
    pthread_join(pool[i], NULL);
  }

  for (int i = 0; i < sim->body_num; i++) {
    sim->a_x[i] = 0;
    sim->a_y[i] = 0;
    for (int j = 0; j < thread_num; j++) {
      sim->a_x[i] += a_x_locals[j][i];
      sim->a_y[i] += a_y_locals[j][i];
    }
  }

  for (int i = 0; i < sim->body_num; i++) {
    sim->v_x[i] += sim->a_x[i] * sim->dt;
    sim->v_y[i] += sim->a_y[i] * sim->dt;
  }

  for (int t = 0; t < thread_num; t++) {
    free(a_x_locals[t]);
    free(a_y_locals[t]);
  }
}

int main(void) {
  srand(time(0));

  SharedMemory *shm = create_shared_memory("simulation", BODY_NUM);
  shm->body_num = BODY_NUM;

  float *shm_x = (float *)((char *)shm + shm->x_offset);
  float *shm_y = (float *)((char *)shm + shm->y_offset);

  // Create and initialize simulation
  Simulation sim = create_simulation(BODY_NUM, 10, 2e4, 1, 5e-5);
  init_sim(&sim);
  initialize_leapfrog(&sim, 6);

  while (1) {
    clock_t start = clock();
    if (sem_trywait(&shm->signal[RESET]) == 0) {
      init_sim(&sim);
    }
    if (sem_trywait(&shm->signal[CLOSE]) == 0) {
      break;
    }

    step_simulation_parallel(&sim, 6);

    memcpy(shm_x, sim.x, BODY_NUM * sizeof(float));
    memcpy(shm_y, sim.y, BODY_NUM * sizeof(float));
    clock_t end = clock();

    double sim_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Sim Time: %f\n", sim_time);

    struct timespec delay = {.tv_sec = 0, .tv_nsec = 5000};
    nanosleep(&delay, NULL);
  }

  free_simulation(&sim);
  return 0;
}

float randfloat(float min, float max) {
  return (float)rand() / RAND_MAX * (max - min) + min;
}

void init_sim(Simulation *sim) {
  // Ensure even number of bodies for equal distribution
  int bodies_per_galaxy = sim->body_num / 2;

  // Galaxy 1 parameters
  float galaxy1_x = 300.0f;
  float galaxy1_y = 300.0f;
  float galaxy1_radius = 600.0f;

  // Galaxy 2 parameters
  float galaxy2_x = 500.0f;
  float galaxy2_y = 500.0f;
  float galaxy2_radius = 600.0f;

  // Initialize galaxy 1
  for (int i = 0; i < bodies_per_galaxy; i++) {
    // Position in a disk around galaxy center
    float angle = randfloat(0, 2 * PI);
    float distance = randfloat(0, galaxy1_radius);
    distance = pow(distance, 0.6); // More density toward center

    sim->x[i] = galaxy1_x + distance * cos(angle);
    sim->y[i] = galaxy1_y + distance * sin(angle);

    // Velocity for rotation (tangential to center)
    float rotation_speed = 1000.0f * sqrt(distance) / (distance + 50.0f);
    sim->v_x[i] = -rotation_speed * sin(angle);
    sim->v_y[i] = rotation_speed * cos(angle);

    // Add small random velocity component
    sim->v_x[i] += randfloat(-100.0f, 100.0f);
    sim->v_y[i] += randfloat(-100.0f, 100.0f);
  }

  // Initialize galaxy 2
  for (int i = 0; i < bodies_per_galaxy; i++) {
    // Position in a disk around galaxy center
    float angle = randfloat(0, 2 * PI);
    float distance = randfloat(0, galaxy2_radius);
    distance = pow(distance, 0.6); // More density toward center

    sim->x[i + bodies_per_galaxy] = galaxy2_x + distance * cos(angle);
    sim->y[i + bodies_per_galaxy] = galaxy2_y + distance * sin(angle);

    // Velocity for rotation (tangential to center)
    float rotation_speed = 1000.0f * sqrt(distance) / (distance + 50.0f);
    sim->v_x[i + bodies_per_galaxy] = -rotation_speed * sin(angle);
    sim->v_y[i + bodies_per_galaxy] = rotation_speed * cos(angle);

    // Add small random velocity component
    sim->v_x[i + bodies_per_galaxy] += randfloat(-100.0f, 100.0f);
    sim->v_y[i + bodies_per_galaxy] += randfloat(-100.0f, 100.0f);
  }

  // Give galaxies some relative velocity for possible interaction
  for (int i = 0; i < bodies_per_galaxy; i++) {
    sim->v_y[i] += 1000.0f;                      // Galaxy 1 moving left
    sim->v_y[i + bodies_per_galaxy] += -1000.0f; // Galaxy 2 moving right
  }
}
