#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "entry_hash_map.h"
#include "memory_queue.h"
#include "time_util.h"

#define CONNECTION_TIMEOUT_US (5000)

// TODO: Set configurable expiry
#define EXPIRY_US (3600000)

static void event_received_header(connection_t *connection);
static void event_received_arguments(connection_t *connection);
static void event_received_body(connection_t *connection);
static connection_result_t event_sent_header(connection_t *connection);
static void event_sent_arguments(connection_t *connection);
static connection_result_t event_sent_body(connection_t *connection);

static void init_request(connection_t *connection);
static connection_result_t connection_transfer(connection_t *connection);
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

  connection->previous = NULL;
  connection->next = NULL;
  connection->file_descriptor = file_descriptor;
  connection->last_interaction_timestamp = 0;
  init_request(connection);
  return connection;
}

void connection_deinit(connection_t *connection) {
  free(connection);
}

connection_result_t connection_proc(connection_t *connection) {
  connection_result_t result = connection_transfer(connection);
  if (result != CONNECTION_RESULT__SUCCESS) {
    return result;
  }

  if (connection->remaining_bytes != 0) {
    return CONNECTION_RESULT__SUCCESS;
  }

  switch (connection->state) {
    case CONNECTION_STATE__RECEIVING_HEADER:
      event_received_header(connection);
      break;

    case CONNECTION_STATE__RECEIVING_ARGUMENTS:
      event_received_arguments(connection);
      break;

    case CONNECTION_STATE__RECEIVING_BODY:
      event_received_body(connection);
      break;

    case CONNECTION_STATE__SENDING_HEADER:
      return event_sent_header(connection);

    case CONNECTION_STATE__SENDING_ARGUMENTS:
      event_sent_arguments(connection);
      break;

    case CONNECTION_STATE__SENDING_BODY:
      return event_sent_body(connection);

    default:
      printf("Unsupported state %u\n", connection->state);
      assert(0);
  }

  return CONNECTION_RESULT__SUCCESS;
}

void connection_close(connection_t *connection) {
  close(connection->file_descriptor);
}

