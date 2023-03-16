#ifndef _MEMORY_QUEUE_H
#define _MEMORY_QUEUE_H

#include "entry_header.h"

typedef struct {
  void *buffer;
  uint64_t entry_count;
  uint64_t occupied_bytes;
  uint64_t capacity_bytes;
  uint64_t read_index;
  uint64_t write_index;
} memory_queue_t;

memory_queue_t *memory_queue_init(uint64_t capacity);
void memory_queue_deinit(memory_queue_t *queue);

entry_header_t *memory_queue_put(memory_queue_t *queue, uint16_t key_size, uint32_t value_size, uint64_t expiry);
uint8_t memory_queue_expire(memory_queue_t *queue, uint64_t time);

#endif

