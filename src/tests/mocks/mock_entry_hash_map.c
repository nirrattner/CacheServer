#include "entry_hash_map.h"

uint8_t entry_hash_map_open(void) {
  return 0;
}

void entry_hash_map_close(void) {
}

entry_header_t *entry_hash_map_get(entry_header_t *key_header) {
  return key_header;

}

uint8_t entry_hash_map_put(entry_header_t *entry_header) {
  return 0;
}

void entry_hash_map_delete(entry_header_t *key_header) {
  key_header->active = 0;
}

