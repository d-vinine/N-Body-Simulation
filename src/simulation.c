#define THROOL_IMPLEMENTATION
#include "quadtree.h"
#include "simulation.h"
#include <math.h>

Simulation *sim_create(int body_num, float G, float eps, float dt, float theta,
                       int thread_num) {
  Simulation *ret = calloc(1, sizeof(Simulation));

  ret->body_num = body_num;

  ret->G = G;
  ret->eps = eps;
  ret->dt = dt;

  ret->theta = theta;

  ret->x = calloc(body_num, sizeof(float));
  ret->y = calloc(body_num, sizeof(float));

  ret->v_x = calloc(body_num, sizeof(float));
  ret->v_y = calloc(body_num, sizeof(float));

  ret->a_x = calloc(body_num, sizeof(float));
  ret->a_y = calloc(body_num, sizeof(float));

  ret->mass = calloc(body_num, sizeof(float));

  ret->throol = throol_create(thread_num);

  return ret;
}

void sim_destroy(Simulation *sim) {
  throol_destroy(sim->throol);

  free(sim->x);
  free(sim->y);

  free(sim->v_x);
  free(sim->v_y);

  free(sim->a_x);
  free(sim->a_y);
}

static inline float fast_inv_sqrt(float x) {
  float xhalf = 0.5f * x;
  int i = *(int *)&x;        // Use int, not long long
  i = 0x5f3759df - (i >> 1); // Correct magic number for float
  x = *(float *)&i;
  x = x * (1.5f - xhalf * x * x); // Newton iteration
  return x;
}

static void sim_calc_acc(Simulation *sim, QuadTreeNode *node, int idx) {
  if (!node || node->is_empty)
    return;

  float dx = node->x - sim->x[idx];
  float dy = node->y - sim->y[idx];
  float dist2 = dx * dx + dy * dy + sim->eps * sim->eps;

  // Skip if too close (self-interaction or very close particles)
  if (dist2 < sim->eps * sim->eps * 0.1)
    return;

  float s = node->size;
  float s2 = s * s;
  float theta2_dist2 = sim->theta * sim->theta * dist2;

  if (node->is_leaf || (s2 < theta2_dist2)) {
    float inv_dist = fast_inv_sqrt(dist2);
    float inv_dist3 = inv_dist * inv_dist * inv_dist;

    float a = sim->G * node->mass * inv_dist3;
    sim->a_x[idx] += a * dx;
    sim->a_y[idx] += a * dy;
  } else {
    for (int j = 0; j < 4; j++) {
      sim_calc_acc(sim, node->children[j], idx);
    }
  }
}

typedef struct AccelTaskData {
  Simulation *sim;
  QuadTreeNode *root;

  int start;
  int end;
} AccelTaskData;

static void thread_calc_acc(void *args) {
  AccelTaskData *data = (AccelTaskData *)args;

  Simulation *sim = data->sim;
  QuadTreeNode *root = data->root;

  int start = data->start;
  int end = data->end;

  for (int i = start; i < end; i++) {
    sim->a_x[i] = 0.0f;
    sim->a_y[i] = 0.0f;

    sim_calc_acc(sim, root, i);
  }

  free(data);
}

static void thread_update_sim(void *args) {
  AccelTaskData *data = (AccelTaskData *)args;

  Simulation *sim = data->sim;
  QuadTreeNode *root = data->root;

  int start = data->start;
  int end = data->end;

  for (int i = start; i < end; i++) {
    sim->x[i] += sim->v_x[i] * sim->dt;
    sim->y[i] += sim->v_y[i] * sim->dt;

    sim->v_x[i] += sim->a_x[i] * sim->dt;
    sim->v_y[i] += sim->a_y[i] * sim->dt;
  }

  free(data);
}

