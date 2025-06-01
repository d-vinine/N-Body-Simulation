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

void sim_init_halo(BodyData *data, SimulationParams params, int start_idx,
                   int count, float halo_mass, float halo_scale_radius,
                   float center_x, float center_y, float velocity_x,
                   float velocity_y, float central_mass) {

  for (int i = 0; i < count; i++) {
    int idx = start_idx + i;

    // NFW-like radial distribution using rejection sampling
    float r, u1, u2;
    do {
      u1 = (float)rand() / RAND_MAX;
      u2 = (float)rand() / RAND_MAX;
      r = halo_scale_radius * u1 / (1.0 - u1);
    } while (u2 > 1.0 / ((1.0 + r / halo_scale_radius) *
                         (1.0 + r / halo_scale_radius)) &&
             r < halo_scale_radius * 50.0);

    // Isotropic angular distribution
    float theta = 2.0 * M_PI * rand() / RAND_MAX;
    float phi = acos(2.0 * rand() / RAND_MAX - 1.0);

    // Position (3D projected to 2D)
    data->x[idx] = center_x + r * cos(theta) * sin(phi);
    data->y[idx] = center_y + r * sin(theta) * sin(phi);

    // Dark matter has mostly random motion
    float halo_velocity_dispersion = sqrt(
        params.G * (central_mass + halo_mass * 0.5) / (r + halo_scale_radius));
    data->vx[idx] =
        velocity_x + gaussian_random() * halo_velocity_dispersion * 0.5;
    data->vy[idx] =
        velocity_y + gaussian_random() * halo_velocity_dispersion * 0.5;

    data->mass[idx] = halo_mass / count;
    data->ax[idx] = 0.0;
    data->ay[idx] = 0.0;
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
  data->ax[idx] = 0.0;
  data->ay[idx] = 0.0;
}

void sim_init_bulge(BodyData *data, SimulationParams params, int start_idx,
                    int count, float bulge_mass, float scale_length,
                    float center_x, float center_y, float velocity_x,
                    float velocity_y, float temp, float central_mass,
                    float halo_mass, float halo_scale_radius) {

  for (int i = 0; i < count; i++) {
    int idx = start_idx + i;

    // Concentrated near center - tighter than disk
    float u = (float)rand() / RAND_MAX;
    float r = scale_length * 0.2 * sqrt(-log(u));
    float theta = 2.0 * M_PI * rand() / RAND_MAX;

    // Position
    data->x[idx] = center_x + r * cos(theta);
    data->y[idx] = center_y + r * sin(theta);

    // Include halo mass in circular velocity calculation
    float enclosed_halo_mass = halo_mass * (r / (r + halo_scale_radius));
    float total_enclosed_mass =
        central_mass + enclosed_halo_mass + bulge_mass * 0.15;
    float v_circ = sqrt(params.G * total_enclosed_mass / (r + 0.1));

    // Bulge has higher velocity dispersion, less organized rotation
    float sigma_r = temp * v_circ * 0.4;
    float sigma_t = temp * v_circ * 0.3;

    data->vx[idx] = velocity_x - v_circ * sin(theta) * 0.7 + // Reduced rotation
                    gaussian_random() * sigma_t * cos(theta) +
                    gaussian_random() * sigma_r * sin(theta);
    data->vy[idx] = velocity_y + v_circ * cos(theta) * 0.7 +
                    gaussian_random() * sigma_t * sin(theta) +
                    gaussian_random() * sigma_r * cos(theta);

    data->mass[idx] = bulge_mass / count;
    data->ax[idx] = 0.0;
    data->ay[idx] = 0.0;
  }
}

void sim_init_disk(BodyData *data, SimulationParams params, int start_idx,
                   int count, float disk_mass, float scale_length,
                   float center_x, float center_y, float velocity_x,
                   float velocity_y, float temp, float central_mass,
                   float halo_mass, float halo_scale_radius) {

  for (int i = 0; i < count; i++) {
    int idx = start_idx + i;
    float r, theta;

    // Exponential disk profile
    float u1, u2;
    do {
      u1 = (float)rand() / RAND_MAX;
      u2 = (float)rand() / RAND_MAX;
      r = -scale_length * log(u1);
    } while (u2 > exp(-r / scale_length) && r > 0.5);

    theta = 2.0 * M_PI * rand() / RAND_MAX;

    // Position
    data->x[idx] = center_x + r * cos(theta);
    data->y[idx] = center_y + r * sin(theta);

    // Circular velocity including halo contribution
    float enclosed_halo_mass = halo_mass * (r / (r + halo_scale_radius));
    float total_enclosed_mass = central_mass + enclosed_halo_mass +
                                disk_mass * (1.0 - exp(-r / scale_length));
    float v_circ = sqrt(params.G * total_enclosed_mass / (r + 0.1));

    // Disk has organized rotation with small dispersion
    float sigma_r = temp * v_circ * 0.1;
    float sigma_t = temp * v_circ * 0.05;

    data->vx[idx] = velocity_x - v_circ * sin(theta) +
                    gaussian_random() * sigma_t * cos(theta) +
                    gaussian_random() * sigma_r * sin(theta);
    data->vy[idx] = velocity_y + v_circ * cos(theta) +
                    gaussian_random() * sigma_t * sin(theta) +
                    gaussian_random() * sigma_r * cos(theta);

    data->mass[idx] = disk_mass / count;
    data->ax[idx] = 0.0;
    data->ay[idx] = 0.0;
  }
}

void sim_init_galaxy(BodyData *data, SimulationParams params, int start_idx,
                     int count, float total_mass, float scale_length,
                     float center_x, float center_y, float velocity_x,
                     float velocity_y, float temp) {

  // Calculate realistic central mass based on total galaxy mass
  // Typical SMBH is 0.1% to 0.5% of total galaxy mass (M-sigma relation)
  float central_mass_percentage = 0.002f; // 0.2% - realistic for most galaxies
  float central_mass = total_mass * central_mass_percentage;
  float remaining_mass = total_mass * (1 - central_mass_percentage);
  float halo_mass = remaining_mass * 0.95;    // 95% in dark matter
  float visible_mass = remaining_mass * 0.05; // 5% in visible matter

  // Particle distribution
  int remaining_particles = count - 1;
  int halo_particles = remaining_particles * 0.5; // 50% for halo
  int disk_particles = remaining_particles * 0.4; // 15% for disk
  int bulge_particles =
      remaining_particles - halo_particles - disk_particles; // Rest for bulge

  // Mass distribution for visible components
  float disk_mass = visible_mass * 0.4;  // 40% of visible in disk
  float bulge_mass = visible_mass * 0.2; // 20% of visible in bulge

  // Halo scale
  float halo_scale_radius = scale_length * 10.0;

  int current_idx = start_idx;

  // Initialize central mass
  sim_init_central_mass(data, current_idx, central_mass, center_x, center_y,
                        velocity_x, velocity_y);
  current_idx++;

  // Initialize dark matter halo
  sim_init_halo(data, params, current_idx, halo_particles, halo_mass,
                halo_scale_radius, center_x, center_y, velocity_x, velocity_y,
                central_mass);
  current_idx += halo_particles;

  // Initialize bulge
  sim_init_bulge(data, params, current_idx, bulge_particles, bulge_mass,
                 scale_length, center_x, center_y, velocity_x, velocity_y, temp,
                 central_mass, halo_mass, halo_scale_radius);
  current_idx += bulge_particles;

  // Initialize disk
  sim_init_disk(data, params, current_idx, disk_particles, disk_mass,
                scale_length, center_x, center_y, velocity_x, velocity_y, temp,
                central_mass, halo_mass, halo_scale_radius);
}
