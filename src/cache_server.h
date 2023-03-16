#ifndef _CACHE_SERVER_H
#define _CACHE_SERVER_H

#include <stdint.h>

#include "connection.h"
#include "entry_hash_map.h"
#include "memory_queue.h"

typedef struct {
  connection_t *current_connection;
  connection_t *head_connection;
  entry_hash_map_t *hash_map;
  memory_queue_t *memory_queue;
  int listen_file_descriptor;
  uint16_t active_connection_limit;
  uint16_t connection_count;
} cache_server_t;

cache_server_t *cache_server_init(
    uint64_t capacity_bytes,
    uint16_t active_connection_limit,
    const char *listen_ip_address,
    uint16_t listen_port);
void cache_server_deinit(cache_server_t *cache_server);

uint8_t cache_server_proc(cache_server_t *cache_server);

#endif

