#include "simulation/simulation_core.h"
#include "simulation/simulation_interface.h"
#include "simulation/simulation_renderer.h"
#include <raylib.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080
#define FPS 60

void init_sim(const SimulationCore *core) {
  sim_init_galaxy(core->bodies, core->params, 0, core->bodies->count, 1e6, 100,
                  WIDTH/2.0, HEIGHT/2.0, 0, 0, 0.04);

  // sim_init_uniform(core->bodies, 0, 200, 0, 200, 2);
  sim_core_init_leapfrog((SimulationCore *)core);
}

int main(void) {
<<<<<<< HEAD
  SimulationParams params = {.body_count = 10000, 
                             .thread_count = 1,
=======
  SimulationParams params = {.body_count = 200000, 
>>>>>>> dev
                             .G = 0.1,
                             .eps = 0.5,
                             .dt = 0.01,
                             .theta = 0.5};

  SimulationCore *core = sim_core_create(params, 1024);
  init_sim(core);

  InitWindow(WIDTH, HEIGHT, "N-Body Sim");
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
