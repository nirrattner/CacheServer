#include <arpa/inet.h>
#include <assert.h>
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
#include "configuration.h"
#include "connection_list.h"
#include "entry_hash_map.h"
#include "memory_queue.h"
#include "time_util.h"

#define CONNECTION_BACKLOG_LIMIT (32)
#define ACCEPT_PERIOD_MICROS (1000)

typedef struct {
  connection_t *current_connection;
  uint64_t last_accept_timestamp;
  int listen_file_descriptor;
} cache_server_context_t;

static cache_server_context_t context;

static uint8_t accept_connection(void);
static connection_t *get_next_connection(void);

uint8_t cache_server_open(void) {
  int result;
  struct sockaddr_in server_listen_address;

  uint64_t entry_capacity_bytes = configuration_get_int(CONFIGURATION_TYPE__ENTRY_CAPACITY_BYTES);
  const char *ip_address = configuration_get_string(CONFIGURATION_TYPE__IP_ADDRESS);
  uint16_t port = configuration_get_int(CONFIGURATION_TYPE__PORT);

  // TODO?
  // uint16_t active_connection_limit = configuration_get_int(CONFIGURATION_TYPE__CONNECTION_ACTIVE_LIMIT);

  result = connection_list_open()
      || entry_hash_map_open()
      || memory_queue_open(entry_capacity_bytes);
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
  server_listen_address.sin_addr.s_addr = inet_addr(ip_address);
  server_listen_address.sin_port = htons(port);

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

  context.current_connection = NULL;
  context.last_accept_timestamp = 0;

  printf("Starting server on %s:%d...\n", ip_address, port);

  return 0;
}

void cache_server_close(void) {
  connection_list_close();
  entry_hash_map_close();
  memory_queue_close();
}

// TODO: Use poll or epoll/kqueue?
void cache_server_proc(void) {
  if (context.current_connection == NULL
      || time_get_timestamp() - context.last_accept_timestamp > ACCEPT_PERIOD_MICROS) {
    accept_connection();
    return;
  }

  connection_t *next_connection = get_next_connection();
  connection_result_t result = connection_proc(context.current_connection);

  switch (result) {
    case CONNECTION_RESULT__SUCCESS:
      context.current_connection = connection_list_get_head();
      break;

    case CONNECTION_RESULT__NO_TRANSFER:
      context.current_connection = next_connection;
      break;

    case CONNECTION_RESULT__NEW_REQUEST:
      connection_list_remove(context.current_connection);
      connection_list_append(context.current_connection);
      context.current_connection = next_connection;
      break;

    case CONNECTION_RESULT__DISCONNECT:
      connection_list_remove(context.current_connection);
      connection_close(context.current_connection);
      connection_deinit(context.current_connection);

      if (next_connection == context.current_connection) {
        context.current_connection = NULL;
      } else {
        context.current_connection = next_connection;
      }
      break;

    //TODO
    case CONNECTION_RESULT__READ_BLOCKED:
    case CONNECTION_RESULT__WRITE_BLOCKED:
      break;

    default:
      assert(0);
      printf("Unsupported connection result %u\n", result);
  }
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
    // TODO: Crash server?
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

static connection_t *get_next_connection(void) {
  // TODO: Logic to reset to head periodically?
  // TODO: Logic to differentiate send vs receive states?
  connection_t *next = connection_get_next(context.current_connection);
  if (next != NULL) {
    return next;
  }

  next = connection_list_get_head();
  return next;
}

