#include "simulation_renderer.h"
#include "simulation_core.h"
#include <raymath.h>

void sim_render_frame(const SimulationCore *core, Camera2D *cam,
                      void init_sim(const SimulationCore *core)) {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / cam->zoom);
    cam->target = Vector2Add(cam->target, delta);
  }

  if (IsKeyPressed(KEY_R)) {
    init_sim(core);
  }

  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *cam);
    cam->offset = GetMousePosition();
    cam->target = mouseWorldPos;
    float scale = 0.2f * wheel;
    cam->zoom = Clamp(expf(logf(cam->zoom) + scale), 0.125f, 64.0f);
  }

  BeginDrawing();
  ClearBackground((Color){15, 15, 25, 255});

  BeginMode2D(*cam);
  for (int i = 10000; i < core->bodies->count; i++) {
    Color particle_color = (Color){255, 233, 200, 10};
    DrawPixel(core->bodies->x[i], core->bodies->y[i], particle_color);
  }
  EndMode2D();

  EndDrawing();
}
