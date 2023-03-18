#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "entry_hash_map.h"
#include "memory_queue.h"

typedef struct {
  void *buffer;
  uint64_t entry_count;
  uint64_t occupied_bytes;
  uint64_t capacity_bytes;
  uint64_t read_index;
  uint64_t write_index;
} memory_queue_context_t;

static memory_queue_context_t context;

static void evict_entry();
static void remove_entry(entry_header_t *entry_header);

uint8_t memory_queue_open(uint64_t capacity_bytes) {
  context.buffer = malloc(capacity_bytes);
  if (context.buffer == NULL) {
    return 1;
  }

  context.entry_count = 0;
  context.occupied_bytes = 0;
  context.capacity_bytes = capacity_bytes;
  context.read_index = 0;
  context.write_index = 0;
  
  return 0;
}

void memory_queue_close(void) {
  if (context.buffer != NULL) {
    free(context.buffer);
  }
}

entry_header_t *memory_queue_put(uint16_t key_size, uint32_t value_size, uint64_t expiry) {
  entry_header_t *entry_header = (entry_header_t *)(context.buffer + context.write_index);
  uint32_t total_size = sizeof(entry_header_t) + key_size + value_size;

  if (total_size > context.capacity_bytes) {
    return NULL;
  }

  if (context.write_index + total_size + sizeof(entry_header_t) >= context.capacity_bytes) {
    while (context.write_index < context.read_index) {
      evict_entry();
    }

    entry_header->end_of_buffer = 1;
    if (context.read_index == context.write_index) {
      context.read_index = 0;
    }
    context.write_index = 0;

    entry_header = (entry_header_t *)context.buffer;
  }

  while (context.entry_count > 0
      && context.write_index <= context.read_index
      && context.write_index + total_size + sizeof(entry_header_t) >= context.read_index) {
    evict_entry();
  }

  entry_header->active = 0;
  entry_header->end_of_buffer = 0;
  entry_header->key_size = key_size;
  entry_header->value_size = value_size;
  entry_header->expiry = expiry;

  context.entry_count++;
  context.write_index += total_size;
  context.occupied_bytes += total_size;

  return entry_header;
}

uint8_t memory_queue_expire(uint64_t time) {
  entry_header_t *entry_header = (entry_header_t *)(context.buffer + context.read_index);

  if (context.entry_count
      && entry_header->expiry <= time) {
    remove_entry(entry_header);
    return 1;
  }

  return 0;
}

static void evict_entry() {
  assert(context.entry_count > 0);

  entry_header_t *entry_header = (entry_header_t *)(context.buffer + context.read_index);
  remove_entry(entry_header);
}

static void remove_entry(entry_header_t *entry_header) {
  uint32_t total_size = sizeof(entry_header_t) + entry_header->key_size + entry_header->value_size;

  context.entry_count--;
  context.read_index += total_size;
  context.occupied_bytes -= total_size;

  if (entry_header->active) {
    entry_hash_map_delete(entry_header);
  }

  entry_header = (entry_header_t *)(context.buffer + context.read_index);
  if (entry_header->end_of_buffer) {
    context.read_index = 0;
  }
}

uint64_t memory_queue_get_entry_count(void) {
  return context.entry_count;
}

uint64_t memory_queue_get_occupied_bytes(void) {
  return context.occupied_bytes;
}

uint64_t memory_queue_get_capacity_bytes(void) {
  return context.capacity_bytes;
}

uint64_t memory_queue_get_read_index(void) {
  return context.read_index;
}

uint64_t memory_queue_get_write_index(void) {
  return context.write_index;
}

