#include "quadtree.h"
#include <stdio.h>
#define THROOL_IMPLEMENTATION
#include "body_data.h"
#include "simulation_core.h"
#include "simulation_interface.h"
#include <math.h>

SimulationCore *sim_core_create(const SimulationParams params,
                                int qt_node_capacity) {
  SimulationCore *core = malloc(sizeof(SimulationCore));
  core->bodies = body_data_create(params.body_count);
  core->params = params;
  core->qt = qt_create(qt_node_capacity);
  return core;
}

void sim_core_destroy(SimulationCore *core) {
  if (core) {
    body_data_destroy(core->bodies);
    qt_destroy(core->qt);
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

  qt_set(core->qt, max_x, max_y, min_x, min_y);

  for (int i = 0; i < bodies->count; i++) {
    qt_insert(core->qt, bodies->x[i], bodies->y[i], bodies->mass[i]);
  }

  qt_propagate(core->qt);
  for (int i = 0; i < bodies->count; i++) {
    qt_acc(core->qt, bodies->x[i], bodies->y[i], core->params.theta,
           core->params.eps, core->params.G, &bodies->ax[i], &bodies->ay[i]);
  }

  for (int i = 0; i < bodies->count; i++) {
    bodies->vx[i] -= bodies->ax[i] * 0.5 * core->params.dt;
    bodies->vy[i] -= bodies->ay[i] * 0.5 * core->params.dt;
  }
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

  qt_set(core->qt, max_x, max_y, min_x, min_y);

  for (int i = 0; i < bodies->count; i++) {
    qt_insert(core->qt, bodies->x[i], bodies->y[i], bodies->mass[i]);
  }
  for (int i = 0; i < bodies->count; i++) {
    qt_acc(core->qt, bodies->x[i], bodies->y[i], core->params.theta,
           core->params.eps, core->params.G, &bodies->ax[i], &bodies->ay[i]);
  }

  qt_propagate(core->qt);

  for (int i = 0; i < bodies->count; i++) {
    qt_acc(core->qt, bodies->x[i], bodies->y[i], core->params.theta,
           core->params.eps, core->params.G, &bodies->ax[i], &bodies->ay[i]);
  }

  for (int i = 0; i < bodies->count; i++) {
    bodies->x[i] += bodies->vx[i] * core->params.dt;
    bodies->y[i] += bodies->vy[i] * core->params.dt;

    bodies->vx[i] += bodies->ax[i] * core->params.dt;
    bodies->vy[i] += bodies->ay[i] * core->params.dt;
  }
}
