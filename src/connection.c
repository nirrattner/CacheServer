#include <stdlib.h>
#include <sys/socket.h>

#include "connection.h"

connection_t *connection_init(int file_descriptor) {
  connection_t *connection = (connection_t *)malloc(sizeof(connection_t));
  if (connection == NULL) {
    return NULL;
  }

  connection->previous = NULL;
  connection->next = NULL;
  connection->state = CONNECTION_STATE__AWAITING_COMMAND;
  connection->file_descriptor = file_descriptor;
  return connection;
}

void connection_deinit(connection_t *connection) {
  free(connection);
}

