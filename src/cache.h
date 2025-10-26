#include "parse_request.h"
#include "uthash.h"

typedef struct cache_response {
  char *response;
  int len;
} cache_response_t;

typedef resource_record_t Key;

typedef struct {
  Key key;
  cache_response_t value;
  UT_hash_handle hh;
} record_t;

enum cache_status { HAS_CACHE, NO_CACHE };

typedef struct cache_result {
  enum cache_status status;
  cache_response_t response;
} cache_result_t;

cache_result_t get_from_cache(resource_record_t *record);

void add_to_cache(resource_record_t *record, char *response, int len);
