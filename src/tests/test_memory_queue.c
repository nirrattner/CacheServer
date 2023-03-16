#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "memory_queue.h"
#include "test_util.h"

#define CAPACITY (128)
#define EXPIRY (100)

static void it_opens(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  assert(memory_queue_get_capacity_bytes() == CAPACITY);

  memory_queue_close();
}

static void it_puts(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  entry_header_t *entry_header = memory_queue_put(8, 8, EXPIRY);

  assert(entry_header != NULL);
  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_fails_to_put_over_capacity(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  entry_header_t *entry_header = memory_queue_put(CAPACITY - sizeof(entry_header_t), 1, EXPIRY);

  assert(entry_header == NULL);
  assert(memory_queue_get_entry_count() == 0);
  assert(memory_queue_get_occupied_bytes() == 0);
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 0);

  memory_queue_close();
}

static void it_puts_and_expires(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  entry_header_t *entry_header = memory_queue_put(8, 8, EXPIRY);
  uint8_t entry_expired = memory_queue_expire(EXPIRY + 10);

  assert(entry_expired == 1);
  assert(memory_queue_get_entry_count() == 0);
  assert(memory_queue_get_occupied_bytes() == 0);
  assert(memory_queue_get_read_index() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_puts_and_does_not_expire_early(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  entry_header_t *entry_header = memory_queue_put(8, 8, EXPIRY);
  uint8_t entry_expired = memory_queue_expire(EXPIRY - 10);

  assert(entry_expired == 0);
  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_puts_multiple(void) {
  PRINT_TEST()
  uint8_t result = memory_queue_open(CAPACITY);
  assert(result == 0);

  entry_header_t *entry_header_1 = memory_queue_put(8, 8, EXPIRY);
  entry_header_t *entry_header_2 = memory_queue_put(8, 8, EXPIRY);

  assert(entry_header_2 != NULL);
  assert(memory_queue_get_entry_count() == 2);
  assert(memory_queue_get_occupied_bytes() == 32 + 2 * sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 32 + 2 * sizeof(entry_header_t));

  memory_queue_close();
}

// TODO - Test:
// - Wrap around
// - End-of-buffer logic
// - Eviction scenarios

int main() {
  it_opens();
  it_puts();
  it_fails_to_put_over_capacity();
  it_puts_and_expires();
  it_puts_and_does_not_expire_early();
  it_puts_multiple();

  return 0;
}

