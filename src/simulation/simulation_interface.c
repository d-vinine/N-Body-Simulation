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

float compute_enclosed_mass(float dist, float min_r, float max_r,
                            float center_mass, float remaining_mass) {
  float dist2 = dist * dist;
  float min2 = min_r * min_r;
  float max2 = max_r * max_r;

  float frac = (dist2 - min2) / (max2 - min2);
  if (frac < 0.0f)
    frac = 0.0f;
  if (frac > 1.0f)
    frac = 1.0f;

  return center_mass + remaining_mass * frac;
}

void sim_init_normal(BodyData *data, SimulationParams params, int start_idx,
                     int count, float total_mass, float center_mass_percent,
                     float min_radius, float max_radius, float center_x,
                     float center_y, float velocity_x, float velocity_y) {

  float center_mass = total_mass * center_mass_percent;
  float remaining_mass = total_mass - center_mass;
  float body_mass = remaining_mass / (float)(count - 1);

  // Central mass
  data->x[start_idx] = center_x;
  data->y[start_idx] = center_y;
  data->vx[start_idx] = velocity_x;
  data->vy[start_idx] = velocity_y;
  data->mass[start_idx] = center_mass;

  for (int i = 1; i < count; ++i) {
    int idx = start_idx + i;

    // Generate position using polar coordinates
    float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;

    // Uniform distribution in annulus (area-wise, not just radius)
    float t = (float)rand() / RAND_MAX;
    float radius =
        sqrtf(t * (max_radius * max_radius - min_radius * min_radius) +
              min_radius * min_radius);

    float x = center_x + radius * cosf(angle);
    float y = center_y + radius * sinf(angle);

    data->x[idx] = x;
    data->y[idx] = y;
    data->mass[idx] = body_mass;

    // Radial vector
    float dx = x - center_x;
    float dy = y - center_y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Orbital velocity magnitude (around central mass)
    float enclosed_mass = compute_enclosed_mass(dist, min_radius, max_radius,
                                                center_mass, remaining_mass);
    float v = sqrtf(params.G * enclosed_mass / dist);

    // Tangential velocity
    float vx = -v * dy / dist;
    float vy = v * dx / dist;

    data->vx[idx] = vx + velocity_x;
    data->vy[idx] = vy + velocity_y;
  }
}
