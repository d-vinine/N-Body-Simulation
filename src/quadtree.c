#include "quadtree.h"
#include <stdlib.h>

QuadTreeNode *qtnode_create_root(float max_x, float max_y, float min_x,
                                 float min_y) {
  QuadTreeNode *ret = calloc(1, sizeof(QuadTreeNode));

  ret->x = (max_x + min_x) / 2;
  ret->y = (max_y + min_y) / 2;
  ret->mass = 0;

  ret->size =
      ((max_x - min_x) > (max_y - min_y)) ? (max_x - min_x) : (max_y - min_y);
  ret->s_x = ret->x;
  ret->s_y = ret->y;

  ret->is_leaf = 1;
  ret->is_empty = 1;

  ret->children = calloc(4, sizeof(QuadTreeNode *));

  return ret;
}

static int qtnode_get_child(QuadTreeNode *node, float x, float y) {
  if (y < node->s_y) {
    return (x < node->s_x) ? 0 : 1;
  } else {
    return (x < node->s_x) ? 3 : 2;
  }
}

static void qtnode_add_body(QuadTreeNode *node, float x, float y, float mass) {
  node->is_empty = 0;

  float node_mass = node->mass;
  float node_x = node->x;
  float node_y = node->y;

  node->x = (node_x * node_mass + x * mass) / (node_mass + mass);
  node->y = (node_y * node_mass + y * mass) / (node_mass + mass);
  node->mass += mass;
}

static void qtnode_subdivide(QuadTreeNode *node) {
  if (!node->is_leaf)
    return;
  node->is_leaf = 0;

  for (int i = 0; i < 4; i++) {
    node->children[i] = calloc(1, sizeof(QuadTreeNode));

    if (i == 0 || i == 3) {
      node->children[i]->x = node->s_x - 0.25 * node->size;
    } else {
      node->children[i]->x = node->s_x + 0.25 * node->size;
    }

    if (i == 0 || i == 1) {
      node->children[i]->y = node->s_y - 0.25 * node->size;
    } else {
      node->children[i]->y = node->s_y + 0.25 * node->size;
    }

    node->children[i]->mass = 0;

    node->children[i]->size = 0.5 * node->size;
    node->children[i]->s_x = node->children[i]->x;
    node->children[i]->s_y = node->children[i]->y;

    node->children[i]->is_empty = 1;
    node->children[i]->is_leaf = 1;

    node->children[i]->children = calloc(4, sizeof(QuadTreeNode *));
  }
}

void qtnode_insert(QuadTreeNode *node, float x, float y, float mass) {
  if (!node->is_leaf) {
    int child_idx = qtnode_get_child(node, x, y);
    qtnode_insert(node->children[child_idx], x, y, mass);

    qtnode_add_body(node, x, y, mass);
    return;
  }

  if (node->is_empty) {
    qtnode_add_body(node, x, y, mass);
    return;
  }

  if (x == node->x && y == node->y) {
    qtnode_add_body(node, x, y, mass);
    return;
  }

  float old_x = node->x;
  float old_y = node->y;
  float old_mass = node->mass;

  qtnode_subdivide(node);

  int old_child_idx = qtnode_get_child(node, old_x, old_y);
  qtnode_insert(node->children[old_child_idx], old_x, old_y, old_mass);

  int new_child_idx = qtnode_get_child(node, x, y);
  qtnode_insert(node->children[new_child_idx], x, y, mass);

  qtnode_add_body(node, x, y, mass);
}

void qtnode_destroy(QuadTreeNode *node) {
  for (int i = 0; i < 4; i++) {
    if (node->children[i])
      qtnode_destroy(node->children[i]);
  }
  free(node->children);
  free(node);
}
