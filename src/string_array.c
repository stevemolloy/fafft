#include <stdio.h>
#include <string.h>

#include "string_array.h"

StringArray new_string_array(size_t initial_cap) {
  StringArray result = {
    .contents = NULL,
    .size = 0,
    .capacity = initial_cap,
  };
  return result;
}

bool string_array_is_full(StringArray *sa) {
  return sa->size == sa->capacity;
}

int add_string_to_array(StringArray *sa, char *input) {
  sa->contents[sa->size++] = strdup(input);

  if (string_array_is_full(sa)) {
    if (realloc_string_array(sa) < 0) {
      return -1;
    }
  }
  return 0;
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

void free_string_array(StringArray *sa) {
  for (size_t i=0; i<sa->size; i++) free(sa->contents[i]);
  free(sa->contents);
}

