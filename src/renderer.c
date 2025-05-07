#include "sharedmemory.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 800
#define FPS 60

int main(void) {
  InitWindow(WIDTH, HEIGHT, "N-Body Sim");
  SetTargetFPS(FPS);
  Camera2D cam = {0};
  cam.zoom = 1;

  SharedMemory *shm = attach_shared_memory("/simulation");
  float *shm_x = (float *)((char *)shm + shm->x_offset);
  float *shm_y = (float *)((char *)shm + shm->y_offset);

  // Main loop
  while (!WindowShouldClose()) {

    // Input handling
    // Reset simulation
    if (IsKeyPressed(KEY_R)) {
      if (sem_post(&shm->signal[RESET]) != 0)
        return 1;
    }

    // Move around using mouse
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / cam.zoom);
      cam.target = Vector2Add(cam.target, delta);
    }

    // Zoom using scroll wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);

      cam.offset = GetMousePosition();

      cam.target = mouseWorldPos;
      float scale = 0.2f * wheel;
      cam.zoom = expf(logf(cam.zoom) + scale);
    }

    BeginDrawing();

    BeginMode2D(cam);
    ClearBackground((Color){52, 52, 52, 255});
    for (int i = 0; i < shm->body_num; i++) {
      DrawPixel(shm_x[i], shm_y[i], (Color){255, 255, 255, 80});
    }
    EndMode2D();

    EndDrawing();
  }

  CloseWindow();
  sem_post(&shm->signal[CLOSE]);
  if (destroy_shared_memory(shm, "/simulation"))
    return 1;
  return 0;
}
