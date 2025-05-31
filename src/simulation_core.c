#include <stdio.h>
#define THROOL_IMPLEMENTATION
#include "body_data.h"
#include "helper_funcs.h"
#include "simulation_core.h"
#include "simulation_interface.h"
#include <math.h>

typedef struct AccelTaskData {
  SimulationCore *core;
  int start;
  int end;
} AccelTaskData;

static void calc_acc(SimulationCore *core, QuadTreeNode *node, int idx) {
  BodyData *bodies = core->bodies;
  SimulationParams params = core->params;

  if (!node || node->is_empty)
    return;

  float dx = node->x - bodies->x[idx];
  float dy = node->y - bodies->y[idx];
  float dist2 = dx * dx + dy * dy + params.eps * params.eps;

  // Skip if too close (self-interaction or very close particles)
  if (dist2 < params.eps * params.eps * 0.1)
    return;

  float s = node->size;
  float s2 = s * s;
  float theta2_dist2 = params.theta * params.theta * dist2;

  if (node->is_leaf || (s2 < theta2_dist2)) {
    float inv_dist = fast_inv_sqrt(dist2);
    float inv_dist3 = inv_dist * inv_dist * inv_dist;

    float a = params.G * node->mass * inv_dist3;
    bodies->ax[idx] += a * dx;
    bodies->ay[idx] += a * dy;
  } else {
    for (int j = 0; j < 4; j++) {
      calc_acc(core, node->children[j], idx);
    }
  }
}

static void thread_calc_acc(void *args) {
  AccelTaskData *data = (AccelTaskData *)args;

  BodyData *bodies = data->core->bodies;
  QuadTreeNode *root = data->core->tree;

  int start = data->start;
  int end = data->end;

  for (int i = start; i < end; i++) {
    bodies->ax[i] = 0.0f;
    bodies->ay[i] = 0.0f;

    calc_acc(data->core, data->core->tree, i);
  }

  free(data);
}

static void thread_update_sim(void *args) {
  AccelTaskData *data = (AccelTaskData *)args;

  BodyData *bodies = data->core->bodies;
  QuadTreeNode *root = data->core->tree;
  SimulationParams params = data->core->params;

  int start = data->start;
  int end = data->end;

  for (int i = start; i < end; i++) {
    bodies->x[i] += bodies->vx[i] * params.dt;
    bodies->y[i] += bodies->vy[i] * params.dt;

    bodies->vx[i] += bodies->ax[i] * params.dt;
    bodies->vy[i] += bodies->ay[i] * params.dt;
  }

  free(data);
}

static void thread_distribute_work(SimulationCore *core, ThroolFn fn) {

  int block = core->bodies->count / core->throol->thread_count;
  int remainder = core->bodies->count % core->throol->thread_count;

  for (int t = 0; t < core->throol->thread_count; t++) {

    int start = t * block + (t < remainder ? t : remainder);
    int end = start + block + (t < remainder ? 1 : 0);

    AccelTaskData *data = calloc(1, sizeof(AccelTaskData));
    data->core = core;
    data->start = start;
    data->end = end;

    throol_add_task(core->throol, (ThroolTask){.task_fn = fn, .args = data});
  }

  throol_wait(core->throol);
}

SimulationCore *sim_core_create(const SimulationParams params) {
  SimulationCore *core = malloc(sizeof(SimulationCore));
  core->bodies = body_data_create(params.body_count);
  core->params = params;
  core->tree = NULL;
  core->throol = throol_create(params.thread_count);
  return core;
}

void sim_core_destroy(SimulationCore *core) {
  if (core) {
    if (core->tree) {
      qtnode_destroy(core->tree);
    }
    throol_destroy(core->throol);
    body_data_destroy(core->bodies);

    free(core);
  }
}

void sim_core_init_leapfrog(SimulationCore *core) {
  float max_x = -INFINITY, min_x = INFINITY;
  float max_y = -INFINITY, min_y = INFINITY;

  BodyData *bodies = core->bodies;

  for (int i = 0; i < bodies->count; i++) {
    if (bodies->x[i] > max_x)
      max_x = bodies->x[i];
    if (bodies->x[i] < min_x)
      min_x = bodies->x[i];
    if (bodies->y[i] > max_y)
      max_y = bodies->y[i];
    if (bodies->y[i] < min_y)
      min_y = bodies->y[i];
  }

  QuadTreeNode *root = qtnode_create_root(max_x, max_y, min_x, min_y);

  for (int i = 0; i < bodies->count; i++) {
    qtnode_insert(root, bodies->x[i], bodies->y[i], bodies->mass[i]);
  }

  core->tree = root;
  thread_distribute_work(core, thread_calc_acc);
  core->tree = NULL;
  for (int i = 0; i < bodies->count; i++) {
    bodies->vx[i] -= bodies->ax[i] * 0.5 * core->params.dt;
    bodies->vy[i] -= bodies->ay[i] * 0.5 * core->params.dt;
  }

  qtnode_destroy(root);
}

void sim_core_step(SimulationCore *core) {
  float max_x = -INFINITY, min_x = INFINITY;
  float max_y = -INFINITY, min_y = INFINITY;

  BodyData *bodies = core->bodies;

  for (int i = 0; i < bodies->count; i++) {
    if (bodies->x[i] > max_x)
      max_x = bodies->x[i];
    if (bodies->x[i] < min_x)
      min_x = bodies->x[i];
    if (bodies->y[i] > max_y)
      max_y = bodies->y[i];
    if (bodies->y[i] < min_y)
      min_y = bodies->y[i];
  }

  QuadTreeNode *root = qtnode_create_root(max_x, max_y, min_x, min_y);

  for (int i = 0; i < bodies->count; i++) {
    qtnode_insert(root, bodies->x[i], bodies->y[i], bodies->mass[i]);
  }

  core->tree = root;
  thread_distribute_work(core, thread_calc_acc);
  thread_distribute_work(core, thread_update_sim);
  core->tree = NULL;

  qtnode_destroy(root);
}
