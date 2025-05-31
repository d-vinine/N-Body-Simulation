#include "body_data.h"
#include <stdlib.h>
#include <string.h>

BodyData *body_data_create(int body_count) {
  BodyData *data = malloc(sizeof(BodyData));
  data->count = body_count;

  data->x = calloc(body_count, sizeof(float));
  data->y = calloc(body_count, sizeof(float));
  data->vx = calloc(body_count, sizeof(float));
  data->vy = calloc(body_count, sizeof(float));
  data->ax = calloc(body_count, sizeof(float));
  data->ay = calloc(body_count, sizeof(float));
  data->mass = calloc(body_count, sizeof(float));

  return data;
}

void body_data_destroy(BodyData *data) {
  if (data) {
    free(data->x);
    free(data->y);
    free(data->vx);
    free(data->vy);
    free(data->ax);
    free(data->ay);
    free(data->mass);
    free(data);
  }
}
