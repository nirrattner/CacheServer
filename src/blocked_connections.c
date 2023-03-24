#include <assert.h>
#include <stdlib.h>

#include "blocked_connections.h"

// TODO: Unit test
// TODO: Track timeout on blocks?
typedef struct blocked_connection {
  struct blocked_connection *next;
  connection_t *head;
  connection_t *tail;
  entry_header_t *entry_header;
  uint32_t size;
} blocked_connection_list_t;

typedef struct {
  blocked_connection_list_t *head;
} blocked_connections_context_t;

static blocked_connections_context_t context;

static blocked_connection_list_t *blocked_connection_list_init(connection_t *connection);
static void blocked_connection_list_deinit(blocked_connection_list_t *list);
static void blocked_connection_list_append(blocked_connection_list_t *list, connection_t *connection);
static connection_t *blocked_connection_list_pop(blocked_connection_list_t *list);


void blocked_connections_open(void) {
  context.head = NULL;
}

void blocked_connections_close(void) {
}

void blocked_connections_add(connection_t *connection) {
  blocked_connection_list_t *list = context.head;

  while (list != NULL) {
    if (list->entry_header == connection_get_entry_header(connection)) {
      blocked_connection_list_append(list, connection);
    }
  }

  list = blocked_connection_list_init(connection);
  list->next = context.head;
  context.head = list;
}

// TODO: Improve data structure for repeated and conditional pops
connection_t *blocked_connections_pop(entry_header_t *entry_header) {
  blocked_connection_list_t *previous_list = NULL;
  blocked_connection_list_t *list = context.head;

  while (list) {
    if (list->entry_header == entry_header) {
      connection_t *connection = blocked_connection_list_pop(list);
      if (list->size == 0) {
        if (previous_list == NULL) {
          context.head = list->next;
        } else {
          previous_list = list->next;
        }
        blocked_connection_list_deinit(list);
      }
      return connection;
    }
    previous_list = list;
    list = list->next;
  }
  return NULL;
}

static blocked_connection_list_t *blocked_connection_list_init(connection_t *connection) {
  blocked_connection_list_t *list = malloc(sizeof(blocked_connection_list_t));
  if (list == NULL) {
    return NULL;
  }

  connection_set_next(connection, NULL);

  list->head = connection;
  list->tail = connection;
  list->entry_header = connection_get_entry_header(connection);
  list->size = 1;

  return list;
}

static void blocked_connection_list_deinit(blocked_connection_list_t *list) {
  assert(list->size == 0);

  free(list);
}

static void blocked_connection_list_append(blocked_connection_list_t *list, connection_t *connection) {
  connection_set_next(connection, NULL);

  connection_set_next(list->tail, connection);
  list->tail = connection;
  list->size++;
}

static connection_t *blocked_connection_list_pop(blocked_connection_list_t *list) {
  assert(list->size > 0);

  connection_t *connection = list->head;
  list->head = connection_get_next(list->head);
  list->size--;
  return connection;
}

