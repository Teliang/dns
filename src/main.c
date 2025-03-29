#include "client.h"
#include "common.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 1024
#define SERVER_PORT 53

typedef struct dns_header {
  uint16_t transaction_ID;
  uint16_t flag;
  uint16_t number_of_questions;
  uint16_t number_of_answers;
  uint16_t number_of_authority_RRs;
  uint16_t number_of_additional_RRs;
} dns_header_t;

typedef struct resource_name {
  char *domains[8];
  int level_count;
} resource_name_t;

typedef struct resource_record {
  resource_name_t name;
  uint16_t type;
  uint16_t class;
} resource_record_t;

void convert_big_little_endian(uint16_t *num) {
  *num = (*num >> 8) | (*num << 8);
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
  printf("type: %d\n", record->type);
  printf("class: %d\n", record->class);
}

void free_record(resource_record_t *record) {
  resource_name_t name = record->name;
  for (int i = 0; i < name.level_count; i++) {
    free(name.domains[i]);
  }
}

void parse_header(dns_header_t *header) {}

void parse_record(resource_record_t *record) {}

void handle_request(char *buf, int n) {
  printf("server received %d bytes\n", n);
  for (int i = 0; i < n; ++i) {
    printf("%x", buf[i]);
  }
  printf("\n");

  dns_header_t header;
  memcpy(&header, buf, sizeof(header));
  // TODO
  convert_big_little_endian(&header.number_of_questions);

  char length;
  resource_record_t record = {};
  int read_postion = sizeof(header);
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
    record.name.domains[record.name.level_count++] = domain;
  };

  memcpy(&record.type, buf + read_postion, sizeof(record.type));
  read_postion += sizeof(record.type);
  // TODO
  convert_big_little_endian(&record.type);

  memcpy(&record.class, buf + read_postion, sizeof(record.class));
  // TODO
  convert_big_little_endian(&record.class);

  print_header(&header);
  print_record(&record);

  free_record(&record);
}

int main(int argc, char **argv) {
  int sockfd;                    /* socket */
  unsigned int clientlen;        /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp;         /* client host info */
  char *buf;                     /* message buf */
  char *hostaddrp;               /* dotted decimal host addr string */
  int optval;                    /* flag value for setsockopt */
  int n;                         /* message byte size */

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /*
   * socket: create the parent socket
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
             sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(SERVER_PORT);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  clientlen = sizeof(clientaddr);
  while (1) {
    /*
     * recvfrom: receive a UDP datagram from a client
     */
    buf = malloc(BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr,
                 &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /*
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");

    handle_request(buf, n);

    char *response = NULL;
    int response_size = send_to_server(buf, n, &response);

    n = sendto(sockfd, response, response_size, 0,
               (struct sockaddr *)&clientaddr, clientlen);

    free(response);
    free(buf);
    if (n < 0)
      error("ERROR in sendto");
  }
}
