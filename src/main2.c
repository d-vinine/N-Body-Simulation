#include "simulation.h"
#include <raylib.h>
#include <raymath.h>

#define BODY_NUM 20000
#define G 0.1
#define EPS 0.1
#define DT 0.003
#define THETA 0.5

#define THREAD_NUM 10

#define WIDTH 800
#define HEIGHT 800
#define FPS 6000

void setup_antennae_collision(Simulation *sim) {
  // Galaxy 1 - Larger spiral
  int galaxy1_particles = 12000;
  float disk_mass1 = 1.5e3;
  float central_mass1 = 4.5e2;
  float scale_length1 = 40.0;
  float cx1 = 150, cy1 = 150;
  float v_cx1 = 0.3, v_cy1 = -0.8; // Approaching trajectory

  // Galaxy 2 - Smaller spiral
  int galaxy2_particles = 8000;
  float disk_mass2 = 1e3;
  float central_mass2 = 3e2;
  float scale_length2 = 30.0;
  float cx2 = 175, cy2 = 100;
  float v_cx2 = -0.5, v_cy2 = 1.2; // Counter-approaching

  sim_init_galaxy_disk_with_center(sim, 0, galaxy1_particles, disk_mass1,
                                   central_mass1, scale_length1, cx1, cy1,
                                   v_cx1, v_cy1);
  sim_init_galaxy_disk_with_center(sim, galaxy1_particles, galaxy2_particles,
                                   disk_mass2, central_mass2, scale_length2,
                                   cx2, cy2, v_cx2, v_cy2);
}

void setup_uniform(Simulation *sim) {
  // Define the simulation bounds
  float min_x = 0, max_x = 400;
  float min_y = 0, max_y = 400;

  // Uniform particle mass
  float particle_mass = 1.0;

  // Maximum random velocity magnitude
  float max_velocity = 2.0;

  for (int i = 0; i < sim->body_num; i++) {
    // Uniform position distribution
    sim->x[i] = min_x + (max_x - min_x) * ((float)rand() / RAND_MAX);
    sim->y[i] = min_y + (max_y - min_y) * ((float)rand() / RAND_MAX);

    // Random velocity direction and magnitude
    float angle = 2.0 * M_PI * ((float)rand() / RAND_MAX);
    float speed = max_velocity * ((float)rand() / RAND_MAX);

    sim->v_x[i] = speed * cos(angle);
    sim->v_y[i] = speed * sin(angle);

    // Uniform mass
    sim->mass[i] = particle_mass;

    // Initialize accelerations to zero
    sim->a_x[i] = 0.0;
    sim->a_y[i] = 0.0;
  }
}
int main(void) {
  srand(time(0));

  Simulation *sim = sim_create(BODY_NUM, G, EPS, DT, THETA, THREAD_NUM);

  setup_antennae_collision(sim);
  sim_init_leapfrog(sim);

  InitWindow(WIDTH, HEIGHT, "N-Body Sim - Fixed Central Hole");

  Camera2D camera = {0};
  camera.zoom = 0.8f;

  SetTargetFPS(FPS);

  while (!WindowShouldClose()) {

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / camera.zoom);
      camera.target = Vector2Add(camera.target, delta);
    }

    if (IsKeyPressed(KEY_R)) {
      setup_antennae_collision(sim);
      sim_init_leapfrog(sim);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.offset = GetMousePosition();
      camera.target = mouseWorldPos;
      float scale = 0.2f * wheel;
      camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
    }

    sim_step_leapfrog(sim);

    BeginDrawing();
    ClearBackground((Color){15, 15, 25, 255});

    BeginMode2D(camera);

    for (int i = 0; i < BODY_NUM; i++) {
      Color particle_color = (Color){255, 233, 81, 10};
      DrawPixel(sim->x[i], sim->y[i], particle_color);
    }
    EndMode2D();

    // Debug info
    DrawText("Press R to reset", 10, HEIGHT - 25, 20, WHITE);

    EndDrawing();
  }

  sim_destroy(sim);
  CloseWindow();
  return 0;
}
