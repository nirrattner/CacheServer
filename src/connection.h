#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdint.h>

#include "cache_communication_protocol.h"
#include "entry_header.h"

typedef enum {
  CONNECTION_RESULT__SUCCESS = 0,
  CONNECTION_RESULT__FAILURE,
} connection_result_t;

typedef enum {
  CONNECTION_STATE__AWAITING_HEADER = 0,
  CONNECTION_STATE__AWAITING_ARGUMENTS,
  CONNECTION_STATE__AWAITING_BODY,
  CONNECTION_STATE__SENDING_HEADER,
  CONNECTION_STATE__SENDING_ARGUMENTS,
  CONNECTION_STATE__SENDING_BODY,
} connection_state_t;

typedef enum {
  TRANSFER_TYPE__NONE = 0,
  TRANSFER_TYPE__RECEIVE,
  TRANSFER_TYPE__SEND,
} transfer_type_t;

typedef union {
  key_arguments_t key;
  key_value_arguments_t key_value;
} request_arguments_t;

typedef struct connection {
  struct connection *previous;
  struct connection *next;
  void *buffer;
  entry_header_t *entry_header;
  uint32_t remaining_bytes;
  uint32_t buffer_index;
  uint64_t last_interaction_timestamp;
  int file_descriptor;
  connection_state_t state;
  request_header_t header;
  request_arguments_t arguments;
  response_type_t response_type;
  transfer_type_t transfer_type;
} connection_t;

connection_t *connection_init(int file_descriptor);
void connection_deinit(connection_t *connection);

connection_result_t connection_proc(connection_t *connection);
void connection_close(connection_t *connection);

#endif

