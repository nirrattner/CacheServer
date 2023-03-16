#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdint.h>

#include "cache_communication_protocol.h"
#include "entry_header.h"

typedef enum {
  CONNECTION_RESULT__SUCCESS = 0,
  CONNECTION_RESULT__NO_TRANSFER,
  CONNECTION_RESULT__NEW_REQUEST,
  CONNECTION_RESULT__DISCONNECT,
} connection_result_t;

typedef struct connection connection_t;

connection_t *connection_init(int file_descriptor);
void connection_deinit(connection_t *connection);

connection_result_t connection_proc(connection_t *connection);
void connection_close(connection_t *connection);

connection_t *connection_get_previous(connection_t *connection);
void connection_set_previous(connection_t *connection, connection_t *previous);
connection_t *connection_get_next(connection_t *connection);
void connection_set_next(connection_t *connection, connection_t *next);

#endif

