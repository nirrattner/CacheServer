#ifndef _CONNECTION_LIST_H
#define _CONNECTION_LIST_H

#include <stdint.h>

#include "connection.h"

uint8_t connection_list_open(void);
void connection_list_close(void);

void connection_list_append(connection_t *connection);
void connection_list_remove(connection_t *connection);

connection_t *connection_list_get_head(void);
connection_t *connection_list_get_tail(void);
uint16_t connection_list_get_count(void);

#endif

