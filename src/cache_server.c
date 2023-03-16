#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cache_server.h"
#include "entry_hash_map.h"
#include "memory_queue.h"

#define CONNECTION_BACKLOG_LIMIT (32)

static uint8_t accept_connection(cache_server_t *cache_server);

cache_server_t *cache_server_init(
    uint64_t capacity_bytes,
    uint16_t connection_limit,
    const char *listen_ip_address,
    uint16_t listen_port) {
  int result;
  struct sockaddr_in server_listen_address;

  cache_server_t *cache_server = (cache_server_t *)malloc(sizeof(cache_server_t));
  if (cache_server == NULL) {
    printf("ERROR: Unable to allocate server\n");
    return NULL;
  }

  result = entry_hash_map_open()
      || memory_queue_open(capacity_bytes);
  if (result == 1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to allocate server fields\n");
    return NULL;
  }

  cache_server->listen_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (cache_server->listen_file_descriptor == -1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to allocate socket -- %s\n", strerror(errno));
    return NULL;
  }

  result = fcntl(cache_server->listen_file_descriptor, F_GETFL);
  if (result == -1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to get flags -- %s\n", strerror(errno));
    return NULL;
  }

  result = fcntl(cache_server->listen_file_descriptor, F_SETFL, result | O_NONBLOCK);
  if (result == -1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to get flags -- %s\n", strerror(errno));
    return NULL;
  }

  server_listen_address.sin_family = AF_INET;
  server_listen_address.sin_addr.s_addr = inet_addr(listen_ip_address);
  server_listen_address.sin_port = htons(listen_port);

  result = bind(
      cache_server->listen_file_descriptor, 
      (struct sockaddr*)&server_listen_address, 
      sizeof(struct sockaddr_in));
  if (result == -1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to bind socket -- %s\n", strerror(errno));
    return NULL;
  }

  result = listen(
      cache_server->listen_file_descriptor, 
      CONNECTION_BACKLOG_LIMIT);
  if (result == -1) {
    cache_server_deinit(cache_server);
    printf("ERROR: Unable to listen socket -- %s\n", strerror(errno));
    return NULL;
  }

  cache_server->active_connection_limit = connection_limit;
  cache_server->connection_count = 0;

  printf("Starting server on %s:%d...\n", listen_ip_address, listen_port);

  return cache_server;
}

void cache_server_deinit(cache_server_t *cache_server) {
  entry_hash_map_close();
  memory_queue_close();
  // TODO: Free connections
  free(cache_server);
}

uint8_t cache_server_proc(cache_server_t *cache_server) {
  if (cache_server->connection_count == 0) {
    // accept_connection(cache_server);
  }

  return 0;
}

static uint8_t accept_connection(cache_server_t *cache_server) {
  int client_file_descriptor = accept(cache_server->listen_file_descriptor, NULL, NULL);
  if (client_file_descriptor == -1) {
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      printf("ERROR: Failed to accept -- %s\n", strerror(errno));
      return 1;
    }
    return 0;
  }

  connection_t *connection = connection_init(client_file_descriptor);
  if (connection == NULL) {
    printf("ERROR: Unable to allocate connection\n");
    return 1;
  }

  printf("Connected!\n");

  return 0;
}

