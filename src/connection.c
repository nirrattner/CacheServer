#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "configuration.h"
#include "connection.h"
#include "entry_hash_map.h"
#include "memory_queue.h"
#include "time_util.h"

typedef enum {
  CONNECTION_STATE__RECEIVING_HEADER = 0,
  CONNECTION_STATE__RECEIVING_ARGUMENTS,
  CONNECTION_STATE__AWAITING_WRITE_UNBLOCK,
  CONNECTION_STATE__RECEIVING_BODY,
  CONNECTION_STATE__SENDING_HEADER,
  CONNECTION_STATE__SENDING_BODY,
} connection_state_t;

typedef enum {
  CONNECTION_FLAG__NONE = 0,
  CONNECTION_FLAG__KEEP_ALIVE = (1 << 0),
  CONNECTION_FLAG__ALLOCATED_BUFFER = (1 << 1),
  CONNECTION_FLAG__LOCK_ACQUIRED = (1 << 2),
} connection_flag_t;

#pragma pack(1)
typedef struct {
  response_header_t header;
  value_arguments_t argument;
} aggregate_response_header_t;
#pragma pack()

typedef union {
  request_header_t request;
  aggregate_response_header_t response;
} aggregate_header_t;

typedef union {
  key_arguments_t key;
  key_value_arguments_t key_value;
} request_arguments_t;

// TODO: Create buffer layer for recevies to reduce system calls
struct connection {
  struct connection *next;
  void *buffer;
  entry_header_t *entry_header;
  uint32_t remaining_bytes;
  uint32_t buffer_index;
  int file_descriptor;
  connection_state_t state;
  aggregate_header_t header;
  request_arguments_t arguments;
  entry_header_lock_event_t lock_release_event;
  connection_flag_t flags;
};

static void event_received_header(connection_t *connection);
static connection_result_t event_received_arguments(connection_t *connection);
static connection_result_t event_received_body(connection_t *connection);
static connection_result_t event_sent_header(connection_t *connection);
static connection_result_t event_sent_body(connection_t *connection);

static void init_request(connection_t *connection);
static connection_result_t connection_transfer(connection_t *connection);
static connection_result_t attempt_memory_write(connection_t *connection);
static void send_header(connection_t *connection, response_type_t type);
static connection_result_t finish_request(connection_t *connection);

connection_t *connection_init(int file_descriptor) {
  int result;
  connection_t *connection = (connection_t *)malloc(sizeof(connection_t));
  if (connection == NULL) {
    return NULL;
  }

  result = fcntl(file_descriptor, F_GETFL);
  if (result == -1) {
    printf("ERROR: Unable to get client flags -- %s\n", strerror(errno));
    return NULL;
  }

  result = fcntl(file_descriptor, F_SETFL, result | O_NONBLOCK);
  if (result == -1) {
    printf("ERROR: Unable to set client flags -- %s\n", strerror(errno));
    return NULL;
  }

  connection->next = NULL;
  connection->file_descriptor = file_descriptor;
  init_request(connection);
  return connection;
}

void connection_deinit(connection_t *connection) {
  free(connection);
}

// TODO: Provide pollfd revents?
connection_result_t connection_proc(connection_t *connection) {
  connection_result_t result = connection_transfer(connection);
  if (result != CONNECTION_RESULT__SUCCESS) {
    return result;
  }

  switch (connection->state) {
    case CONNECTION_STATE__RECEIVING_HEADER:
      event_received_header(connection);
      break;

    case CONNECTION_STATE__RECEIVING_ARGUMENTS:
      return event_received_arguments(connection);

    case CONNECTION_STATE__RECEIVING_BODY:
      return event_received_body(connection);

    case CONNECTION_STATE__SENDING_HEADER:
      return event_sent_header(connection);

    case CONNECTION_STATE__SENDING_BODY:
      return event_sent_body(connection);

    default:
      printf("Unsupported state %u\n", connection->state);
      assert(0);
  }

  return CONNECTION_RESULT__SUCCESS;
}

