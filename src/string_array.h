#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct StringArray {
  char **contents;
  size_t size;
  size_t capacity;
} StringArray;

StringArray new_string_array(size_t);

bool string_array_is_full(StringArray*);

int alloc_string_array(StringArray*);

int realloc_string_array(StringArray*);

int add_string_to_array(StringArray*, char*);

void free_string_array(StringArray*);

#endif // !STRING_ARRAY_H
