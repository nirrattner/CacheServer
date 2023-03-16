#ifndef _CACHE_SERVER_H
#define _CACHE_SERVER_H

#include <stdint.h>

#include "connection.h"

uint8_t cache_server_open(void);
void cache_server_close(void);

void cache_server_proc(void);

#endif

