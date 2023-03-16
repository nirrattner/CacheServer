#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entry_hash_map.h"

#define INITIAL_CAPACITY (0)
#define SEED_VALUE (0)

static int entry_compare(const void *entry_1, const void *entry_2, void *udata);
static uint64_t entry_hash(const void *entry, uint64_t seed0, uint64_t seed1);

entry_hash_map_t *entry_hash_map_init(void) {
  entry_hash_map_t *hash_map = (entry_hash_map_t *)malloc(sizeof(entry_hash_map_t));

  if (hash_map == NULL) {
    return NULL;
  }

  hash_map->hashmap = hashmap_new(
      sizeof(entry_hash_map_t *),
      INITIAL_CAPACITY,
      SEED_VALUE,
      SEED_VALUE,
      entry_hash,
      entry_compare,
      NULL,
      NULL);
  return hash_map;
}

void entry_hash_map_deinit(entry_hash_map_t *hash_map) {
  hashmap_free(hash_map->hashmap);
  free(hash_map);
}

entry_header_t *entry_hash_map_get(
    entry_hash_map_t *hash_map,
    entry_header_t *key_header) {
  entry_header_t **header_pointer = (entry_header_t **)hashmap_get(
      hash_map->hashmap,
      &key_header);

  if (header_pointer == NULL) {
    return NULL;
  }
  return *header_pointer;
}

void entry_hash_map_put(
    entry_hash_map_t *hash_map,
    entry_header_t *entry_header) {
  hashmap_set(
      hash_map->hashmap,
      &entry_header);
}

void entry_hash_map_delete(
    entry_hash_map_t *hash_map,
    entry_header_t *key_header) {
  hashmap_delete(
      hash_map->hashmap,
      &key_header);
}

static int entry_compare(const void *pointer_1, const void *pointer_2, void *udata) {
  entry_header_t *entry_header_1 = *((entry_header_t **)pointer_1);
  entry_header_t *entry_header_2 = *((entry_header_t **)pointer_2);

  if (entry_header_1->key_size != entry_header_2->key_size) {
    return entry_header_1->key_size > entry_header_2->key_size;
  }

  return strncmp(
      (const char *)(entry_header_1) + sizeof(entry_header_t),
      (const char *)(entry_header_2) + sizeof(entry_header_t),
      entry_header_1->key_size);
}

static uint64_t entry_hash(const void *pointer, uint64_t seed0, uint64_t seed1) {
  entry_header_t *entry_header = *((entry_header_t **)pointer);
  return hashmap_sip(
      entry_header + sizeof(entry_header_t), 
      entry_header->key_size, 
      seed0, 
      seed1);
}

