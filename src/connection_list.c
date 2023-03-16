#include <assert.h>
#include <stdlib.h>

#include "connection_list.h"

typedef struct {
  connection_t *head;
  connection_t *tail;
  uint16_t count;
} connection_list_context_t;

static connection_list_context_t context;

uint8_t connection_list_open(void) {
  context.head = NULL;
  context.tail = NULL;
  context.count = 0;

  return 0;
}

void connection_list_close(void) {
  connection_t *current_connection = context.head;
  connection_t *next_connection;

  while (current_connection) {
    next_connection = connection_get_next(current_connection);
    connection_deinit(current_connection);
    current_connection = next_connection;
  }
}

void connection_list_append(connection_t *connection) {
  context.count++;

  if (context.head == NULL) {
    context.head = connection;
    context.tail = connection;
    return;
  }

  connection_set_previous(connection, context.tail);
  connection_set_next(context.tail, connection);
  context.tail = connection;
}

void connection_list_remove(connection_t *connection) {
  context.count--;

  if (connection == context.tail) {
    assert(connection_get_next(connection) == NULL);
    context.tail = connection_get_previous(connection);
  } else {
    connection_set_previous(
        connection_get_next(connection),
        connection_get_previous(connection));
  }

  if (connection == context.head) {
    assert(connection_get_previous(connection) == NULL);
    context.head = connection_get_next(connection);
  } else {
    connection_set_next(
        connection_get_previous(connection), 
        connection_get_next(connection));
  }
}

connection_t *connection_list_get_head(void) {
  return context.head;
}

connection_t *connection_list_get_tail(void) {
  return context.tail;
}

uint16_t connection_list_get_count(void) {
  return context.count;
}

