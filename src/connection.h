#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdint.h>

#include "cache_communication_protocol.h"

typedef enum {
  CONNECTION_STATE__AWAITING_HEADER = 0,
  CONNECTION_STATE__AWAITING_ARGUMENTS,
  CONNECTION_STATE__AWAITING_BODY,
  CONNECTION_STATE__SENDING_RESPONSE,
} connection_state_t;

typedef union {
  key_arguments_t key;
  key_value_arguments_t key_value;
} request_arguments_t;

typedef struct connection {
  struct connection *previous;
  struct connection *next;
  void *buffer;
  uint64_t remaining_bytes;
  uint64_t write_index;
  uint64_t last_interaction_timestamp;
  int file_descriptor;
  connection_state_t state;
  request_header_t header;
  request_arguments_t arguments;
  response_type_t response_type;
} connection_t;

connection_t *connection_init(int file_descriptor);
void connection_deinit(connection_t *connection);

uint8_t connection_proc(connection_t *connection);

#endif

