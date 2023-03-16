#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "cache_server.h"
#include "connection_list.h"
#include "entry_hash_map.h"
#include "memory_queue.h"
#include "time_util.h"

#define CONNECTION_BACKLOG_LIMIT (32)
#define ACCEPT_PERIOD_MICROS (500)

typedef struct {
  connection_t *current_connection;
  uint64_t last_accept_timestamp;
  int listen_file_descriptor;
  uint16_t active_connection_limit;
} cache_server_context_t;

static cache_server_context_t context;

static uint8_t accept_connection(void);

uint8_t cache_server_open(
    uint64_t capacity_bytes,
    uint16_t connection_limit,
    const char *listen_ip_address,
    uint16_t listen_port) {
  int result;
  struct sockaddr_in server_listen_address;

  result = connection_list_open()
      || entry_hash_map_open()
      || memory_queue_open(capacity_bytes);
  if (result == 1) {
    cache_server_close();
    printf("ERROR: Unable to allocate server fields\n");
    return 1;
  }

  context.listen_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (context.listen_file_descriptor == -1) {
    cache_server_close();
    printf("ERROR: Unable to allocate socket -- %s\n", strerror(errno));
    return 1;
  }

  result = fcntl(context.listen_file_descriptor, F_GETFL);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to get flags -- %s\n", strerror(errno));
    return 1;
  }

  result = fcntl(context.listen_file_descriptor, F_SETFL, result | O_NONBLOCK);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to set flags -- %s\n", strerror(errno));
    return 1;
  }

  server_listen_address.sin_family = AF_INET;
  server_listen_address.sin_addr.s_addr = inet_addr(listen_ip_address);
  server_listen_address.sin_port = htons(listen_port);

  result = bind(
      context.listen_file_descriptor, 
      (struct sockaddr*)&server_listen_address, 
      sizeof(struct sockaddr_in));
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to bind socket -- %s\n", strerror(errno));
    return 1;
  }

  result = listen(
      context.listen_file_descriptor, 
      CONNECTION_BACKLOG_LIMIT);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to listen socket -- %s\n", strerror(errno));
    return 1;
  }

  context.active_connection_limit = connection_limit;
  context.current_connection = NULL;
  context.last_accept_timestamp = 0;

  printf("Starting server on %s:%d...\n", listen_ip_address, listen_port);

  return 0;
}

void cache_server_close(void) {
  connection_list_close();
  entry_hash_map_close();
  memory_queue_close();
}

uint8_t cache_server_proc(void) {
  if (context.current_connection == NULL
      || time_get_timestamp() - context.last_accept_timestamp > ACCEPT_PERIOD_MICROS) {
    return accept_connection();
  }

  // TODO: Change current connection as needed
  return connection_proc(context.current_connection);
}

static uint8_t accept_connection(void) {
  context.last_accept_timestamp = time_get_timestamp();

  int client_file_descriptor = accept(context.listen_file_descriptor, NULL, NULL);
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

  connection_list_append(connection);
  if (context.current_connection == NULL) {
    context.current_connection = connection;
  }

  // TODO: Remove
  printf("Connected!\n");

  return 0;
}

