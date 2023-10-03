#ifndef OUTPUT_DATA_H

#include "defns.h"

typedef struct OutputData {
  FILE *ptr;
  size_t N;
  double freq;
  double *x_mag;
  double *x_angle;
  double *y_mag;
  double *y_angle;
} OutputData;

#if THREADED
void *write_file(void*);
#else
void write_file(OutputData*);
#endif // THREADED

#endif // !OUTPUT_DATA_H

