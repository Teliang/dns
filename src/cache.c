#include "cache.h"
#include "parse_request.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

record_t *records = NULL;

char *build_key(resource_record_t *record) {
  char *key = calloc(50, sizeof(char));

  resource_name_t name = record->name;
  for (int i = 0; i < name.level_count; i++) {
    strcat(key, name.domains[i]);
    if (i != name.level_count - 1) {
      strcat(key, ".");
    }
  }

  char char_array[10]; // Allocate enough space for the number and null
                       // terminator
                       //
  // Convert to decimal string
  sprintf(char_array, "%d", record->type);

  strcat(key, "_");
  strcat(key, char_array);

  // Convert to decimal string
  sprintf(char_array, "%d", record->clazz);

  strcat(key, "_");
  strcat(key, char_array);

  printf("key: %s\n", key);

  return key;
}

cache_result_t get_from_cache(char *key) {
  cache_result_t result = {NO_CACHE, NULL};
  record_t l, *p;
  l.key = key;
  HASH_FIND_STR(records, key, p);
  if (p) {
    printf("found %p\n", p);
    printf("found, key: %s\n", p->key);
    result.status = HAS_CACHE;
    result.response = p->value;
  }
  return result;
}

void add_to_cache(char *key, char *response, int len) {
  record_t *r = (record_t *)malloc(sizeof *r);
  cache_response_t value = {response, len};
  memset(r, 0, sizeof *r);
  r->key = key;
  r->value = value;
  HASH_ADD_KEYPTR(hh, records, r->key, strlen(r->key), r);
}
