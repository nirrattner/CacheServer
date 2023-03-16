#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "memory_queue.h"
#include "test_util.h"

#define CAPACITY (128)
#define EXPIRY (100)

static void it_inits(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  assert(queue != NULL);
  assert(queue->buffer != NULL);
  assert(queue->capacity_bytes == CAPACITY);

  memory_queue_deinit(queue);
}

static void it_puts(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  entry_header_t *entry_header = memory_queue_put(queue, 8, 8, EXPIRY);

  assert(entry_header != NULL);
  assert(queue->buffer == entry_header);
  assert(queue->entry_count == 1);
  assert(queue->occupied_bytes == 16 + sizeof(entry_header_t));
  assert(queue->read_index == 0);
  assert(queue->write_index == 16 + sizeof(entry_header_t));

  memory_queue_deinit(queue);
}

static void it_fails_to_put_over_capacity(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  entry_header_t *entry_header = memory_queue_put(queue, CAPACITY - sizeof(entry_header_t), 1, EXPIRY);

  assert(entry_header == NULL);
  assert(queue->entry_count == 0);
  assert(queue->occupied_bytes == 0);
  assert(queue->read_index == 0);
  assert(queue->write_index == 0);

  memory_queue_deinit(queue);
}

static void it_puts_and_expires(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  entry_header_t *entry_header = memory_queue_put(queue, 8, 8, EXPIRY);
  uint8_t entry_expired = memory_queue_expire(queue, EXPIRY + 10);

  assert(entry_expired == 1);

  assert(queue->entry_count == 0);
  assert(queue->occupied_bytes == 0);
  assert(queue->read_index == 16 + sizeof(entry_header_t));
  assert(queue->write_index == 16 + sizeof(entry_header_t));

  memory_queue_deinit(queue);
}

static void it_puts_and_does_not_expire_early(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  entry_header_t *entry_header = memory_queue_put(queue, 8, 8, EXPIRY);
  uint8_t entry_expired = memory_queue_expire(queue, EXPIRY - 10);

  assert(entry_expired == 0);

  assert(queue->buffer == entry_header);
  assert(queue->entry_count == 1);
  assert(queue->occupied_bytes == 16 + sizeof(entry_header_t));
  assert(queue->read_index == 0);
  assert(queue->write_index == 16 + sizeof(entry_header_t));

  memory_queue_deinit(queue);
}

static void it_puts_multiple(void) {
  PRINT_TEST()
  memory_queue_t *queue = memory_queue_init(CAPACITY);

  entry_header_t *entry_header_1 = memory_queue_put(queue, 8, 8, EXPIRY);
  entry_header_t *entry_header_2 = memory_queue_put(queue, 8, 8, EXPIRY);

  assert(entry_header_2 != NULL);
  assert(queue->buffer + sizeof(entry_header_t) + 16 == entry_header_2);
  assert(queue->entry_count == 2);
  assert(queue->occupied_bytes == 32 + 2 * sizeof(entry_header_t));
  assert(queue->read_index == 0);
  assert(queue->write_index == 32 + 2 * sizeof(entry_header_t));

  memory_queue_deinit(queue);
}

// TODO - Test:
// - Wrap around
// - End-of-buffer logic
// - Eviction scenarios

int main() {
  it_inits();
  it_puts();
  it_fails_to_put_over_capacity();
  it_puts_and_expires();
  it_puts_and_does_not_expire_early();
  it_puts_multiple();

  return 0;
}

