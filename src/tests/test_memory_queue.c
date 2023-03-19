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

static void it_allocates(void) {
  PRINT_TEST()
  entry_header_t *entry_header;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(entry_header != NULL);
  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_fails_to_allocate_over_capacity(void) {
  PRINT_TEST()
  entry_header_t *entry_header = NULL;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(CAPACITY - sizeof(entry_header_t), 1, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__OUT_OF_MEMORY);

  assert(entry_header == NULL);
  assert(memory_queue_get_entry_count() == 0);
  assert(memory_queue_get_occupied_bytes() == 0);
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 0);

  memory_queue_close();
}

static void it_cleans_expired(void) {
  PRINT_TEST()
  entry_header_t *entry_header;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  result = memory_queue_clean(EXPIRY + 10);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(memory_queue_get_entry_count() == 0);
  assert(memory_queue_get_occupied_bytes() == 0);
  assert(memory_queue_get_read_index() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_cleans_inactive(void) {
  PRINT_TEST()
  entry_header_t *entry_header;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  result = memory_queue_clean(EXPIRY - 10);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(memory_queue_get_entry_count() == 0);
  assert(memory_queue_get_occupied_bytes() == 0);
  assert(memory_queue_get_read_index() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_does_not_clean_unexpired_active(void) {
  PRINT_TEST()
  entry_header_t *entry_header;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);
  entry_header->active = 1;

  result = memory_queue_clean(EXPIRY - 10);
  assert(result == MEMORY_QUEUE_RESULT__NONE);

  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_does_not_clean_locked(void) {
  PRINT_TEST()
  entry_header_t *entry_header;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);
  entry_header->locks = 1;

  result = memory_queue_clean(EXPIRY - 10);
  assert(result == MEMORY_QUEUE_RESULT__WRITE_BLOCKED);

  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 16 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 16 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_allocates_multiple(void) {
  PRINT_TEST()
  entry_header_t *entry_header_1;
  entry_header_t *entry_header_2;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header_1);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  result = memory_queue_allocate(8, 8, EXPIRY, &entry_header_2);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(entry_header_2 != NULL);
  assert(memory_queue_get_entry_count() == 2);
  assert(memory_queue_get_occupied_bytes() == 32 + 2 * sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 32 + 2 * sizeof(entry_header_t));

  memory_queue_close();
}

static void it_wraps_around_with_two_large_entries(void) {
  PRINT_TEST()
  entry_header_t *entry_header_1;
  entry_header_t *entry_header_2;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 72, EXPIRY, &entry_header_1);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  result = memory_queue_allocate(8, 80, EXPIRY, &entry_header_2);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(memory_queue_get_entry_count() == 1);
  assert(memory_queue_get_occupied_bytes() == 88 + sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 88 + sizeof(entry_header_t));

  memory_queue_close();
}

static void it_wraps_around_with_three_entries(void) {
  PRINT_TEST()
  entry_header_t *entry_header_1;
  entry_header_t *entry_header_2;
  entry_header_t *entry_header_3;
  memory_queue_result_t result;
  uint8_t open_result = memory_queue_open(CAPACITY);
  assert(open_result == 0);

  result = memory_queue_allocate(8, 32, EXPIRY, &entry_header_1);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  result = memory_queue_allocate(4, 4, EXPIRY, &entry_header_2);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(memory_queue_get_entry_count() == 2);
  assert(memory_queue_get_occupied_bytes() == 48 + 2 * sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 0);
  assert(memory_queue_get_write_index() == 48 + 2 * sizeof(entry_header_t));

  result = memory_queue_allocate(4, 18, EXPIRY, &entry_header_3);
  assert(result == MEMORY_QUEUE_RESULT__SUCCESS);

  assert(memory_queue_get_entry_count() == 2);
  assert(memory_queue_get_occupied_bytes() == 30 + 2 * sizeof(entry_header_t));
  assert(memory_queue_get_read_index() == 40 + sizeof(entry_header_t));
  assert(memory_queue_get_write_index() == 22 + sizeof(entry_header_t));

  entry_header_t *end_of_buffer_header = (entry_header_t *)((void *)entry_header_2 + 8 + sizeof(entry_header_t));
  assert(end_of_buffer_header->end_of_buffer == 1);

  memory_queue_close();
}

// TODO - Test:
// - Wrap around
// - End-of-buffer logic

int main() {
  it_opens();
  it_allocates();
  it_fails_to_allocate_over_capacity();
  it_cleans_expired();
  it_cleans_inactive();
  it_does_not_clean_unexpired_active();
  it_does_not_clean_locked();
  it_allocates_multiple();
  it_wraps_around_with_two_large_entries();
  it_wraps_around_with_three_entries();

  return 0;
}

