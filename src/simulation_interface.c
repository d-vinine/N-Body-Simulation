#include "simulation_interface.h"
#include "body_data.h"
#include "helper_funcs.h"
#include <math.h>
#include <stdlib.h>

void sim_init_uniform(BodyData *data, float min_x, float max_x, float min_y,
                      float max_y, float max_velocity) {
  for (int i = 0; i < data->count; i++) {
    data->x[i] = min_x + (max_x - min_x) * ((float)rand() / RAND_MAX);
    data->y[i] = min_y + (max_y - min_y) * ((float)rand() / RAND_MAX);

    float angle = 2.0f * M_PI * ((float)rand() / RAND_MAX);
    float speed = max_velocity * ((float)rand() / RAND_MAX);

    data->vx[i] = speed * cosf(angle);
    data->vy[i] = speed * sinf(angle);
    data->mass[i] = 1.0f; // Uniform mass
  }
}

void sim_init_galaxy(BodyData *data, SimulationParams params, int start_idx,
                     int count, float total_mass, float central_mass_percentage,
                     float scale_length, float center_x, float center_y,
                     float velocity_x, float velocity_y, float temp) {

  // Set up the central massive particle first
  float central_mass = total_mass * central_mass_percentage;
  data->x[start_idx] = center_x;
  data->y[start_idx] = center_y;
  data->vx[start_idx] = velocity_x;
  data->vy[start_idx] = velocity_y;

  data->mass[start_idx] = central_mass;
  data->ax[start_idx] = 0.0;
  data->ay[start_idx] = 0.0;

  int disk_particles = count - 1;              // Reserve 1 for central mass
  int bulge_particles = disk_particles * 0.15; // 15% in bulge
  int pure_disk_particles = disk_particles - bulge_particles;

  float disk_mass = total_mass * (1 - central_mass_percentage);

  for (int i = 0; i < disk_particles; i++) {
    int idx = start_idx + 1 + i; // +1 to skip the central mass
    float r, theta;

    if (i < bulge_particles) {
      // BULGE COMPONENT - concentrated near center
      float u = (float)rand() / RAND_MAX;
      r = scale_length * 0.2 * sqrt(-log(u)); // Tighter than disk
      theta = 2.0 * M_PI * rand() / RAND_MAX;
      data->mass[idx] =
          disk_mass * 2.0 / disk_particles; // Higher mass for bulge
    } else {
      // DISK COMPONENT - exponential profile
      float u1, u2;
      do {
        u1 = (float)rand() / RAND_MAX;
        u2 = (float)rand() / RAND_MAX;
        r = -scale_length * log(u1);
      } while (u2 > exp(-r / scale_length) && r > 0.5); // Avoid r=0

      theta = 2.0 * M_PI * rand() / RAND_MAX;
      data->mass[idx] = disk_mass / disk_particles;
    }

    // Position
    data->x[idx] = center_x + r * cos(theta);
    data->y[idx] = center_y + r * sin(theta);
    // PROPER CIRCULAR VELOCITY including central mass
    float total_enclosed_mass =
        central_mass + disk_mass * (1.0 - exp(-r / scale_length));
    float v_circ = sqrt(params.G * total_enclosed_mass /
                        (r + 0.1)); // +0.1 prevents division by zero

    // Velocity components with dispersion
    float sigma_r = temp * v_circ * 0.2; // Radial velocity dispersion
    float sigma_t = temp * v_circ * 0.1; // Tangential velocity dispersion

    // Base circular motion + random components
    data->vx[idx] = velocity_x - v_circ * sin(theta) +
                    gaussian_random() * sigma_t * cos(theta) +
                    gaussian_random() * sigma_r * sin(theta);
    data->vy[idx] = velocity_y + v_circ * cos(theta) +
                    gaussian_random() * sigma_t * sin(theta) +
                    gaussian_random() * sigma_r * cos(theta);

    // Initialize accelerations
    data->ax[idx] = 0.0;
    data->ay[idx] = 0.0;
  }
}