static void thread_distribute_work(Simulation *sim, QuadTreeNode *root,
                                   ThroolFn fn) {

  int block = sim->body_num / sim->throol->thread_count;
  int remainder = sim->body_num % sim->throol->thread_count;

  for (int t = 0; t < sim->throol->thread_count; t++) {

    int start = t * block + (t < remainder ? t : remainder);
    int end = start + block + (t < remainder ? 1 : 0);

    AccelTaskData *data = calloc(1, sizeof(AccelTaskData));
    data->sim = sim;
    data->root = root;
    data->start = start;
    data->end = end;

    throol_add_task(sim->throol, (ThroolTask){.task_fn = fn, .args = data});
  }

  throol_wait(sim->throol);
}

void sim_init_leapfrog(Simulation *sim) {
  float max_x = -INFINITY, min_x = INFINITY;
  float max_y = -INFINITY, min_y = INFINITY;

  for (int i = 0; i < sim->body_num; i++) {
    if (sim->x[i] > max_x)
      max_x = sim->x[i];
    if (sim->x[i] < min_x)
      min_x = sim->x[i];
    if (sim->y[i] > max_y)
      max_y = sim->y[i];
    if (sim->y[i] < min_y)
      min_y = sim->y[i];
  }

  QuadTreeNode *root = qtnode_create_root(max_x, max_y, min_x, min_y);

  for (int i = 0; i < sim->body_num; i++) {
    qtnode_insert(root, sim->x[i], sim->y[i], sim->mass[i]);
  }

  thread_distribute_work(sim, root, thread_calc_acc);

  for (int i = 0; i < sim->body_num; i++) {
    sim->v_x[i] -= sim->a_x[i] * 0.5 * sim->dt;
    sim->v_y[i] -= sim->a_y[i] * 0.5 * sim->dt;
  }

  qtnode_destroy(root);
}

void sim_step_leapfrog(Simulation *sim) {
  float max_x = -INFINITY, min_x = INFINITY;
  float max_y = -INFINITY, min_y = INFINITY;

  for (int i = 0; i < sim->body_num; i++) {
    if (sim->x[i] > max_x)
      max_x = sim->x[i];
    if (sim->x[i] < min_x)
      min_x = sim->x[i];
    if (sim->y[i] > max_y)
      max_y = sim->y[i];
    if (sim->y[i] < min_y)
      min_y = sim->y[i];
  }

  QuadTreeNode *root = qtnode_create_root(max_x, max_y, min_x, min_y);

  for (int i = 0; i < sim->body_num; i++) {
    qtnode_insert(root, sim->x[i], sim->y[i], sim->mass[i]);
  }

  thread_distribute_work(sim, root, thread_calc_acc);
  thread_distribute_work(sim, root, thread_update_sim);

  qtnode_destroy(root);
}

static float calc_exp_radius(float scale_length) {
  float u = (float)rand() / RAND_MAX;
  return scale_length * sqrt(-log(u));
}

static float gaussian_random() {
  static int has_spare = 0;
  static float spare;

  if (has_spare) {
    has_spare = 0;
    return spare;
  }

  has_spare = 1;

  float u = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
  float v = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
  float s = u * u + v * v;

  while (s >= 1.0 || s == 0.0) {
    u = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    v = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    s = u * u + v * v;
  }

  s = sqrt(-2.0 * log(s) / s);
  spare = v * s;
  return u * s;
}

