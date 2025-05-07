#define _POSIX_C_SOURCE 199309L
#include "simulation.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Macro definitions
#define WIDTH 800
#define HEIGHT 800
#define FPS 1000
#define BODY_NUM 10000

// Function prototypes
float randfloat(int min, int max);
void init_sim(Simulation *sim);

int main(void) {
  srand(time(0));

  // Create and initialize simulation
  Simulation sim = create_simulation(BODY_NUM, 10, 1e4, 1, 1e-4);
  init_sim(&sim);

  // Initialize window and camera
  InitWindow(WIDTH, HEIGHT, "N-Body Sim");
  SetTargetFPS(FPS);
  Camera2D cam = {0};
  cam.zoom = 1;

  // Main loop
  while (!WindowShouldClose()) {

    // Input handling
    // Reset simulation
    if (IsKeyPressed(KEY_R)) {
      init_sim(&sim);
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

    // Simulate and draw
    for (int i = 0; i < 5; i++) {
      step_simulation_parallel(&sim, 8);
    }

    BeginDrawing();

    BeginMode2D(cam);
    ClearBackground((Color){52, 52, 52, 255});
    for (int i = 0; i < sim.body_num; i++) {
      DrawPixel(sim.x[i], sim.y[i], WHITE);
    }
    EndMode2D();

    EndDrawing();

    printf("FPS: %d\n", GetFPS());
  }

  CloseWindow();
  return 0;
}

// Helper Functions
float randfloat(int min, int max) {
  return (float)rand() / RAND_MAX * (max - min) + min;
}

void init_sim(Simulation *sim) {
  for (int i = 0; i < sim->body_num; i++) {
    sim->x[i] = randfloat(0, 2000);
    sim->y[i] = randfloat(0, 2000);
    sim->v_x[i] = randfloat(-100, 100);
    sim->v_y[i] = randfloat(-100, 100);
  }
}

