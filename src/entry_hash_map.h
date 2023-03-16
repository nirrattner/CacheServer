#ifndef _ENTRY_HASH_MAP_H
#define _ENTRY_HASH_MAP_H

#include "entry_header.h"
#include "hashmap.h"

typedef struct {
  struct hashmap *hashmap;
} entry_hash_map_t;

entry_hash_map_t *entry_hash_map_init(void);
void entry_hash_map_deinit(entry_hash_map_t *hash_map);

entry_header_t *entry_hash_map_get(entry_hash_map_t *hash_map, entry_header_t *key_header);
void entry_hash_map_put(entry_hash_map_t *hash_map, entry_header_t *entry_header);
void entry_hash_map_delete(entry_hash_map_t *hash_map, entry_header_t *key_header);

#endif
