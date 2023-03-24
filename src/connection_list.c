#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "connection_list.h"

#define MIN_CAPACITY (4)

// TODO: Downsize on time interval

typedef struct {
  connection_t **connections;
  struct pollfd *pollfds;
  uint32_t size;
  uint32_t capacity;
  uint32_t max_capacity;
} connection_list_context_t;

static connection_list_context_t context;

static uint8_t resize(uint32_t size);

uint8_t connection_list_open(uint32_t max_capacity) {
  assert(MIN_CAPACITY <= max_capacity);

  context.size = 0;
  context.capacity = MIN_CAPACITY;
  context.max_capacity = max_capacity;

  context.connections = (connection_t **)malloc(context.capacity * sizeof(connection_t *));
  context.pollfds = (struct pollfd *)malloc((context.capacity + LISTEN_FILE_DESCRIPTOR_OFFSET) * sizeof(struct pollfd));

  if (context.connections == NULL
      || context.pollfds == NULL) {
    return 1;
  }

  return 0;
}

void connection_list_close(void) {
  free(context.connections);
  free(context.pollfds);
}

uint8_t connection_list_add(connection_t *connection) {
  uint32_t result;

  if (context.size == context.max_capacity) {
    return 1;
  }

  if (context.size == context.capacity) {
    result = resize(context.capacity << 1);
    if (result != 0) {
      return result;
    }
  }

  context.connections[context.size] = connection;
  context.pollfds[context.size + LISTEN_FILE_DESCRIPTOR_OFFSET].fd = connection_get_file_descriptor(connection);
  context.pollfds[context.size + LISTEN_FILE_DESCRIPTOR_OFFSET].events = connection_get_pollfd_events(connection);
  context.size++;
  return 0;
}

connection_t *connection_list_get(uint32_t index) {
  assert(index < context.size);
  return context.connections[index];
}

void connection_list_remove(uint32_t index) {
  assert(index < context.size);
  context.size--;

  if (index != context.size) {
    context.connections[index] = context.connections[context.size];
    context.pollfds[index + LISTEN_FILE_DESCRIPTOR_OFFSET] = context.pollfds[context.size + LISTEN_FILE_DESCRIPTOR_OFFSET];
  }
}

void connection_list_set_listen_file_descriptor(int listen_file_descriptor) {
  context.pollfds[0].fd = listen_file_descriptor;
  context.pollfds[0].events = POLLIN;
}

uint32_t connection_list_get_size(void) {
  return context.size;
}

struct pollfd *connection_list_get_pollfds(void) {
  return context.pollfds;
}

static uint8_t resize(uint32_t capacity) {
  if (capacity > context.max_capacity) {
    capacity = context.max_capacity;
  } else if (capacity < MIN_CAPACITY) {
    capacity = MIN_CAPACITY;
  }

  assert(context.size <= capacity);

  if (context.capacity == capacity) {
    return 1;
  }
  context.capacity = capacity;

  connection_t **new_connections = (connection_t **)malloc(context.capacity * sizeof(connection_t *));
  struct pollfd *new_pollfds = (struct pollfd *)malloc((context.capacity + 1) * sizeof(struct pollfd));

  if (new_connections == NULL
      || new_pollfds == NULL) {
    return 1;
  }

  memcpy(new_connections, context.connections, context.size * sizeof(connection_t *));
  free(context.connections);
  context.connections = new_connections;

  memcpy(new_pollfds, context.pollfds, (context.size + LISTEN_FILE_DESCRIPTOR_OFFSET) * sizeof(struct pollfd));
  free(context.pollfds);
  context.pollfds = new_pollfds;

  return 0;
}
