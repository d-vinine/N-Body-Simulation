#include "simulation_core.h"
#include "raylib.h"
#include "raymath.h"

#define WIDTH 1024
#define HEIGHT 1024
#define FPS 600

void init_sim(const SimulationCore *core) {
  float total_mass = 1e6;
  float scale_length = 100;
  float centre_x = WIDTH / 2.0;
  float centre_y = HEIGHT / 2.0;
  float vel_x = 0;
  float vel_y = 0;
  float temp = 0.05;

  sim_init_galaxy(core->bodies, core->params, 0, core->bodies->count,
                  total_mass, scale_length, centre_x, centre_y, vel_x, vel_y,
                  temp);

  // sim_init_uniform(core->bodies, 0, 200, 0, 200, 2);
}

int main(void) {
  SimulationParams params = {
      .body_count = 5e4, .G = 0.1, .eps = 0.4, .dt = 0.002, .theta = 0.5};

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
      Color particle_color = (Color){255, 233, 200, 10};
      DrawPixel(core->bodies->x[i], core->bodies->y[i], particle_color);
    }
    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
