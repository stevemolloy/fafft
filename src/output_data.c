#include <stdio.h>

#include "output_data.h"

// OutputData* new_outputdata(void) {
//
// }

#if THREADED
void write_file(void *ptr) {
  OutputData *output;
  output = (OutputData *) ptr;
#else
void write_file(OutputData *output) {
#endif // THREADED
  fprintf(output->ptr, "Freq/Hz, x Mag, x Angle, y Mag, y Angle\n");
  for (size_t i=0; i<output->N; i++) {
    fprintf(output->ptr, "%lf, ", (double)i*output->freq);
    fprintf(output->ptr, "%f, ", output->x_mag[i]);
    fprintf(output->ptr, "%f, ", output->x_angle[i]);
    fprintf(output->ptr, "%f, ", output->y_mag[i]);
    fprintf(output->ptr, "%f\n", output->y_angle[i]);
  }
}

