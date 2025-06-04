#ifndef SIMULATION_CORE_H
#define SIMULATION_CORE_H

#include "throol.h"
#include "body_data.h"
#include "quadtree.h"
#include "simulation_interface.h"

typedef struct SimulationCore {
  BodyData *bodies;
  SimulationParams params;

  QuadTree *qt;
} SimulationCore;

SimulationCore *sim_core_create(SimulationParams params, int qt_node_capacity);
void sim_core_destroy(SimulationCore *core);

// Physics-only functions
void sim_core_init_leapfrog(SimulationCore *core);
void sim_core_step(SimulationCore *core);

#endif
