#ifndef _BLOCKED_CONNECTIONS_H
#define _BLOCKED_CONNECTIONS_H

#include <stdint.h>

#include "connection.h"

typedef enum {
  BLOCK_TYPE__READ = 0,
  BLOCK_TYPE__WRITE,
} block_type_t;

void blocked_connections_open(void);
void blocked_connections_close(void);

void blocked_connections_add(connection_t *connection, block_type_t type);
connection_t *blocked_connections_pop(entry_header_t *entry_header, block_type_t type);

#endif

