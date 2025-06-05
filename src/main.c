#include "simulation_core.h"
#include "simulation_interface.h"
#include "simulation_renderer.h"
#include <raylib.h>

void init_sim(const SimulationCore *core) {
  sim_init_galaxy(core->bodies, core->params, 0, core->bodies->count, 1e6, 50,
                  512, 512, 0, 0, 0.05);

  // sim_init_uniform(core->bodies, 0, 200, 0, 200, 2);
  sim_core_init_leapfrog((SimulationCore *)core);
}

int main(void) {
  SimulationParams params = {.body_count = 60000, 
                             .G = 0.1,
                             .eps = 0.7,
                             .dt = 0.01,
                             .theta = 0.5};

  SimulationCore *core = sim_core_create(params, 1024);
  init_sim(core);

  InitWindow(1024, 1024, "N-Body Sim");
  SetTargetFPS(600);

  Camera2D cam = {0};
  cam.zoom = 1.0f;

  while (!WindowShouldClose()) {
    sim_core_step(core);
    sim_render_frame(core, &cam, init_sim);
  }

  sim_core_destroy(core);
  CloseWindow();
}
