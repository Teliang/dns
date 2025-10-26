#include "cache.h"
#include "parse_request.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdio.h>

record_t *records = NULL;

cache_result_t get_from_cache(resource_record_t *record) {
  cache_result_t result = {NO_CACHE, NULL};
  record_t l, *p;
  l.key = *record;
  HASH_FIND(hh, records, &l.key, sizeof(resource_record_t), p);
  if (p) {
    printf("found %p\n", p);
    result.status = HAS_CACHE;
    result.response = p->value;
  }
  return result;
}

void add_to_cache(resource_record_t *record, char *response, int len) {
  record_t *r;
  r = (record_t *)malloc(sizeof *r);
  cache_response_t value = {response, len};
  memset(r, 0, sizeof *r);
  r->key = *record;
  r->value = value;
  HASH_ADD(hh, records, key, sizeof(resource_record_t), r);
}

// TODO

unsigned key_hash(Key *key) {
  unsigned hash = 100;
  hash += key->type;
  hash += key->clazz;
  return hash;
}

bool key_equal(Key *a, Key *b) {
  if (a->type != b->type) {
    return false;
  }
  if (a->clazz != b->clazz) {
    return false;
  }
  return true;
}

#undef HASH_KEYCMP
#undef HASH_FUNCTION
#define HASH_FUNCTION(s, len, hashv) (hashv) = key_hash((Key *)s)
#define HASH_KEYCMP(a, b, len) (!key_equal((Key *)a, (Key *)b))
