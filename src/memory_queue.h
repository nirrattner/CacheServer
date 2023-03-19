#ifndef _MEMORY_QUEUE_H
#define _MEMORY_QUEUE_H

#include "entry_header.h"

typedef enum {
  MEMORY_QUEUE_RESULT__SUCCESS = 0,
  MEMORY_QUEUE_RESULT__NONE,
  MEMORY_QUEUE_RESULT__OUT_OF_MEMORY,
  MEMORY_QUEUE_RESULT__WRITE_BLOCKED,
} memory_queue_result_t;

uint8_t memory_queue_open(uint64_t capacity);
void memory_queue_close(void);

memory_queue_result_t memory_queue_allocate(
    uint16_t key_size,
    uint32_t value_size,
    uint64_t expiry,
    entry_header_t **entry_header);
memory_queue_result_t memory_queue_clean(uint64_t time);

uint64_t memory_queue_get_entry_count(void);
uint64_t memory_queue_get_occupied_bytes(void);
uint64_t memory_queue_get_capacity_bytes(void);
uint64_t memory_queue_get_read_index(void);
uint64_t memory_queue_get_write_index(void);

#endif

