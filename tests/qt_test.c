#include "../src/quadtree.h"
#include <raylib.h>
#include <stdio.h>

void qt_draw(QuadTree *qt) {
  int padding = 3;

  for (int i = 0; i < qt->node_count; i++) {
    float x = qt->nodes[i].s_x - qt->nodes[i].size * 0.5 + padding;
    float y = qt->nodes[i].s_y - qt->nodes[i].size * 0.5 + padding;
    float size = qt->nodes[i].size - padding;

    DrawRectangleLines(x, y, size, size, WHITE);

    if (qt->nodes[i].mass > 0) {
      DrawCircle(qt->nodes[i].c_x, qt->nodes[i].c_y, 2, RED);
    }
  }
}

int main(void) {
  QuadTree *qt = qt_create(1024);
  qt_set(qt, 700, 700, 100, 100);

  InitWindow(800, 800, "QuadTree Test");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      float x = GetMouseX();
      float y = GetMouseY();

      qt_insert(qt, x, y, 1);
    }

    BeginDrawing();

    ClearBackground((Color){23, 23, 33, 255});
    qt_draw(qt);

    EndDrawing();
  }

  CloseWindow();
  qt_destroy(qt);
  return 0;
}
