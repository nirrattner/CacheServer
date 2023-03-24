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

#include "blocked_connections.h"
#include "cache_server.h"
#include "configuration.h"
#include "connection_list.h"
#include "entry_hash_map.h"
#include "memory_queue.h"
#include "time_util.h"

#define CONNECTION_BACKLOG_LIMIT (32)
#define POLL_TIMEOUT_MS (1000)

typedef struct {
  uint64_t last_accept_timestamp;
} cache_server_context_t;

static cache_server_context_t context;

static uint8_t accept_connection(int listen_file_descriptor);
static void handle_lock_release_event(connection_t *current_connection);

uint8_t cache_server_open(void) {
  int result;
  struct sockaddr_in server_listen_address;
  int listen_file_descriptor;

  uint64_t entry_capacity_bytes = configuration_get_int(CONFIGURATION_TYPE__ENTRY_CAPACITY_BYTES);
  const char *ip_address = configuration_get_string(CONFIGURATION_TYPE__IP_ADDRESS);
  uint16_t port = configuration_get_int(CONFIGURATION_TYPE__PORT);
  uint32_t active_connection_limit = configuration_get_int(CONFIGURATION_TYPE__CONNECTION_ACTIVE_LIMIT);

  result = connection_list_open(active_connection_limit)
      || entry_hash_map_open()
      || memory_queue_open(entry_capacity_bytes);
  if (result == 1) {
    cache_server_close();
    printf("ERROR: Unable to allocate server fields\n");
    return 1;
  }

  listen_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_file_descriptor == -1) {
    cache_server_close();
    printf("ERROR: Unable to allocate socket -- %s\n", strerror(errno));
    return 1;
  }

  result = fcntl(listen_file_descriptor, F_GETFL);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to get flags -- %s\n", strerror(errno));
    return 1;
  }

  result = fcntl(listen_file_descriptor, F_SETFL, result | O_NONBLOCK);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to set flags -- %s\n", strerror(errno));
    return 1;
  }

  server_listen_address.sin_family = AF_INET;
  server_listen_address.sin_addr.s_addr = inet_addr(ip_address);
  server_listen_address.sin_port = htons(port);

  result = bind(
      listen_file_descriptor,
      (struct sockaddr*)&server_listen_address,
      sizeof(struct sockaddr_in));
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to bind socket -- %s\n", strerror(errno));
    return 1;
  }

  result = listen(
      listen_file_descriptor,
      CONNECTION_BACKLOG_LIMIT);
  if (result == -1) {
    cache_server_close();
    printf("ERROR: Unable to listen socket -- %s\n", strerror(errno));
    return 1;
  }

  context.last_accept_timestamp = 0;
  connection_list_set_listen_file_descriptor(listen_file_descriptor);

  printf("Starting server on %s:%d...\n", ip_address, port);
  return 0;
}

void cache_server_close(void) {
  connection_list_close();
  entry_hash_map_close();
  memory_queue_close();
}


// TODO: Use epoll/kqueue?
void cache_server_proc(void) {
  connection_t *connection;
  connection_result_t connection_result;
  struct pollfd *pollfd;
  uint32_t connection_list_size = connection_list_get_size();

  int poll_result = poll(
      connection_list_get_pollfds(),
      connection_list_size + LISTEN_FILE_DESCRIPTOR_OFFSET,
      POLL_TIMEOUT_MS);
  // printf("Polling[%u] %d\n", connection_list_size, result);

  // TODO: Handle failures
  if (poll_result == -1) {
    printf("ERROR: Failed to poll -- %s\n", strerror(errno));
    return;
  }

  // TODO: Check server capacity
  pollfd = connection_list_get_listen_pollfd();
  if (poll_result &&
      pollfd->revents != 0) {
    poll_result--;
    accept_connection(pollfd->fd);
  }

  for (uint32_t index = 0; poll_result && index < connection_list_size; index++) {
    pollfd = connection_list_get_connection_pollfd(index);
    if (pollfd->revents == 0) {
      continue;
    }
    poll_result--;

    connection = connection_list_get(index);

    assert(connection_get_file_descriptor(connection) == pollfd->fd);
    connection_result = connection_proc(connection);
    handle_lock_release_event(connection);

    switch (connection_result) {
      case CONNECTION_RESULT__SUCCESS:
      case CONNECTION_RESULT__NEW_REQUEST:
        pollfd->events = connection_get_pollfd_events(connection);
        break;

      case CONNECTION_RESULT__PARTIAL_TRANSFER:
        break;

      case CONNECTION_RESULT__NO_TRANSFER:
        printf("WARN: No transfer\n");
        break;

      case CONNECTION_RESULT__DISCONNECT:
        // TODO: Remove
        // printf("Disconnected[%d]\n", pollfd->fd);

        connection_list_remove(index);
        connection_close(connection);
        connection_deinit(connection);
        index--;
        connection_list_size--;
        break;

      case CONNECTION_RESULT__WRITE_BLOCKED:
        connection_list_remove(index);
        blocked_connections_add(connection);
        index--;
        connection_list_size--;
        break;

      default:
        printf("Unsupported connection result %u\n", connection_result);
        assert(0);
    }
  }
}

static uint8_t accept_connection(int listen_file_descriptor) {
  context.last_accept_timestamp = time_get_timestamp();

  int client_file_descriptor = accept(listen_file_descriptor, NULL, NULL);
  if (client_file_descriptor == -1) {
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      printf("ERROR: Failed to accept -- %s\n", strerror(errno));
      return 1;
    }
    return 0;
  }

  // TODO: connection_list should allocate
  connection_t *connection = connection_init(client_file_descriptor);
  if (connection == NULL) {
    // TODO: Crash server?
    printf("ERROR: Unable to allocate connection\n");
    return 1;
  }

  connection_list_add(connection);

  // TODO: Remove
  // printf("Connected[%d]\n", client_file_descriptor);

  return 0;
}

static void handle_lock_release_event(connection_t *current_connection) {
  connection_t *connection;
  connection_result_t result;
  entry_header_lock_event_t lock_release_event = connection_get_lock_release_event(current_connection);

  switch (lock_release_event) {
    case ENTRY_HEADER_LOCK_EVENT__NONE:
      return;

    case ENTRY_HEADER_LOCK_EVENT__WRITES_UNBLOCKED:
      connection = blocked_connections_pop(connection_get_entry_header(current_connection));
      while (connection) {
        result = connection_unblock_write(connection);
        if (result != CONNECTION_RESULT__SUCCESS) {
          blocked_connections_add(connection);
          return;
        }
        connection_list_add(connection);
        connection = blocked_connections_pop(connection_get_entry_header(current_connection));
      }
      break;

    default:
      printf("Unsupported lock release event %u\n", lock_release_event);
      assert(0);
      break;
  }
}

