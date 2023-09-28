#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NLINES 65536

void print_file(char **lines, size_t n) {
  for (size_t i=0; i<n; i++) printf("%zu:: %s", i, lines[i]);
}

int main(void) {
  FILE *fp = NULL;
  size_t num_lines_max = MAX_NLINES;
  char **filecontents = NULL;
  char *lineptr = NULL;
  size_t n = 0;
  size_t line_number = 0;
  size_t nchr = 0;

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

  print_file(filecontents, line_number);

  for (size_t i=0; i<line_number; i++) free(filecontents[i]);
  free(filecontents);
  if (fp) fclose(fp);
  if (lineptr) free(lineptr);

  return 0;
}
