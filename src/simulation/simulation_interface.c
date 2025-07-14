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
    data->mass[i] = 1.0f;
    data->ax[i] = 0.0f;
    data->ay[i] = 0.0f;
  }
}

void sim_init_central_mass(BodyData *data, int idx, float central_mass,
                           float center_x, float center_y, float velocity_x,
                           float velocity_y) {
  data->x[idx] = center_x;
  data->y[idx] = center_y;
  data->vx[idx] = velocity_x;
  data->vy[idx] = velocity_y;
  data->mass[idx] = central_mass;
  data->ax[idx] = 0.0f;
  data->ay[idx] = 0.0f;
}

void sim_init_disk(BodyData *data, SimulationParams params, int start_idx,
                   int count, float disk_mass, float scale_length,
                   float center_x, float center_y, float velocity_x,
                   float velocity_y, float temp, float central_mass,
                   float bulge_mass, float bulge_scale) {
  for (int i = 0; i < count; i++) {
    int idx = start_idx + i;
    float r, theta;

    // Rejection sampling for exponential disk profile
    float u1, u2;
    do {
      u1 = (float)rand() / RAND_MAX;
      u2 = (float)rand() / RAND_MAX;
      r = -scale_length * logf(u1);
    } while ((u2 > expf(-r / scale_length)) || (r < 0.1f));

    theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);

    data->x[idx] = center_x + r * cosf(theta);
    data->y[idx] = center_y + r * sinf(theta);

    // Enclosed mass includes disk, bulge, and central
    float disk_enclosed = disk_mass * (1.0f - expf(-r / scale_length));
    float bulge_enclosed = bulge_mass * (1.0f - expf(-r / bulge_scale));
    float total_enclosed_mass = central_mass + disk_enclosed + bulge_enclosed;

    float v_circ =
        sqrtf(params.G * total_enclosed_mass / (r + 0.05f)); // softened

    float sigma_r = temp * v_circ * 0.1f;
    float sigma_t = temp * v_circ * 0.05f;

    // Organized tangential motion with small dispersion
    data->vx[idx] = velocity_x - v_circ * sinf(theta) +
                    gaussian_random() * sigma_t * cosf(theta) +
                    gaussian_random() * sigma_r * sinf(theta);
    data->vy[idx] = velocity_y + v_circ * cosf(theta) +
                    gaussian_random() * sigma_t * sinf(theta) +
                    gaussian_random() * sigma_r * cosf(theta);

    data->mass[idx] = disk_mass / count;
    data->ax[idx] = 0.0f;
    data->ay[idx] = 0.0f;
  }
}

void sim_init_bulge(BodyData *data, SimulationParams params, int start_idx,
                    int count, float bulge_mass, float scale_radius,
                    float center_x, float center_y, float velocity_x,
                    float velocity_y, float temp) {
  for (int i = 0; i < count; i++) {
    int idx = start_idx + i;

    // Sample radius from exponential (approx Hernquist)
    float u = (float)rand() / RAND_MAX;
    float r = -scale_radius * logf(u);
    if (r < 0.05f)
      r = 0.05f;

    float theta = 2.0f * M_PI * ((float)rand() / RAND_MAX);

    data->x[idx] = center_x + r * cosf(theta);
    data->y[idx] = center_y + r * sinf(theta);

    float sigma = temp * sqrtf(params.G * bulge_mass / (r + 0.05f));

    // Isotropic random motion (pressure supported)
    data->vx[idx] = velocity_x + gaussian_random() * sigma;
    data->vy[idx] = velocity_y + gaussian_random() * sigma;

    data->mass[idx] = bulge_mass / count;
    data->ax[idx] = 0.0f;
    data->ay[idx] = 0.0f;
  }
}

void sim_init_galaxy(BodyData *data, SimulationParams params, int start_idx,
                     int count, float total_mass, float scale_length,
                     float center_x, float center_y, float velocity_x,
                     float velocity_y, float temp) {
  float central_mass_fraction = 0.001f;
  float central_mass = total_mass * central_mass_fraction;
  float remaining_mass = total_mass * (1.0f - central_mass_fraction);

  float visible_mass = remaining_mass * 0.05f;

  int remaining_particles = count - 1;
  int disk_particles = (int)(remaining_particles * 0.6f);
  int bulge_particles = remaining_particles - disk_particles;

  float disk_mass = visible_mass * 0.6f;
  float bulge_mass = visible_mass * 0.4f;

  int current_idx = start_idx;

  sim_init_central_mass(data, current_idx++, central_mass, center_x, center_y,
                        velocity_x, velocity_y);

  sim_init_disk(data, params, current_idx, disk_particles, disk_mass,
                scale_length, center_x, center_y, velocity_x, velocity_y, temp,
                central_mass, bulge_mass, scale_length * 0.5f);
  current_idx += disk_particles;

  sim_init_bulge(data, params, current_idx, bulge_particles, bulge_mass,
                 scale_length * 0.5f, center_x, center_y, velocity_x,
                 velocity_y, temp);
}

