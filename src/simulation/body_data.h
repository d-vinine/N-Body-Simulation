#ifndef BODY_DATA_H
#define BODY_DATA_H

typedef struct BodyData {
  float *x;    // x positions
  float *y;    // y positions
  float *vx;   // x velocities
  float *vy;   // y velocities
  float *ax;   // x accelerations
  float *ay;   // y accelerations
  float *mass; // masses
  int count;   // number of bodies
} BodyData;

// Creates a new BodyData structure
BodyData *body_data_create(int body_count);

// Destroys and frees a BodyData structure
void body_data_destroy(BodyData *data);

#endif // BODY_DATA_H
