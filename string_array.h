#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct StringArray {
  char **contents;
  size_t size;
  size_t capacity;
} StringArray;

bool string_array_is_full(StringArray*);

int alloc_string_array(StringArray*);

int realloc_string_array(StringArray*);

#endif // !