void sim_init_galaxy_disk(Simulation *sim, int start_idx, int body_num,
                          float total_mass, float scale_length, float cx,
                          float cy, float v_cx, float v_cy) {

  for (int i = 0; i < body_num; i++) {
    int idx = start_idx + i;

    // Sample radius with shifted exponential to avoid central spike
    float r, u1, u2;
    do {
      u1 = (float)rand() / RAND_MAX;
      u2 = (float)rand() / RAND_MAX;
      r = -scale_length * log(u1); // shifted exponential
    } while (u2 > (r)*exp(-r / scale_length));

    if (i < body_num * 0.1) {
      // Bulge particles - tighter distribution
      r = scale_length * 0.3 * sqrt(-log((float)rand() / RAND_MAX));
      sim->mass[idx] = total_mass * 3.0 / body_num; // Higher mass for bulge
    }

    float theta = 2.0 * M_PI * rand() / RAND_MAX;

    float x = r * cos(theta);
    float y = r * sin(theta);

    sim->x[idx] = x + cx;
    sim->y[idx] = y + cy;

    // Circular velocity (flat rotation curve approximation)
    float v_circ = sqrt(sim->G * total_mass * r / (r + scale_length));

    // Velocity dispersions
    float sigma_r = v_circ * 0.3;
    float sigma_t = v_circ * 0.15;

    // Tangential velocity + anisotropic velocity noise
    float vx = -v_circ * sin(theta) + gaussian_random() * sigma_t * cos(theta) +
               gaussian_random() * sigma_r * sin(theta);
    float vy = v_circ * cos(theta) + gaussian_random() * sigma_t * sin(theta) +
               gaussian_random() * sigma_r * cos(theta);

    sim->v_x[idx] = vx + v_cx;
    sim->v_y[idx] = vy + v_cy;

    sim->mass[idx] = total_mass / body_num;
    sim->a_x[idx] = 0.0;
    sim->a_y[idx] = 0.0;
  }
}

void sim_init_galaxy_disk_with_center(Simulation *sim, int start_idx,
                                      int total_particles, float disk_mass,
                                      float central_mass, float scale_length,
                                      float cx, float cy, float v_cx,
                                      float v_cy) {

  // Set up the central massive particle first
  sim->x[start_idx] = cx;
  sim->y[start_idx] = cy;
  sim->v_x[start_idx] = v_cx;
  sim->v_y[start_idx] = v_cy;
  sim->mass[start_idx] = central_mass;
  sim->a_x[start_idx] = 0.0;
  sim->a_y[start_idx] = 0.0;

  int disk_particles = total_particles - 1;    // Reserve 1 for central mass
  int bulge_particles = disk_particles * 0.15; // 15% in bulge
  int pure_disk_particles = disk_particles - bulge_particles;

  for (int i = 0; i < disk_particles; i++) {
    int idx = start_idx + 1 + i; // +1 to skip the central mass
    float r, theta;

    if (i < bulge_particles) {
      // BULGE COMPONENT - concentrated near center
      float u = (float)rand() / RAND_MAX;
      r = scale_length * 0.2 * sqrt(-log(u)); // Tighter than disk
      theta = 2.0 * M_PI * rand() / RAND_MAX;
      sim->mass[idx] =
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
      sim->mass[idx] = disk_mass / disk_particles;
    }

    // Position
    sim->x[idx] = cx + r * cos(theta);
    sim->y[idx] = cy + r * sin(theta);
    // PROPER CIRCULAR VELOCITY including central mass
    float total_enclosed_mass =
        central_mass + disk_mass * (1.0 - exp(-r / scale_length));
    float v_circ = sqrt(sim->G * total_enclosed_mass /
                        (r + 0.1)); // +0.1 prevents division by zero

    // Velocity components with dispersion
    float sigma_r = v_circ * 0.2; // Radial velocity dispersion
    float sigma_t = v_circ * 0.1; // Tangential velocity dispersion

    // Base circular motion + random components
    sim->v_x[idx] = v_cx - v_circ * sin(theta) +
                    gaussian_random() * sigma_t * cos(theta) +
                    gaussian_random() * sigma_r * sin(theta);
    sim->v_y[idx] = v_cy + v_circ * cos(theta) +
                    gaussian_random() * sigma_t * sin(theta) +
                    gaussian_random() * sigma_r * cos(theta);

    // Initialize accelerations
    sim->a_x[idx] = 0.0;
    sim->a_y[idx] = 0.0;
  }
}
