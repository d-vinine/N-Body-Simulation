#include "quadtree.h"
#include "simulation.h"
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>

#define BODY_NUM 10000
#define MASS 10
#define G 1e5
#define EPS 2
#define DT 1e-4
#define THREAD_NUM 10
#define WIDTH 800
#define HEIGHT 800
#define FPS 6090

Color ColorFromHSV(float hue, float sat, float val) {
  float c = val * sat;
  float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2) - 1));
  float m = val - c;

  float r = 0, g = 0, b = 0;
  if (hue < 60) {
    r = c;
    g = x;
    b = 0;
  } else if (hue < 120) {
    r = x;
    g = c;
    b = 0;
  } else if (hue < 180) {
    r = 0;
    g = c;
    b = x;
  } else if (hue < 240) {
    r = 0;
    g = x;
    b = c;
  } else if (hue < 300) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  return (Color){(unsigned char)((r + m) * 255), (unsigned char)((g + m) * 255),
                 (unsigned char)((b + m) * 255), 255};
}

void qtnode_draw(QuadTreeNode *node, int depth) {
  if (!node)
    return;

  // Cycle hue every few levels
  float hue =
      fmodf(depth * 47.0f, 360.0f); // 47° per level to get good separation
  Color nodeColor = ColorFromHSV(hue, 0.9f, 0.9f);
  Color outlineColor = ColorFromHSV(hue, 0.5f, 0.7f); // dimmer for borders

  float shrink = 2.5f;
  float half = node->size / 2.0f;

  float draw_x = node->s_x - half + shrink;
  float draw_y = node->s_y - half + shrink;
  float draw_size = node->size - 2 * shrink;

  DrawRectangleLines((int)draw_x, (int)draw_y, (int)draw_size, (int)draw_size,
                     outlineColor);

  if (!node->is_empty) {
    DrawCircle((int)node->x, (int)node->y, 2.5f, nodeColor);
  }

  if (!node->is_leaf) {
    for (int i = 0; i < 4; i++) {
      qtnode_draw(node->children[i], depth + 1);
    }
  }
}

int main(void) {
  srand(time(0));

  float x[BODY_NUM], y[BODY_NUM];
  int num = 0;

  QuadTreeNode *root = qtnode_create_root(WIDTH, HEIGHT, 0, 0);

  InitWindow(WIDTH, HEIGHT, "N-Body Sim");
  SetTargetFPS(FPS);

  while (!WindowShouldClose()) {

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      x[num] = GetMouseX();
      y[num] = GetMouseY();

      qtnode_insert(root, x[num], y[num], MASS);

      num++;
    }

    BeginDrawing();
    ClearBackground((Color){52, 52, 52, 255});
    for (int i = 0; i < BODY_NUM; i++) {
      DrawCircle(x[i], y[i], 3, (Color){255, 255, 255, 80});
    }

    qtnode_draw(root, 0);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
