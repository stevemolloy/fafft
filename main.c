#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <complex.h>
#include <fftw3.h>

#define MAX_NLINES 65536
#define timespec_diff_macro(a, b, result)             \
  do {                                                \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;     \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;  \
    if ((result)->tv_nsec < 0) {                      \
      --(result)->tv_sec;                             \
      (result)->tv_nsec += 1000000000;                \
    }                                                 \
  } while (0)

int main(void) {
  FILE *fp = NULL;
  size_t num_lines_max = MAX_NLINES;
  char **filecontents = NULL;
  char *lineptr = NULL;
  size_t n = 0;
  size_t line_number = 0;
  size_t nchr = 0;

  // Reading the file
  if (!(fp = fopen ("fa_data_001.dat", "r"))) {
      fprintf (stderr, "error: file open failed");
      return 1;
  }

  if (!(filecontents = calloc(num_lines_max, sizeof(filecontents)))) {
    printf("Memory allocation failed.\n");
    fclose(fp);
    return 1;
  }

  while ((nchr = getline(&lineptr, &n, fp)) != -1) {
    filecontents[line_number++] = strdup(lineptr);

    if (line_number == num_lines_max) {
      num_lines_max *= 2;
      filecontents = realloc(filecontents, num_lines_max * sizeof(filecontents));
      if (filecontents == NULL) {
        printf("Memory allocation failed.\n");
        for (size_t i=0; i<line_number; i++) free(filecontents[i]);
        free(filecontents);
        if (fp) fclose(fp);
        if (lineptr) free(lineptr);
        return 1;
      }
    }
  }
  // At this point, the file has been read and stored in filecontents

  // Preparing for the FFT
  size_t N = line_number - 2;
  fftw_complex *x_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *y_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *x_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *y_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

  struct timespec new_ts = {0}, old_ts = {0}, result = {0};
  unsigned long long time_diff_sum = 0;

  for (size_t i=2; i<line_number; i++) {
    int year, month, day, hour, minute, second, nanoseconds, x_int, y_int;
    if (sscanf(
          filecontents[i], 
          "%d-%d-%d_%d:%d:%d.%d, [%d, %d]", 
          &year, &month, &day, &hour, &minute, &second, &nanoseconds, &x_int, &y_int
        ) != 9) {
        fprintf(stderr, "Invalid input format\n");
        return 1;
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
        return 1;
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
  fftw_plan px = fftw_plan_dft_1d(N, x_pos, x_fft, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan py = fftw_plan_dft_1d(N, y_pos, y_fft, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(px);
  fftw_execute(py);

  for (size_t i=0; i<N; i++) {
    printf("%lf, %f\n", i*frequency, cabs(x_fft[i]));
  }

  fftw_destroy_plan(px);
  fftw_destroy_plan(py);
  fftw_free(x_pos);
  fftw_free(y_pos);
  fftw_free(x_fft);
  fftw_free(y_fft);
  fftw_cleanup();

  for (size_t i=0; i<line_number; i++) free(filecontents[i]);
  free(filecontents);
  if (fp) fclose(fp);
  if (lineptr) free(lineptr);

  return 0;
}
