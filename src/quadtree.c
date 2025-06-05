#include "quadtree.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function prototypes
int qt_get_child(QuadTreeNode *node, float x, float y);

QuadTreeError qt_add_node(QuadTree *qt, QuadTreeNode node);

QuadTreeError qt_add_parent(QuadTree *qt, int parent_idx);

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

  int parent_capacity = node_capacity * 0.75;
  ret->parents = malloc(parent_capacity * sizeof(int));
  if (!ret->parents) {
    return NULL;
  }

  ret->parent_count = 0;
  ret->parent_capacity = parent_capacity;

  return ret;
}

QuadTreeError qt_destroy(QuadTree *qt) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  free(qt->nodes);
  free(qt->parents);
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
  qt->nodes[0].c_x = qt->nodes[0].s_x;
  qt->nodes[0].c_y = qt->nodes[0].s_y;
  qt->nodes[0].mass = 0;

  // Setting first child to 0 and adding root node
  qt->nodes[0].first_child = 0;
  qt->node_count = 1;

  // Setting the next (non existant for root so 0)
  qt->nodes[0].next = 0;

  // Setting parent count to 0
  qt->parent_count = 0;

  return QT_SUCCESS;
}

QuadTreeError qt_insert(QuadTree *qt, float x, float y, float mass) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  QuadTreeNode *curr_node = &qt->nodes[0];

  // Case 1: Not a leaf
  int curr_idx = 0;
  while (!qt_is_leaf(curr_node)) {
    curr_idx = curr_node->first_child + qt_get_child(curr_node, x, y);
    curr_node = &qt->nodes[curr_idx];
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

  int old_child_idx, child_idx;

  do {
    curr_node = &qt->nodes[curr_idx];

    QuadTreeError err;
    // Add current node to parent list as it will now be subdivided
    err = qt_add_parent(qt, curr_idx);
    if (err != QT_SUCCESS) {
      return err;
    }
    err = qt_subdivide(qt, curr_node);
    if (err != QT_SUCCESS) {
      return err;
    }
    curr_node = &qt->nodes[curr_idx];

    old_child_idx =
        curr_node->first_child + qt_get_child(curr_node, old_x, old_y);
    child_idx = curr_node->first_child + qt_get_child(curr_node, x, y);

    curr_idx = old_child_idx;
  } while (old_child_idx == child_idx);

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

QuadTreeError qt_propagate(QuadTree *qt) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  QuadTreeNode *parent;
  for (int i = qt->parent_count - 1; i >= 0; i--) {
    parent = &qt->nodes[qt->parents[i]];

    float x1 = qt->nodes[parent->first_child].c_x;
    float y1 = qt->nodes[parent->first_child].c_y;
    float m1 = qt->nodes[parent->first_child].mass;

    float x2 = qt->nodes[parent->first_child + 1].c_x;
    float y2 = qt->nodes[parent->first_child + 1].c_y;
    float m2 = qt->nodes[parent->first_child + 1].mass;

    float x3 = qt->nodes[parent->first_child + 2].c_x;
    float y3 = qt->nodes[parent->first_child + 2].c_y;
    float m3 = qt->nodes[parent->first_child + 2].mass;

    float x4 = qt->nodes[parent->first_child + 3].c_x;
    float y4 = qt->nodes[parent->first_child + 3].c_y;
    float m4 = qt->nodes[parent->first_child + 3].mass;

    float total_mass = m1 + m2 + m3 + m4;

    parent->c_x = (m1 * x1 + m2 * x2 + m3 * x3 + m4 * x4) / total_mass;
    parent->c_y = (m1 * y1 + m2 * y2 + m3 * y3 + m4 * y4) / total_mass;
    parent->mass = total_mass;
  }

  return QT_SUCCESS;
}

