#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "memory_queue.h"

static void evict_entry(memory_queue_t *queue);
static void remove_entry(memory_queue_t *queue, entry_header_t *entry_header);

memory_queue_t *memory_queue_init(uint64_t capacity_bytes) {
  memory_queue_t *queue = (memory_queue_t *)malloc(sizeof(memory_queue_t));
  if (queue == NULL) {
    return NULL;
  }

  queue->buffer = malloc(capacity_bytes);
  if (queue->buffer == NULL) {
    free(queue);
    return NULL;
  }

  queue->entry_count = 0;
  queue->occupied_bytes = 0;
  queue->capacity_bytes = capacity_bytes;
  queue->read_index = 0;
  queue->write_index = 0;
  
  return queue;
}

void memory_queue_deinit(memory_queue_t *queue) {
  free(queue->buffer);
  free(queue);
}

entry_header_t *memory_queue_put(memory_queue_t *queue, uint16_t key_size, uint32_t value_size, uint64_t expiry) {
  entry_header_t *entry_header = (entry_header_t *)(queue->buffer + queue->write_index);
  uint32_t total_size = sizeof(entry_header_t) + key_size + value_size;

  if (total_size > queue->capacity_bytes) {
    return NULL;
  }

  if (queue->write_index + total_size + sizeof(entry_header_t) >= queue->capacity_bytes) {
    while (queue->write_index < queue->read_index) {
      evict_entry(queue);
    }

    entry_header->end_of_buffer = 1;
    if (queue->read_index == queue->write_index) {
      queue->read_index = 0;
    }
    queue->write_index = 0;

    entry_header = (entry_header_t *)queue->buffer;
  }

  while (queue->entry_count > 0
      && queue->write_index <= queue->read_index
      && queue->write_index + total_size + sizeof(entry_header_t) >= queue->read_index) {
    evict_entry(queue);
  }

  entry_header->active = 1;
  entry_header->end_of_buffer = 0;
  entry_header->key_size = key_size;
  entry_header->value_size = value_size;
  entry_header->expiry = expiry;

  queue->entry_count++;
  queue->write_index += total_size;
  queue->occupied_bytes += total_size;

  return entry_header;
}

uint8_t memory_queue_expire(memory_queue_t *queue, uint64_t time) {
  entry_header_t *entry_header = (entry_header_t *)(queue->buffer + queue->read_index);

  if (queue->entry_count
      && entry_header->expiry <= time) {
    remove_entry(queue, entry_header);
    return 1;
  }

  return 0;
}

static void evict_entry(memory_queue_t *queue) {
  assert(queue->entry_count > 0);

  entry_header_t *entry_header = (entry_header_t *)(queue->buffer + queue->read_index);
  remove_entry(queue, entry_header);
}

static void remove_entry(memory_queue_t *queue, entry_header_t *entry_header) {
  uint32_t total_size = sizeof(entry_header_t) + entry_header->key_size + entry_header->value_size;

  if (entry_header->active) {
    // TODO
    // uint8_t *key_pointer = ((uint8_t *)entry_header) + sizeof(entry_header_t);
    // hash_delete(key_pointer, entry_header->key_size);
  }

  queue->entry_count--;
  queue->read_index += total_size;
  queue->occupied_bytes -= total_size;

  entry_header = (entry_header_t *)(queue->buffer + queue->read_index);
  if (entry_header->end_of_buffer) {
    queue->read_index = 0;
  }
}