static void event_received_header(connection_t *connection) {
  // TODO: Verify version?
  if (connection->header.flags & REQUEST_FLAG__KEEP_ALIVE) {
    connection->flags |= CONNECTION_FLAG__KEEP_ALIVE;
  }

  switch (connection->header.type) {
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

static void event_received_arguments(connection_t *connection) {
  // TODO: Validate arguments
  switch (connection->header.type) {
    case REQUEST_TYPE__GET:
    case REQUEST_TYPE__DELETE:
      // TODO: free on failures
      connection->entry_header = (entry_header_t *)malloc(sizeof(entry_header_t)
          + connection->arguments.key.key_size);

      if (connection->entry_header == NULL) {
        send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
        return;
      }

      connection->flags |= CONNECTION_FLAG__ALLOCATED_BUFFER;
      connection->buffer = (void *)connection->entry_header + sizeof(entry_header_t);
      connection->remaining_bytes = connection->arguments.key.key_size;
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__RECEIVING_BODY;
      break;

    case REQUEST_TYPE__PUT:
      connection->entry_header = memory_queue_put(
          connection->arguments.key_value.key_size,
          connection->arguments.key_value.value_size,
          time_get_timestamp() + EXPIRY_US);

      if (connection->entry_header == NULL) {
        send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
        return;
      }

      connection->buffer = (void *)connection->entry_header + sizeof(entry_header_t);
      connection->remaining_bytes = connection->arguments.key_value.key_size
          + connection->arguments.key_value.value_size;
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__RECEIVING_BODY;
      break;

    default:
      printf("Unsupported type for receive arguments %u\n", connection->header.type);
      assert(0);
      break;
   }
}

static void event_received_body(connection_t *connection) {
  uint8_t result;
  entry_header_t *header_result;

  switch (connection->header.type) {
    case REQUEST_TYPE__GET:
      header_result = entry_hash_map_get(connection->entry_header);
      free(connection->entry_header);
      connection->flags &= ~CONNECTION_FLAG__ALLOCATED_BUFFER;
      connection->entry_header = header_result;
      if (connection->entry_header == NULL) {
        send_header(connection, RESPONSE_TYPE__NOT_FOUND);
        return;
      }

      // TODO: Check expiry
      send_header(connection, RESPONSE_TYPE__VALUE);
      break;

    case REQUEST_TYPE__DELETE:
      entry_hash_map_delete(connection->entry_header);
      free(connection->entry_header);
      send_header(connection, RESPONSE_TYPE__OK);
      break;

    case REQUEST_TYPE__PUT:
      result = entry_hash_map_put(connection->entry_header);
      if (result) {
        send_header(connection, RESPONSE_TYPE__OUT_OF_MEMORY);
        return;
      }
      send_header(connection, RESPONSE_TYPE__OK);
      break;

    default:
      printf("Unsupported type for receiving body %u\n", connection->header.type);
      assert(0);
      break;
   }
}

static connection_result_t event_sent_header(connection_t *connection) {
  switch (connection->response_type) {
    case RESPONSE_TYPE__KEY_OVERSIZED:
    case RESPONSE_TYPE__NOT_FOUND:
    case RESPONSE_TYPE__OUT_OF_MEMORY:
    case RESPONSE_TYPE__UNKNOWN_REQUEST:
    case RESPONSE_TYPE__UNSUPPORTED_VERSION:
    case RESPONSE_TYPE__VALUE_OVERSIZED:
      connection->flags &= ~CONNECTION_FLAG__KEEP_ALIVE;

    case RESPONSE_TYPE__OK:
    case RESPONSE_TYPE__PONG:
      return finish_request(connection);

    case RESPONSE_TYPE__VALUE:
      connection->arguments.value.value_size = connection->entry_header->value_size;
      connection->buffer = &connection->arguments.value.value_size;
      connection->remaining_bytes = sizeof(value_arguments_t);
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__SENDING_ARGUMENTS;
      return CONNECTION_RESULT__SUCCESS;

    default:
      printf("Unsupported type for send header %u\n", connection->response_type);
      assert(0);
      break;
  }
}

static void event_sent_arguments(connection_t *connection) {
  switch (connection->response_type) {
    case RESPONSE_TYPE__VALUE:
      connection->buffer = (void *)connection->entry_header
          + sizeof(entry_header_t)
          + connection->entry_header->key_size;
      connection->remaining_bytes = connection->entry_header->value_size;
      connection->buffer_index = 0;
      connection->state = CONNECTION_STATE__SENDING_BODY;
      break;

    default:
      printf("Unsupported type for send arguments %u\n", connection->response_type);
      assert(0);
      break;
  }
}

static connection_result_t event_sent_body(connection_t *connection) {
  switch (connection->response_type) {
    case RESPONSE_TYPE__VALUE:
      return finish_request(connection);

    default:
      printf("Unsupported type for send body %u\n", connection->response_type);
      assert(0);
      break;
  }
}

static void init_request(connection_t *connection) {
  connection->response_type = RESPONSE_TYPE__NONE;
  connection->buffer = &connection->header;
  connection->remaining_bytes = sizeof(request_header_t);
  connection->buffer_index = 0;
  connection->state = CONNECTION_STATE__RECEIVING_HEADER;
  connection->transfer_type = TRANSFER_TYPE__RECEIVE;
  connection->entry_header = NULL;
  connection->flags = CONNECTION_FLAG__NONE;
}

static connection_result_t connection_transfer(connection_t *connection) {
  int result;
  switch (connection->transfer_type) {
    case TRANSFER_TYPE__RECEIVE:
      result = recv(
          connection->file_descriptor,
          connection->buffer + connection->buffer_index,
          connection->remaining_bytes,
          0);
      break;

    case TRANSFER_TYPE__SEND:
      result = send(
          connection->file_descriptor,
          connection->buffer + connection->buffer_index,
          connection->remaining_bytes,
          0);
      break;

    default:
      printf("Unsupported type for arguments %u\n", connection->header.type);
      assert(0);
      break;
  }

  if (result == -1) {
    if ((errno == EWOULDBLOCK || errno == EAGAIN)
        && !(time_get_timestamp() - connection->last_interaction_timestamp > CONNECTION_TIMEOUT_US)) {
      return CONNECTION_RESULT__NO_TRANSFER;
    }
    printf("Socket failure %s", strerror(errno));

    if (connection->flags & CONNECTION_FLAG__ALLOCATED_BUFFER) {
      free(connection->buffer);
    }
    return CONNECTION_RESULT__DISCONNECT;
  }

  connection->last_interaction_timestamp = time_get_timestamp();
  connection->remaining_bytes -= result;
  connection->buffer_index += result;
  return CONNECTION_RESULT__SUCCESS;
}

static void send_header(connection_t *connection, response_type_t type) {
  connection->response_type = type;
  connection->buffer = &connection->response_type;
  connection->remaining_bytes = sizeof(response_type_t);
  connection->buffer_index = 0;
  connection->state = CONNECTION_STATE__SENDING_HEADER;
  connection->transfer_type = TRANSFER_TYPE__SEND;
}

static connection_result_t finish_request(connection_t *connection) {
  if (connection->flags & CONNECTION_FLAG__KEEP_ALIVE) {
    init_request(connection);
    return CONNECTION_RESULT__NEW_REQUEST;
  }
  return CONNECTION_RESULT__DISCONNECT;
}

