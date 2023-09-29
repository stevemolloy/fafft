#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <complex.h>
#include <fftw3.h>

#include "string_array.h"

#define MAX_NLINES 131072
#define timespec_diff_macro(a, b, result)             \
  do {                                                \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;     \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;  \
    if ((result)->tv_nsec < 0) {                      \
      --(result)->tv_sec;                             \
      (result)->tv_nsec += 1000000000;                \
    }                                                 \
  } while (0)
// I got the following from http://twitch.tv/tsoding
#define return_defer(value) do { exe_result = (value); goto defer; } while (0)

int main(void) {
  int exe_result = 0;
  FILE *fp = NULL;
  StringArray file_contents = new_string_array(MAX_NLINES);
  char *lineptr = NULL;
  size_t n = 0; // Needed for the upcoming getline call
  fftw_complex *x_pos = NULL;
  fftw_complex *y_pos = NULL;
  fftw_complex *x_fft = NULL;
  fftw_complex *y_fft = NULL;
  fftw_plan px = NULL;
  fftw_plan py = NULL;

  // Reading the file
  if (!(fp = fopen ("fa_data_001.dat", "r"))) {
    fprintf (stderr, "error: file open failed");
    return_defer(1);
  }

  if (alloc_string_array(&file_contents) < 0) {
    return_defer(1);
  }

  while (getline(&lineptr, &n, fp) > 0) {
    if (add_string_to_array(&file_contents, lineptr) < 0) {
      return_defer(1);
    }
  }
  // At this point, the file has been read and stored in file_contents

  // Preparing for the FFT
  size_t N = file_contents.size - 2;
  x_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  y_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  x_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  y_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

  struct timespec new_ts = {0}, old_ts = {0}, result = {0};
  unsigned long long time_diff_sum = 0;

  for (size_t i=2; i<file_contents.size; i++) {
    int year, month, day, hour, minute, second, nanoseconds, x_int, y_int;
    if (sscanf(
          file_contents.contents[i], 
          "%d-%d-%d_%d:%d:%d.%d, [%d, %d]", 
          &year, &month, &day, &hour, &minute, &second, &nanoseconds, &x_int, &y_int
        ) != 9) {
        fprintf(stderr, "Invalid input format\n");
        return_defer(1);
    }

    x_pos[i-2] = x_int + 0.0*I;
    y_pos[i-2] = y_int + 0.0*I;
    struct tm tm_data = {
        .tm_year = year - 1900, // Adjust for year offset
        .tm_mon = month - 1,    // Adjust for month offset
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min = minute,
        .tm_sec = second,
    };

    // Convert struct tm to a timestamp in seconds since the epoch
    time_t timestamp = mktime(&tm_data);
    if (timestamp == -1) {
        perror("mktime");
        return_defer(1);
    }

    // Create a timespec structure
    if (i>2) old_ts = new_ts;
    new_ts.tv_sec = timestamp + nanoseconds / 1000000000; // Convert nanoseconds to seconds
    new_ts.tv_nsec = nanoseconds % 1000000000;

    if (i>2) {
       timespec_diff_macro(&new_ts, &old_ts, &result);
       time_diff_sum += result.tv_sec*1000000000 + result.tv_nsec;
    }
  }
  double T = ((double)time_diff_sum / ((double)N-1)) / 1e9;
  double frequency = 1/T;

  // Making the plans
  px = fftw_plan_dft_1d(N, x_pos, x_fft, FFTW_FORWARD, FFTW_ESTIMATE);
  py = fftw_plan_dft_1d(N, y_pos, y_fft, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(px);
  fftw_execute(py);

  for (size_t i=0; i<N; i++) {
    printf("%lf, %f, %f\n", i*frequency, cabs(x_fft[i]), carg(x_fft[i]));
  }

  defer:
    if (fp) fclose(fp);
    if (lineptr) free(lineptr);
    if (file_contents.contents) free_string_array(&file_contents);
    if (x_pos) fftw_free(x_pos);
    if (y_pos) fftw_free(y_pos);
    if (x_fft) fftw_free(x_fft);
    if (y_fft) fftw_free(y_fft);
    if (px) fftw_destroy_plan(px);
    if (py) fftw_destroy_plan(py);
    fftw_cleanup();

    return exe_result;
}
