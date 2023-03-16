#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entry_hash_map.h"
#include "hashmap.h"

#define INITIAL_CAPACITY (0)
#define SEED_VALUE (0)

typedef struct {
  struct hashmap *hashmap;
} entry_hash_map_context_t;

static entry_hash_map_context_t context;

static int entry_compare(const void *entry_1, const void *entry_2, void *udata);
static uint64_t entry_hash(const void *entry, uint64_t seed0, uint64_t seed1);

uint8_t entry_hash_map_open(void) {
  context.hashmap = hashmap_new(
      sizeof(entry_header_t *),
      INITIAL_CAPACITY,
      SEED_VALUE,
      SEED_VALUE,
      entry_hash,
      entry_compare,
      NULL,
      NULL);
  
  if (context.hashmap == NULL) {
    return 1;
  }

  return 0;
}

void entry_hash_map_close(void) {
  if (context.hashmap != NULL) {
    hashmap_free(context.hashmap);
  }
}

entry_header_t *entry_hash_map_get(entry_header_t *key_header) {
  entry_header_t **header_pointer = (entry_header_t **)hashmap_get(
      context.hashmap,
      &key_header);

  if (header_pointer == NULL) {
    return NULL;
  }
  return *header_pointer;
}

// TODO: Check OOM
void entry_hash_map_put(entry_header_t *entry_header) {
  hashmap_set(
      context.hashmap,
      &entry_header);
}

void entry_hash_map_delete(entry_header_t *key_header) {
  hashmap_delete(
      context.hashmap,
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

