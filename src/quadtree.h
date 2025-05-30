#ifndef QUADTREE_H
#define QUADTREE_H

typedef struct QuadTreeNode {
  float x, y; // Centre of mass coords
  float mass; // Total mass in node
  
  float size; // side length of square
  float s_x, s_y; // centre coords of square
  
  int is_leaf;
  int is_empty;

  struct QuadTreeNode **children; 
} QuadTreeNode;

#endif // QUADTREE_H

QuadTreeNode *qtnode_create_root(float max_x, float max_y, float min_x, float min_y);
void qtnode_destroy(QuadTreeNode *node);
void qtnode_insert(QuadTreeNode *node, float x, float y, float mass);
