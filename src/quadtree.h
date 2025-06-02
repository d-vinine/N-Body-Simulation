#ifndef QUADTREE_H
#define QUADTREE_H

typedef enum QuadTreeError {
  QT_SUCCESS,
  QT_ALLOC_FAILURE,
  QT_INVALID_POINTER,
} QuadTreeError;

typedef struct QuadTreeNode {
  float c_x, c_y; // Centre of mass coords
  float mass;     // Total mass in node

  float size;     // side length of square
  float s_x, s_y; // centre coords of square

  int first_child; // index of first child (0 means no child)
} QuadTreeNode;

typedef struct QuadTree {
  QuadTreeNode *nodes;
  int node_count;
  int node_capacity;
} QuadTree;

QuadTree *qt_create(int node_capacity);
QuadTreeError qt_destroy(QuadTree *qt);
QuadTreeError qt_set(QuadTree *qt, float max_x, float max_y, float min_x,
                     float min_y);
QuadTreeError qt_insert(QuadTree *qt, float x, float y, float mass);

#endif // QUADTREE_H