QuadTreeError qt_acc(QuadTree *qt, float x, float y, float theta, float eps,
                     float G, float *ax, float *ay) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  *ax = 0;
  *ay = 0;

  float theta2 = theta * theta;
  float eps2 = eps * eps;

  int curr_idx = 0;
  while (1) {
    QuadTreeNode *curr_node = &qt->nodes[curr_idx];
    float size2 = curr_node->size * curr_node->size;

    float dx = curr_node->c_x - x;
    float dy = curr_node->c_y - y;
    float dist2 = dx * dx + dy * dy;
    if (dist2 < eps2)
      dist2 = eps2;

    if (qt_is_leaf(curr_node) || size2 < dist2 * theta2) {

      float inv_dist = 1.0f / sqrtf(dist2);
      float inv_dist3 = inv_dist * inv_dist * inv_dist;
      float a = G * curr_node->mass * inv_dist3;

      *ax += a * dx;
      *ay += a * dy;

      if (curr_node->next == 0) {
        break;
      }
      curr_idx = curr_node->next;
    } else {
      curr_idx = curr_node->first_child;
    }
  }

  return QT_SUCCESS;
}

// Helper functions
int qt_is_empty(QuadTreeNode *node) { return node->mass == 0; }

int qt_is_leaf(QuadTreeNode *node) { return node->first_child == 0; }

int qt_get_child(QuadTreeNode *node, float x, float y) {
  if (y <= node->s_y) {
    return (x <= node->s_x) ? 0 : 1; // Indexing is as arranged (0 : NW, etc.)
  } else {
    return (x <= node->s_x) ? 3 : 2;
  }
}

QuadTreeError qt_add_node(QuadTree *qt, QuadTreeNode node) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  // Check to see if we will be over capacity
  while (qt->node_count + 1 > qt->node_capacity) {
    qt->node_capacity *= 2;
  }

  qt->nodes = realloc(qt->nodes, qt->node_capacity * sizeof(QuadTreeNode));
  if (!qt->nodes) {
    return QT_ALLOC_FAILURE;
  }

  // Add node
  qt->nodes[qt->node_count] = node;
  qt->node_count++;

  return QT_SUCCESS;
}

QuadTreeError qt_add_parent(QuadTree *qt, int parent_idx) {
  if (!qt) {
    return QT_INVALID_POINTER;
  }

  // Check to see if we will be over capacity
  while (qt->parent_count + 1 > qt->parent_capacity) {
    qt->parent_capacity *= 2;
  }

  qt->parents = realloc(qt->parents, qt->parent_capacity * sizeof(int));
  if (!qt->parents) {
    return QT_ALLOC_FAILURE;
  }

  qt->parents[qt->parent_count] = parent_idx;
  qt->parent_count++;

  return QT_SUCCESS;
}

QuadTreeError qt_subdivide(QuadTree *qt, QuadTreeNode *node) {
  if (!node) {
    return QT_INVALID_POINTER;
  }

  node->first_child = qt->node_count;

  QuadTreeNode child;
  for (int i = 0; i < 4; i++) {

    // Setting the quad centre of the child
    if (i == 0) {
      child.s_x = node->s_x - 0.25 * node->size;
      child.s_y = node->s_y - 0.25 * node->size;
      child.next = node->first_child + 1;
    } else if (i == 1) {
      child.s_x = node->s_x + 0.25 * node->size;
      child.s_y = node->s_y - 0.25 * node->size;
      child.next = node->first_child + 2;
    } else if (i == 2) {
      child.s_x = node->s_x + 0.25 * node->size;
      child.s_y = node->s_y + 0.25 * node->size;
      child.next = node->first_child + 3;
    } else {
      child.s_x = node->s_x - 0.25 * node->size;
      child.s_y = node->s_y + 0.25 * node->size;
      child.next = node->next;
    }

    child.size = 0.5 * node->size;

    // Setting the centre of mass equal to the quad centre
    child.c_x = child.s_x;
    child.c_y = child.s_y;
    child.mass = 0;

    // Setting the first child = 0
    child.first_child = 0;

    QuadTreeError err = qt_add_node(qt, child);
    if (err != QT_SUCCESS) {
      return err;
    }
  }

  return QT_SUCCESS;
}
