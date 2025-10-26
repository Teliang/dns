#include "parse_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void convert_big_little_endian(uint16_t *num) {
  *num = (*num >> 8) | (*num << 8);
}

void convert_32(uint32_t *num) {
  *num = ((*num >> 24) & 0xFF) | ((*num >> 8) & 0xFF00) |
         ((*num & 0xFF00) << 8) | ((*num & 0xFF) << 24);
}

void print_header(dns_header_t *header) {
  printf("transaction_ID: %d\n", header->transaction_ID);
  printf("flag: %d\n", header->flag);
  printf("number_of_questions: %d\n", header->number_of_questions);
  printf("number_of_answers: %d\n", header->number_of_answers);
  printf("number_of_authority_RRs: %d\n", header->number_of_authority_RRs);
  printf("number_of_additional_RRs: %d\n", header->number_of_additional_RRs);
}

void print_record(resource_record_t *record) {
  printf("record doman count: %d\n", record->name.level_count);
  printf("record domain: ");
  for (int i = 0; i < 8; i++) {
    printf("%s", record->name.domains[i]);
  }
  printf("\n");
  printf("type: %d\n", record->type);
  printf("class: %d\n", record->clazz);
}

void free_record(resource_record_t *record) {
  resource_name_t name = record->name;
  for (int i = 0; i < name.level_count; i++) {
    free(name.domains[i]);
  }
}

void parse_header(char *buf, dns_header_t *header) {
  memcpy(header, buf, sizeof(dns_header_t));
  // TODO
  convert_big_little_endian(&header->number_of_questions);
  print_header(header);
}

void parse_record(char *buf, resource_record_t *record) {
  /*
   * read domain
   * len|domain|len|domain
   */
  char length;
  int read_postion = sizeof(dns_header_t);
  while (1) {
    memcpy(&length, buf + read_postion, sizeof(length));
    read_postion++;
    if (!length) {
      break;
    }
    char *domain = malloc(length + 1);

    memcpy(domain, buf + read_postion, length);
    read_postion += length;
    domain[length] = 0x00;
    record->name.domains[record->name.level_count++] = domain;
  };

  memcpy(&record->type, buf + read_postion, sizeof(record->type));
  read_postion += sizeof(record->type);
  // TODO
  convert_big_little_endian(&record->type);

  memcpy(&record->clazz, buf + read_postion, sizeof(record->clazz));
  // TODO
  convert_big_little_endian(&record->clazz);
  read_postion += sizeof(record->clazz);

  print_record(record);
  // free_record(record);
}

void handle_request(char *buf, int buf_len, dns_request_t *request) {
  printf("server received %d bytes: ", buf_len);
  for (int i = 0; i < buf_len; ++i) {
    printf("%x", buf[i]);
  }
  printf("\n");

  parse_header(buf, request->header);
  parse_record(buf, request->record);
}
