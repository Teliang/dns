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

#define RESPONSE_MAX 1024
#define SERVER_PORT 53      // The port to send data to
#define SERVER_IP "8.8.8.8" // The server's IP address

int send_to_server(char *buf, int length, char **rec) {
  int sockfd, n;

  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  // Set up the server address struct
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("Invalid address or address not supported");
    exit(1);
  }

  /* send the message to the server */
  n = sendto(sockfd, buf, length, 0, (struct sockaddr *)&server_addr,
             sizeof(server_addr));
  if (n < 0)
    error("ERROR in sendto");

  if (!*rec)
    *rec = malloc(RESPONSE_MAX);

  struct sockaddr_in dest_addr;
  socklen_t serverlen = sizeof(dest_addr);
  /* print the server's reply */
  n = recvfrom(sockfd, *rec, RESPONSE_MAX, 0, (struct sockaddr *)&dest_addr,
               &serverlen);
  if (n < 0)
    error("ERROR in recvfrom");

  printf("recv from server: ");
  for (int i = 0; i < n; i++) {
    printf("%x", (*rec)[i]);
  }
  printf("\n");

  close(sockfd);

  return n;
}
