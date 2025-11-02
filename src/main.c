#include "cache.h"
#include "client.h"
#include "common.h"
#include "parse_request.h"
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

int main(int argc, char **argv) {
  int sockfd;                    /* socket */
  unsigned int clientlen;        /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp;         /* client host info */
  char *msg;                     /* message buf */
  char *hostaddrp;               /* dotted decimal host addr string */
  int optval;                    /* flag value for setsockopt */
  int msg_len;                   /* message byte size */

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
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(SERVER_PORT);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  clientlen = sizeof(clientaddr);
  int request_count = 0;

  while (1) {
    printf("\n");
    printf("===== request count is: %d =====\n", request_count);
    request_count++;

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    msg = malloc(BUFSIZE);
    msg_len = recvfrom(sockfd, msg, BUFSIZE, 0, (struct sockaddr *)&clientaddr,
                       &clientlen);

    if (msg_len < 0)
      error("ERROR in recvfrom");

    dns_header_t header = {};
    resource_record_t *record = calloc(1, sizeof(resource_record_t));
    if (!record)
      error("alloc record");

    dns_request_t request = {&header, record};
    handle_request(msg, msg_len, &request);

    char *cache_key = build_key(record);

    free_record(record);

    cache_result_t result = get_from_cache(cache_key);

    char *response = NULL;
    int response_size = 0;

    if (result.status == HAS_CACHE) {
      printf("HAS_CACHE\n");
      response_size = result.response.len;
      response = result.response.response;
    } else {
      printf("NO_CACHE\n");
      response_size = send_to_server(msg, msg_len, &response);
      add_to_cache(cache_key, response, response_size);
    }

    free(cache_key);

    msg_len = sendto(sockfd, response, response_size, 0,
                     (struct sockaddr *)&clientaddr, clientlen);
    if (msg_len < 0)
      error("ERROR in sendto");

    free(msg);
  }
}
