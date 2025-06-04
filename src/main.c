#include "simulation_core.h"
#include "simulation_interface.h"
#include "simulation_renderer.h"
#include <raylib.h>
#include <stdio.h>

void init_sim(const SimulationCore *core) {
  sim_init_galaxy(core->bodies, core->params, 0, core->bodies->count, 1e3, 20,
                  400, 400, 0, 0, 1.2);

  // sim_init_uniform(core->bodies, 0, 200, 0, 200, 2);
  sim_core_init_leapfrog((SimulationCore *)core);
}

int main(void) {
  SimulationParams params = {.body_count = 10000, 
                             .thread_count = 1,
                             .G = 0.1,
                             .eps = 0.1,
                             .dt = 0.001,
                             .theta = 0.5};

  SimulationCore *core = sim_core_create(params);
  init_sim(core);

  InitWindow(800, 800, "N-Body Sim");
  SetTargetFPS(6000);

  Camera2D cam = {0};
  cam.zoom = 1.0f;

  while (!WindowShouldClose()) {
    sim_core_step(core);
    sim_render_frame(core, &cam, init_sim);
    printf("FPS: %d\n", GetFPS()); 
  }

  sim_core_destroy(core);
  CloseWindow();
}
