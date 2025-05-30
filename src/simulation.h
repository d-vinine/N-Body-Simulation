#ifndef SIMULATION_HEADER
#define SIMULATION_HEADER

#include "throol.h"

typedef struct Simulation {
  // Body data
  float *x;
  float *y;

  float *v_x;
  float *v_y;

  float *a_x;
  float *a_y;

  float *mass;

  // Simulation parameters
  float G;
  float eps;
  float dt;

  float theta; // quadtree

  int body_num;

  // Other stuff
  Throol *throol;
  int first_step;

} Simulation;

Simulation *sim_create(int body_num, float G, float eps, float dt, float theta,
                       int thread_num);
void sim_destroy(Simulation *sim);
void sim_init_leapfrog(Simulation *sim);
void sim_step_leapfrog(Simulation *sim);

// Bulk initializations [TODO]
// void sim_init_from_file(Simulation *sim, const char *filename); [TODO]
// void sim_init_random_sphere(Simulation *sim, float radius, float center_x,
// float center_y); [TODO]

void sim_init_galaxy_disk(Simulation *sim, int start_idx, int body_num,
                          float total_mass, float scale_length, float cx,
                          float cy, float v_cx, float v_cy);

void sim_init_galaxy_disk_with_center(Simulation *sim, int start_idx,
                                      int total_particles, float disk_mass,
                                      float central_mass, float scale_length,
                                      float cx, float cy, float v_cx,
                                      float v_cy);
// Bulk output [TODO]
// void sim_export_positions(Simulation *sim, const char *filename); [TODO]
// void sim_export_snapshot(Simulation *sim, const char *filename); [TODO]

#endif // SIMULATION_HEADER
