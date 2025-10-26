#ifndef PARSE_REQUEST_H
#define PARSE_REQUEST_H
#include <stdint.h>

typedef struct dns_header {
  uint16_t transaction_ID;
  uint16_t flag;
  uint16_t number_of_questions;
  uint16_t number_of_answers;
  uint16_t number_of_authority_RRs;
  uint16_t number_of_additional_RRs;
} dns_header_t;

// const uint32_t DNS_HEADER_POSITION = sizeof(dns_header_t);

typedef struct resource_name {
  char *domains[8];
  int level_count;
} resource_name_t;

typedef struct resource_record {
  resource_name_t name;
  uint16_t type;
  uint16_t clazz;
} resource_record_t;

typedef struct dns_request {
  dns_header_t *header;
  resource_record_t *record;
} dns_request_t;

void handle_request(char *buf, int buf_len, dns_request_t *request);
void free_record(resource_record_t *record);

#endif
