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
    next_connection = current_connection->next;
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

  connection->previous = context.tail;
  context.tail->next = connection;
  context.tail = connection;
}

void connection_list_remove(connection_t *connection) {
  context.count--;

  if (connection == context.tail) {
    assert(connection->next == NULL);
    context.tail = connection->previous;
  } else {
    connection->next->previous = connection->previous;
  }

  if (connection == context.head) {
    assert(connection->previous == NULL);
    context.head = connection->next;
  } else {
    connection->previous->next = connection->next;
  }

  connection_deinit(connection);
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

