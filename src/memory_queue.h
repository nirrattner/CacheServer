#ifndef _MEMORY_QUEUE_H
#define _MEMORY_QUEUE_H

#include "entry_header.h"

uint8_t memory_queue_open(uint64_t capacity);
void memory_queue_close(void);

entry_header_t *memory_queue_put(uint16_t key_size, uint32_t value_size, uint64_t expiry);
uint8_t memory_queue_expire(uint64_t time);

uint64_t memory_queue_get_entry_count(void);
uint64_t memory_queue_get_occupied_bytes(void);
uint64_t memory_queue_get_capacity_bytes(void);
uint64_t memory_queue_get_read_index(void);
uint64_t memory_queue_get_write_index(void);

#endif

