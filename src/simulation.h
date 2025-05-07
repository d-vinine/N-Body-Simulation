#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <stdatomic.h>

typedef struct Simulation {
  float *x;
  float *y;

  float *v_x;
  float *v_y;

  float *a_x;
  float *a_y;

  float mass; ///< Every body has the same mass

  float G;   ///< Gravitational Constant
  float eps; ///< Softening factor
  float dt;  ///< Time step
  int body_num;
} Simulation;

Simulation create_simulation(int body_num, float mass, float G, float eps, float dt);
void free_simulation(Simulation *sim);
void step_simulation_parallel(Simulation *sim, int thread_num);

#endif
