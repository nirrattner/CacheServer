#ifndef _MOCK_CONNECTION_H
#define _MOCK_CONNECTION_H

#include "connection.h"

int connection_get_file_descriptor(connection_t *connection);

struct connection {
  struct connection *previous;
  struct connection *next;
  int file_descriptor;
};

#endif
