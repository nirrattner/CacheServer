#ifndef _CONNECTION_H
#define _CONNECTION_H

typedef enum {
  CONNECTION_STATE__AWAITING_COMMAND = 0,
} connection_state_t;

typedef struct connection {
  struct connection *previous;
  struct connection *next;
  int file_descriptor;
  connection_state_t state;
} connection_t;

connection_t *connection_init(int file_descriptor);
void connection_deinit(connection_t *connection);

#endif

