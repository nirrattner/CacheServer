#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "time_util.h"

static void event_awaiting_header(connection_t *connection);
static void event_awaiting_arguments(connection_t *connection);
static void event_awaiting_body(connection_t *connection);
static void event_sending_response(connection_t *connection);

static void init_request(connection_t *connection);
static uint8_t receive(connection_t *connection);

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

  // connection->buffer = malloc(sizeof(request_header_t));
  // if (connection->buffer == NULL) {
  //   printf("ERROR: Unable to allocate buffer\n");
  //   return NULL;
  // }

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

uint8_t connection_proc(connection_t *connection) {
  // TODO: Check last interaction for timeout
  // TODO: Handle disconnect from send/recv failure

  switch (connection->state) {
    case CONNECTION_STATE__AWAITING_HEADER:
      event_awaiting_header(connection);
      break;
    case CONNECTION_STATE__AWAITING_ARGUMENTS:
      event_awaiting_arguments(connection);
      break;
    case CONNECTION_STATE__AWAITING_BODY:
      event_awaiting_body(connection);
      break;
    case CONNECTION_STATE__SENDING_RESPONSE:
      event_sending_response(connection);
      break;
    default:
      printf("Unsupported state %u\n", connection->state);
      assert(0);
  }

  return 0;
}

static void event_awaiting_header(connection_t *connection) {
  receive(connection);
  if (connection->remaining_bytes != 0) {
    return;
  }

  // TODO: Verify version?
  switch (connection->header.type) {
    case REQUEST_TYPE__PING:
      connection->response_type = RESPONSE_TYPE__PONG;
      connection->state = CONNECTION_STATE__SENDING_RESPONSE;
      break;
    case REQUEST_TYPE__GET:
    case REQUEST_TYPE__DELETE:
      connection->buffer = &connection->arguments;
      connection->remaining_bytes = sizeof(key_arguments_t);
      connection->write_index = 0;
      connection->state = CONNECTION_STATE__AWAITING_ARGUMENTS;
      break;
    case REQUEST_TYPE__PUT:
      connection->buffer = &connection->arguments;
      connection->remaining_bytes = sizeof(key_value_arguments_t);
      connection->write_index = 0;
      connection->state = CONNECTION_STATE__AWAITING_ARGUMENTS;
      break;
    default:
      connection->response_type = RESPONSE_TYPE__UNSUPPORTED_TYPE;
      connection->state = CONNECTION_STATE__SENDING_RESPONSE;
      break;
   }
}

static void event_awaiting_arguments(connection_t *connection) {
  receive(connection);
  if (connection->remaining_bytes != 0) {
    return;
  }
}

static void event_awaiting_body(connection_t *connection) {
  receive(connection);
  if (connection->remaining_bytes != 0) {
    return;
  }
}

static void event_sending_response(connection_t *connection) {

}

static void init_request(connection_t *connection) {
  connection->response_type = RESPONSE_TYPE__NONE;
  connection->buffer = &connection->header;
  connection->remaining_bytes = sizeof(request_header_t);
  connection->write_index = 0;
  connection->state = CONNECTION_STATE__AWAITING_HEADER;
}

static uint8_t receive(connection_t *connection) {
  int result = recv(
      connection->file_descriptor,
      connection->buffer + connection->write_index,
      connection->remaining_bytes,
      0);

  if (result == -1) {
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
      return 0;
    }
    // TODO: Close connection on receive failure
    return 1;
  }

  connection->last_interaction_timestamp = time_get_timestamp();
  connection->remaining_bytes -= result;
  connection->write_index += result;
  return 0;
}

