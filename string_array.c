#include <stdio.h>

#include "string_array.h"

bool string_array_is_full(StringArray *sa) {
  return sa->size == sa->capacity;
}

int alloc_string_array(StringArray *sa) {
  sa->contents = calloc(sa->capacity, sizeof(sa->contents));
  if (sa->contents == NULL) {
    printf("Memory allocation failed\n");
    return -1;
  }
  return 0;
}

int realloc_string_array(StringArray *sa) {
  sa->capacity *= 2;
  sa->contents = realloc(sa->contents, sa->capacity * sizeof(sa->contents));
  if (sa->contents == NULL) {
    printf("Memory allocation failed\n");
    return -1;
  }
  return 0;
}