connection_result_t connection_unblock_write(connection_t *connection) {
  assert(connection->state == CONNECTION_STATE__AWAITING_WRITE_UNBLOCK);
  return attempt_memory_write(connection);
}

void connection_close(connection_t *connection) {
  if (connection->file_descriptor > 0) {
    close(connection->file_descriptor);
  }
}

connection_t *connection_get_next(connection_t *connection) {
  return connection->next;
}

void connection_set_next(connection_t *connection, connection_t *next) {
  connection->next = next;
}

int connection_get_file_descriptor(connection_t *connection) {
  return connection->file_descriptor;
}

short connection_get_pollfd_events(connection_t *connection) {
  switch (connection->state) {
    case CONNECTION_STATE__RECEIVING_HEADER:
    case CONNECTION_STATE__RECEIVING_ARGUMENTS:
    case CONNECTION_STATE__RECEIVING_BODY:
      return POLLIN;

    case CONNECTION_STATE__SENDING_HEADER:
    case CONNECTION_STATE__SENDING_BODY:
      return POLLOUT;

    default:
      printf("Unsupported pollfd event state %u\n", connection->state);
      assert(0);
      break;
  }
}

entry_header_t *connection_get_entry_header(connection_t *connection) {
  return connection->entry_header;
}

entry_header_lock_event_t connection_get_lock_release_event(connection_t *connection) {
  return connection->lock_release_event;
}

static void event_received_header(connection_t *connection) {
  if (connection->header.request.version != CACHE_PROTOCOL_VERSION) {
      send_header(connection, RESPONSE_TYPE__UNSUPPORTED_VERSION);
      return;
  }

  if (connection->header.request.flags & REQUEST_FLAG__KEEP_ALIVE) {
    connection->flags |= CONNECTION_FLAG__KEEP_ALIVE;
  }

  switch (connection->header.request.type) {
    case REQUEST_TYPE__PING:
      send_header(connection, RESPONSE_TYPE__PONG);
      break;

    case REQUEST_TYPE__GET:
    case REQUEST_TYPE__DELETE:
      connection->buffer = &connection->arguments;
      connection->remaining_bytes = sizeof(key_arguments_t);
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__RECEIVING_ARGUMENTS;
      break;

    case REQUEST_TYPE__PUT:
      connection->buffer = &connection->arguments;
      connection->remaining_bytes = sizeof(key_value_arguments_t);
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__RECEIVING_ARGUMENTS;
      break;

    default:
      send_header(connection, RESPONSE_TYPE__UNKNOWN_REQUEST);
      break;
   }
}

static connection_result_t event_received_arguments(connection_t *connection) {
  // TODO: Validate arguments
  memory_queue_result_t result;

  switch (connection->header.request.type) {
    case REQUEST_TYPE__GET:
    case REQUEST_TYPE__DELETE:
      connection->entry_header = (entry_header_t *)malloc(sizeof(entry_header_t)
          + connection->arguments.key.key_size);

      if (connection->entry_header == NULL) {
        send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
        return CONNECTION_RESULT__SUCCESS;
      }

      connection->entry_header->key_size = connection->arguments.key.key_size;
      connection->flags |= CONNECTION_FLAG__ALLOCATED_BUFFER;
      connection->buffer = (void *)connection->entry_header + sizeof(entry_header_t);
      connection->remaining_bytes = connection->arguments.key.key_size;
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__RECEIVING_BODY;
      break;

    case REQUEST_TYPE__PUT:
      return attempt_memory_write(connection);

    default:
      printf("Unsupported type for receive arguments %u\n", connection->header.request.type);
      assert(0);
      break;
   }

  return CONNECTION_RESULT__SUCCESS;
}

