#ifndef SIMULATION_INTERFACE_H
#define SIMULATION_INTERFACE_H

#include "body_data.h"

typedef struct SimulationParams {
  float G;          // Gravitational constant
  float eps;        // Softening length
  float dt;         // Time step
  float theta;      // BH opening angle
  int body_count;   // Number of bodies
  int thread_count; // Thread count
} SimulationParams;

// Pure initialization functions
void sim_init_galaxy(BodyData *data, SimulationParams params, int start_idx,
                     int count, float total_mass, float central_mass_percentage,
                     float scale_length, float center_x, float center_y,
                     float velocity_x, float velocity_y, float temp);

void sim_init_uniform(BodyData *data, float min_x, float max_x, float min_y,
                      float max_y, float max_velocity);

#endif
