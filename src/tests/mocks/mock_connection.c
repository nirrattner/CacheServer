#include <stdlib.h>

#include "connection.h"

struct connection {
  struct connection *previous;
  struct connection *next;
  int file_descriptor;
};

connection_t *connection_init(int file_descriptor) {
  connection_t *connection = (connection_t *)malloc(sizeof(connection_t));
  if (connection == NULL) {
    return NULL;
  }

  connection->previous = NULL;
  connection->next = NULL;
  connection->file_descriptor = file_descriptor;
  return connection;
}

void connection_deinit(connection_t *connection) {
  free(connection);
}

connection_result_t connection_proc(connection_t *connection) {
}

void connection_close(connection_t *connection) {
}

connection_t *connection_get_previous(connection_t *connection) {
  return connection->previous;
}

void connection_set_previous(connection_t *connection, connection_t *previous) {
  connection->previous = previous;
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

