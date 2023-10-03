#ifndef OUTPUT_DATA_H

typedef struct OutputData {
  FILE *ptr;
  size_t N;
  double freq;
  double *x_mag;
  double *x_angle;
  double *y_mag;
  double *y_angle;
} OutputData;

void write_file(OutputData*);

#endif // !OUTPUT_DATA_H

