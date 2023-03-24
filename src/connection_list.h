#ifndef _CONNECTION_LIST_H
#define _CONNECTION_LIST_H

#include <poll.h>
#include <stdint.h>

#include "connection.h"

#define LISTEN_FILE_DESCRIPTOR_OFFSET (1)

uint8_t connection_list_open(uint32_t max_capacity);
void connection_list_close(void);

uint8_t connection_list_add(connection_t *connection);
connection_t *connection_list_get(uint32_t index);
void connection_list_remove(uint32_t index);

void connection_list_set_listen_file_descriptor(int listen_file_descriptor);
uint32_t connection_list_get_size(void);
struct pollfd *connection_list_get_pollfds(void);

#endif

