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
  int next;        // index of next node (see qt_acc)
} QuadTreeNode;

typedef struct QuadTree {
  QuadTreeNode *nodes;
  int node_count;
  int node_capacity;

  int *parents; // index of non-leaf nodes (top to bottom ordered)
  int parent_count;
  int parent_capacity;
} QuadTree;

QuadTree *qt_create(int node_capacity);
QuadTreeError qt_destroy(QuadTree *qt);
QuadTreeError qt_set(QuadTree *qt, float max_x, float max_y, float min_x,
                     float min_y);
QuadTreeError qt_insert(QuadTree *qt, float x, float y, float mass);
QuadTreeError qt_propagate(QuadTree *qt);
QuadTreeError qt_acc(QuadTree *qt, float x, float y, float theta, float eps,
                     float G, float *ax, float *ay);

int qt_is_empty(QuadTreeNode *node);
int qt_is_leaf(QuadTreeNode *node);

#endif // QUADTREE_H
