#ifndef _CACHE_SERVER_H
#define _CACHE_SERVER_H

#include <stdint.h>

#include "connection.h"

uint8_t cache_server_open(
    uint64_t capacity_bytes,
    uint16_t active_connection_limit,
    const char *listen_ip_address,
    uint16_t listen_port);
void cache_server_close(void);

uint8_t cache_server_proc(void);

#endif

