#include "raylib.h"
#include "raymath.h"
#include "simulation/simulation_core.h"
#include "simulation/simulation_interface.h"

#define WIDTH 1024
#define HEIGHT 1024
#define FPS 600

void init_sim(const SimulationCore *core) {
  float total_mass = 4e6;
  float min_radius = 10;
  float max_radius = 80;
  float center_x = WIDTH / 2.0;
  float center_y = HEIGHT / 2.0;
  float vel_x = 0;
  float vel_y = 0;

  sim_init_normal(core->bodies, core->params, 0, core->params.body_count,
                  total_mass, 0.8, min_radius, max_radius, center_x, center_y,
                  vel_x, vel_y);

  // sim_init_uniform(core->bodies, 0, 200, 0, 200, 2);
}

int main(void) {
  SimulationParams params = {
      .body_count = 1e4, .G = 0.1, .eps = 0.7, .dt = 0.0001, .theta = 0.5};

  SimulationCore *core = sim_core_create(params, 1024);
  init_sim(core);

  Camera2D cam = {0};
  cam.zoom = 1;
  InitWindow(WIDTH, HEIGHT, "N-Body Sim");
  SetTargetFPS(FPS);

  while (!WindowShouldClose()) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / cam.zoom);
      cam.target = Vector2Add(cam.target, delta);
    }

    if (IsKeyPressed(KEY_R)) {
      init_sim(core);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);
      cam.offset = GetMousePosition();
      cam.target = mouseWorldPos;
      float scale = 0.2f * wheel;
      cam.zoom = Clamp(expf(logf(cam.zoom) + scale), 0.125f, 64.0f);
    }

    sim_core_step(core);

    BeginDrawing();
    ClearBackground((Color){15, 15, 25, 255});

    BeginMode2D(cam);
    for (int i = 0; i < core->bodies->count; i++) {
      Color particle_color = (Color){255, 233, 200, 100};
      DrawPixel(core->bodies->x[i], core->bodies->y[i], particle_color);
    }
    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
