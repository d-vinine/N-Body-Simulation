#include "helper_funcs.h"
#include "math.h"
#include "stdlib.h"

float gaussian_random() {
  static int has_spare = 0;
  static float spare;

  if (has_spare) {
    has_spare = 0;
    return spare;
  }

  float u, v, s;
  do {
    u = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    v = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    s = u * u + v * v;
  } while (s >= 1.0f || s == 0.0f);

  s = sqrtf(-2.0f * logf(s) / s);
  spare = v * s;
  has_spare = 1;

  return u * s;
}
