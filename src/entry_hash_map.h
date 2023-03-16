#ifndef _ENTRY_HASH_MAP_H
#define _ENTRY_HASH_MAP_H

#include <stdint.h>

#include "entry_header.h"

uint8_t entry_hash_map_open(void);
void entry_hash_map_close(void);

entry_header_t *entry_hash_map_get(entry_header_t *key_header);
uint8_t entry_hash_map_put(entry_header_t *entry_header);
void entry_hash_map_delete(entry_header_t *key_header);

#endif

