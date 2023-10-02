#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <complex.h>
#include <fftw3.h>

#include "string_array.h"

#define MAX_NLINES 131072
#define FILE_EXTENSION ".fft"
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
//#define return_defer(value) do { exe_result = (value); goto defer; } while (0)

typedef struct FileWriter {
  FILE *ptr;
  size_t N;
  double freq;
  double *x_mag;
  double *x_angle;
  double *y_mag;
  double *y_angle;
} FileWriter;

void write_file(FileWriter *fw) {
  for (size_t i=0; i<fw->N; i++) {
    fprintf(
        fw->ptr,
        "%lf, %f, %f, %f, %f\n",
        (double)i*fw->freq,
        fw->x_mag[i], fw->x_angle[i],
        fw->y_mag[i], fw->y_angle[i]
      );
  }
}

time_t time_to_epoch ( const struct tm *ltm, int utcdiff ) {
   const int mon_days [] =
      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   long tyears, tdays, leaps, utc_hrs;
   int i;

   tyears = ltm->tm_year - 70 ; // tm->tm_year is from 1900.
   leaps = (tyears + 2) / 4; // no of next two lines until year 2100.
   //i = (ltm->tm_year – 100) / 100;
   //leaps -= ( (i/4)*3 + i%4 );
   tdays = 0;
   for (i=0; i < ltm->tm_mon; i++) tdays += mon_days[i];

   tdays += ltm->tm_mday-1; // days of month passed.
   tdays = tdays + (tyears * 365) + leaps;

   utc_hrs = ltm->tm_hour + utcdiff; // for your time zone.
   return (tdays * 86400) + (utc_hrs * 3600) + (ltm->tm_min * 60) + ltm->tm_sec;
}

int main(int argc, char* argv[]) {
  int exe_result = 0;

  const char* exe_name = argv[0];
  if (argc == 1) {
    fprintf(stderr, "Please provide a file containing the FA data to be analysed.\n");
    fprintf(stderr, "%s filename\n", exe_name);
    return 1;
  }

  FileWriter *fw = malloc(sizeof(FileWriter) * (unsigned long)(argc - 1));

  for (int file_ctr=1; file_ctr<argc; file_ctr++) {
    char* output_filename = NULL;
    FILE *input_file_ptr = NULL;
    FILE *output_file_ptr = NULL;
    StringArray file_contents;
    char *lineptr = NULL;
    size_t n = 0; // Needed for the upcoming getline call
    fftw_complex *x_pos = NULL;
    fftw_complex *y_pos = NULL;
    fftw_complex *x_fft = NULL;
    fftw_complex *y_fft = NULL;
    fftw_plan px = NULL;
    fftw_plan py = NULL;

    printf("Working on %s\n", argv[file_ctr]);

    // Generate the output filename by concatenating ".fft" to the input filename
    char* input_filename = argv[file_ctr];
    output_filename = (char*) malloc((strlen(input_filename) + strlen(FILE_EXTENSION) + 1) * sizeof(char));
    strcpy(output_filename, input_filename);
    strcat(output_filename, ".fft");

    // Reading the file
    if (!(input_file_ptr = fopen (input_filename, "r"))) {
      fprintf (stderr, "error: file open failed");
      return 1;
    }
    
    file_contents = new_string_array(MAX_NLINES);
    if (alloc_string_array(&file_contents) < 0) {
      return 1;
    }

    while (getline(&lineptr, &n, input_file_ptr) > 0) {
      if (add_string_to_array(&file_contents, lineptr) < 0) {
        return 1;
      }
    }
    // At this point, the file has been read and stored in file_contents

    // Preparing for the FFT
    size_t N = file_contents.size - 2;
    x_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (unsigned long)N);
    y_pos = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (unsigned long)N);
    x_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (unsigned long)N);
    y_fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (unsigned long)N);

    struct timespec new_ts = {0}, old_ts = {0}, result = {0};
    long time_diff_sum = 0;

    for (size_t i=2; i<file_contents.size; i++) {
      int year, month, day, hour, minute, second, nanoseconds, x_int, y_int;
      // TODO: Can this sscanf be replaced with fscanf to speed things up?
      if (sscanf(
            file_contents.contents[i], 
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
      // time_t timestamp = mktime(&tm_data);
      time_t timestamp = time_to_epoch(&tm_data, 2);
      if (timestamp == -1) {
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

    double T = (double)time_diff_sum / 1e9;
    double frequency = 1/T;

    // Performing the two FFTs
    px = fftw_plan_dft_1d((int)N, x_pos, x_fft, FFTW_FORWARD, FFTW_ESTIMATE);
    py = fftw_plan_dft_1d((int)N, y_pos, y_fft, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(px);
    fftw_execute(py);

    if (!(output_file_ptr = fopen (output_filename, "w"))) {
      fprintf (stderr, "error: file open failed");
      return 1;
    }

    double *x_mag = malloc(sizeof(double) * N);
    double *y_mag = malloc(sizeof(double) * N);
    double *x_angle = malloc(sizeof(double) * N);
    double *y_angle = malloc(sizeof(double) * N);
    fprintf(output_file_ptr, "Freq/Hz, x Mag, x Angle, y Mag, y Angle\n");
    for (size_t i=0; i<N; i++) {
      x_mag[i] = cabs(x_fft[i]);
      y_mag[i] = cabs(y_fft[i]);
      x_angle[i] = carg(x_fft[i]);
      y_angle[i] = carg(y_fft[i]);
    }

    fw[file_ctr - 1] = (FileWriter) {
      .ptr = output_file_ptr,
      .N = N,
      .freq = frequency,
      .x_mag = x_mag,
      .x_angle = x_angle,
      .y_mag = y_mag,
      .y_angle = y_angle,
    };

    write_file(&fw[file_ctr - 1]);

    if (output_filename) free(output_filename);
    if (input_file_ptr) fclose(input_file_ptr);
    if (output_file_ptr) fclose(output_file_ptr);
    if (lineptr) free(lineptr);
    if (file_contents.contents) free_string_array(&file_contents);
    if (x_pos) fftw_free(x_pos);
    if (y_pos) fftw_free(y_pos);
    if (x_fft) fftw_free(x_fft);
    if (y_fft) fftw_free(y_fft);
    if (px) fftw_destroy_plan(px);
    if (py) fftw_destroy_plan(py);
    if (x_mag) free(x_mag);
    if (y_mag) free(y_mag);
    if (x_angle) free(x_angle);
    if (y_angle) free(y_angle);
  }
  free(fw);
  fftw_cleanup();

  return exe_result;
}