static connection_result_t event_received_body(connection_t *connection) {
  uint8_t result;
  entry_header_t *header_result;

  switch (connection->header.request.type) {
    case REQUEST_TYPE__GET:
      // TODO: Remove
      // printf("GET[%.*s]\n", 
      //     connection->entry_header->key_size,
      //     (uint8_t *)connection->entry_header + sizeof(entry_header_t));

      header_result = entry_hash_map_get(connection->entry_header);
      free(connection->entry_header);
      connection->flags &= ~CONNECTION_FLAG__ALLOCATED_BUFFER;
      connection->entry_header = header_result;

      if (connection->entry_header == NULL) {
        send_header(connection, RESPONSE_TYPE__NOT_FOUND);
        return CONNECTION_RESULT__SUCCESS;
      }

      entry_header_lock_event_t event = entry_header_acquire_lock(connection->entry_header);
      connection->flags |= CONNECTION_FLAG__LOCK_ACQUIRED;

      // TODO: Check expiry
      send_header(connection, RESPONSE_TYPE__VALUE);
      connection->header.response.argument.value_size = connection->entry_header->value_size;
      connection->remaining_bytes = sizeof(aggregate_response_header_t);
      break;

    case REQUEST_TYPE__DELETE:
      // TODO: Remove
      // printf("DELETE[%.*s]\n", 
      //     connection->entry_header->key_size,
      //     (uint8_t *)connection->entry_header + sizeof(entry_header_t));

      entry_hash_map_delete(connection->entry_header);
      free(connection->entry_header);
      send_header(connection, RESPONSE_TYPE__OK);
      break;

    case REQUEST_TYPE__PUT:
      result = entry_hash_map_put(connection->entry_header);

      // TODO: Remove
      // printf("PUT[%.*s]: %.*s\n", 
      //     connection->entry_header->key_size,
      //     (uint8_t *)connection->entry_header + sizeof(entry_header_t),
      //     connection->entry_header->value_size,
      //     (uint8_t *)connection->entry_header + sizeof(entry_header_t) + connection->entry_header->key_size);

      entry_hash_map_get(connection->entry_header);

      if (result) {
        send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
        return CONNECTION_RESULT__SUCCESS;
      }
      send_header(connection, RESPONSE_TYPE__OK);
      break;

    default:
      printf("Unsupported type for receiving body %u\n", connection->header.request.type);
      assert(0);
      break;
   }

   return CONNECTION_RESULT__SUCCESS;
}

static connection_result_t event_sent_header(connection_t *connection) {
  switch (connection->header.response.header.type) {
    // TODO: Should all of these end connections?
    case RESPONSE_TYPE__KEY_OVERSIZED:
    case RESPONSE_TYPE__OUT_OF_MEMORY:
    case RESPONSE_TYPE__UNKNOWN_REQUEST:
    case RESPONSE_TYPE__UNSUPPORTED_VERSION:
    case RESPONSE_TYPE__VALUE_OVERSIZED:
      connection->flags &= ~CONNECTION_FLAG__KEEP_ALIVE;

    case RESPONSE_TYPE__NOT_FOUND:
    case RESPONSE_TYPE__OK:
    case RESPONSE_TYPE__PONG:
      return finish_request(connection);

    case RESPONSE_TYPE__VALUE:
      connection->buffer = (void *)connection->entry_header
          + sizeof(entry_header_t)
          + connection->entry_header->key_size;
      connection->remaining_bytes = connection->entry_header->value_size;
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__SENDING_BODY;
      return CONNECTION_RESULT__SUCCESS;

    default:
      printf("Unsupported type for send header %u\n", connection->header.response.header.type);
      assert(0);
      break;
  }
}

static connection_result_t event_sent_body(connection_t *connection) {
  switch (connection->header.response.header.type) {
    case RESPONSE_TYPE__VALUE:
      connection->lock_release_event = entry_header_release_lock(connection->entry_header);
      connection->flags &= ~CONNECTION_FLAG__LOCK_ACQUIRED;
      return finish_request(connection);

    default:
      printf("Unsupported type for send body %u\n", connection->header.response.header.type);
      assert(0);
      break;
  }
}

