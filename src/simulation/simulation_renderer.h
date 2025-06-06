#ifndef SIMULATION_RENDER_H
#define SIMULATION_RENDER_H

#include "simulation_core.h"
#include <raylib.h>

void sim_render_frame(const SimulationCore *core, Camera2D *cam,
                      void init_sim(const SimulationCore *core));

#endif
