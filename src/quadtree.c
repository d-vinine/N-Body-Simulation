#include "quadtree.h"
#include <stdlib.h>

// Helper function prototypes
int qt_is_empty(QuadTreeNode *node);

int qt_is_leaf(QuadTreeNode *node);

int qt_get_child(QuadTreeNode *node, float x, float y);

QuadTreeError qt_add_node(QuadTree *qt, QuadTreeNode node);

QuadTreeError qt_subdivide(QuadTree *qt, QuadTreeNode *node);


QuadTree *qt_create(int node_capacity) {
  QuadTree *ret = malloc(sizeof(QuadTree));
  if (!ret) {
    return NULL;
  }

  ret->nodes = malloc(node_capacity * sizeof(QuadTreeNode));
  if (!ret->nodes) {
    return NULL;
  }

  ret->node_count = 0;
  ret->node_capacity = node_capacity;

  return ret;
}

QuadTreeError qt_destroy(QuadTree *qt) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  free(qt->nodes);
  free(qt);

  return QT_SUCCESS;
}

QuadTreeError qt_set(QuadTree *qt, float max_x, float max_y, float min_x,
                     float min_y) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  // Calculating quad centre
  qt->nodes[0].size =
      ((max_x - min_x) > (max_y - min_y)) ? (max_x - min_x) : (max_y - min_y);
  qt->nodes[0].s_x = (max_x + min_x) / 2;
  qt->nodes[0].s_y = (max_y + min_y) / 2;

  // Setting centre of mass equal to quad centre (default empty node)
  qt->nodes[0].c_x = qt->nodes[0].c_x;
  qt->nodes[0].c_y = qt->nodes[0].c_y;
  qt->nodes[0].mass = 0;

  // Setting first child to 0 and adding root node
  qt->nodes[0].first_child = 0;
  qt->node_count = 1;

  return QT_SUCCESS;
}

QuadTreeError qt_insert(QuadTree *qt, float x, float y, float mass) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  QuadTreeNode *curr_node = &qt->nodes[0];

  // Case 1: Not a leaf
  while (!qt_is_leaf(curr_node)) {
    int child_idx = curr_node->first_child + qt_get_child(curr_node, x, y);
    curr_node = &qt->nodes[child_idx];
  }

  // Case 2: Empty leaf
  if (qt_is_empty(curr_node)) {
    curr_node->c_x = x;
    curr_node->c_y = y;
    curr_node->mass = mass;

    return QT_SUCCESS;
  }

  // Case 3: Non Empty leaf but overlapping bodies
  if (curr_node->c_x == x && curr_node->c_y == y) {
    curr_node->mass += mass;

    return QT_SUCCESS;
  }

  // Case 4: Non empty leaf, non overlapping bodies

  // Data of the existing body
  float old_x = curr_node->c_x;
  float old_y = curr_node->c_y;
  float old_mass = curr_node->mass;

  int old_child_idx =
      curr_node->first_child + qt_get_child(curr_node, old_x, old_y);
  int child_idx = curr_node->first_child + qt_get_child(curr_node, x, y);

  // Subdividing until the old and the new body end up in different nodes
  while (old_child_idx == child_idx) {
    QuadTreeError err = qt_subdivide(qt, curr_node);
    if (err != QT_SUCCESS) {
      return err;
    }

    curr_node->mass = 0;
    curr_node = &qt->nodes[old_child_idx];

    old_child_idx =
        curr_node->first_child + qt_get_child(curr_node, old_x, old_y);
    child_idx = curr_node->first_child + qt_get_child(curr_node, x, y);
  }

  // Adding old body
  qt->nodes[old_child_idx].c_x = old_x;
  qt->nodes[old_child_idx].c_y = old_y;
  qt->nodes[old_child_idx].mass = old_mass;

  // Adding new body
  qt->nodes[child_idx].c_x = x;
  qt->nodes[child_idx].c_y = y;
  qt->nodes[child_idx].mass = mass;

  return QT_SUCCESS;
}

// Helper functions
int qt_is_empty(QuadTreeNode *node) { return node->mass == 0; }

int qt_is_leaf(QuadTreeNode *node) { return node->first_child == 0; }

int qt_get_child(QuadTreeNode *node, float x, float y) {
  if (y < node->s_y) {
    return (x < node->s_x) ? 0 : 1; // Indexing is as arranged (0 : NW, etc.)
  } else {
    return (x < node->s_x) ? 3 : 2;
  }
}

QuadTreeError qt_add_node(QuadTree *qt, QuadTreeNode node) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  // Check to see if we will be over capacity
  if (qt->node_count + 1 > qt->node_capacity) {
    int new_capacity = 2 * qt->node_capacity;

    qt->nodes = realloc(qt->nodes, new_capacity * sizeof(QuadTreeNode));
    if (!qt->nodes) {
      return QT_ALLOC_FAILURE;
    }

    qt->node_capacity = new_capacity;
  }

  // Add node
  qt->nodes[qt->node_count] = node;
  qt->node_count++;

  return QT_SUCCESS;
}

QuadTreeError qt_subdivide(QuadTree *qt, QuadTreeNode *node) {
  if (!node) {
    return QT_INVALID_POINTER;
  }

  QuadTreeNode child;
  for (int i = 0; i < 4; i++) {

    // Setting the quad centre of the child
    if (i == 0 || i == 3) {
      child.s_x = node->s_x - 0.25 * node->size;
    } else {
      child.s_x = node->s_x + 0.25 * node->size;
    }

    if (i == 0 || i == 1) {
      child.s_y = node->s_y - 0.25 * node->size;
    } else {
      child.s_y = node->s_y + 0.25 * node->size;
    }

    child.size = 0.5 * node->size;

    // Setting the centre of mass equal to the quad centre
    child.c_x = child.s_x;
    child.c_y = child.s_y;
    child.mass = 0;

    // Setting first child = 0
    child.first_child = 0;

    QuadTreeError err = qt_add_node(qt, child);
    if (err != QT_SUCCESS) {
      return err;
    }
  }

  return QT_SUCCESS;
}