static void init_request(connection_t *connection) {
  connection->buffer = &connection->header;
  connection->remaining_bytes = sizeof(request_header_t);
  connection->buffer_index = 0;
  connection->state = CONNECTION_STATE__RECEIVING_HEADER;
  connection->entry_header = NULL;
  connection->flags = CONNECTION_FLAG__NONE;
  connection->lock_release_event = ENTRY_HEADER_LOCK_EVENT__NONE;
}

static connection_result_t connection_transfer(connection_t *connection) {
  int result;

  switch (connection->state) {
    case CONNECTION_STATE__RECEIVING_HEADER:
    case CONNECTION_STATE__RECEIVING_ARGUMENTS:
    case CONNECTION_STATE__RECEIVING_BODY:
      result = recv(
          connection->file_descriptor,
          connection->buffer + connection->buffer_index,
          connection->remaining_bytes,
          0);
      break;

    case CONNECTION_STATE__SENDING_HEADER:
    case CONNECTION_STATE__SENDING_BODY:
      result = send(
          connection->file_descriptor,
          connection->buffer + connection->buffer_index,
          connection->remaining_bytes,
          0);
      break;

    default:
      printf("Unsupported state in transfer %u\n", connection->state);
      assert(0);
  }

  // TODO: Introduce timeout mid-request
  if (result < 1) {
    if (result == 0 
        || errno == EWOULDBLOCK 
        || errno == EAGAIN) {
      return CONNECTION_RESULT__NO_TRANSFER;
    }

    printf("Socket failure \"%s\"\n", strerror(errno));

    if (connection->flags & CONNECTION_FLAG__ALLOCATED_BUFFER) {
      free(connection->buffer);
    }
    if (connection->flags & CONNECTION_FLAG__LOCK_ACQUIRED) {
      connection->lock_release_event = entry_header_release_lock(connection->entry_header);
    }
    return CONNECTION_RESULT__DISCONNECT;
  }

  connection->remaining_bytes -= result;
  connection->buffer_index += result;
  if (connection->remaining_bytes != 0) {
    return CONNECTION_RESULT__PARTIAL_TRANSFER;
  }
  return CONNECTION_RESULT__SUCCESS;
}

static connection_result_t attempt_memory_write(connection_t *connection) {
  memory_queue_result_t result = memory_queue_allocate(
      connection->arguments.key_value.key_size,
      connection->arguments.key_value.value_size,
      time_get_timestamp() + configuration_get_int(CONFIGURATION_TYPE__ENTRY_EXPIRY_MICROS),
      &connection->entry_header);

  if (result == MEMORY_QUEUE_RESULT__OUT_OF_MEMORY) {
    send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
    return CONNECTION_RESULT__SUCCESS;
  }

  if (result == MEMORY_QUEUE_RESULT__WRITE_BLOCKED) {
    connection->state = CONNECTION_STATE__AWAITING_WRITE_UNBLOCK;
    return CONNECTION_RESULT__WRITE_BLOCKED;
  }

  connection->remaining_bytes = connection->arguments.key_value.key_size
      + connection->arguments.key_value.value_size;
  connection->buffer_index = 0;
  connection->buffer = (void *)connection->entry_header + sizeof(entry_header_t);
  connection->state = CONNECTION_STATE__RECEIVING_BODY;

  return CONNECTION_RESULT__SUCCESS;
}

static void send_header(connection_t *connection, response_type_t type) {
  connection->header.response.header.type = type;
  connection->buffer = &connection->header;
  connection->remaining_bytes = sizeof(response_header_t);
  connection->buffer_index = 0;
  connection->state = CONNECTION_STATE__SENDING_HEADER;
}

static connection_result_t finish_request(connection_t *connection) {
  if (connection->flags & CONNECTION_FLAG__KEEP_ALIVE) {
    init_request(connection);
    return CONNECTION_RESULT__NEW_REQUEST;
  }
  return CONNECTION_RESULT__DISCONNECT;
}

