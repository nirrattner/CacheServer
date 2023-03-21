#ifndef _BLOCKED_CONNECTIONS_H
#define _BLOCKED_CONNECTIONS_H

#include <stdint.h>

#include "connection.h"

void blocked_connections_open(void);
void blocked_connections_close(void);

void blocked_connections_add(connection_t *connection);
connection_t *blocked_connections_pop(entry_header_t *entry_header);

#endif

