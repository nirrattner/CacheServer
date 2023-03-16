#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory_pool.h"
#include "test_util.h"

static void it_allocates_whole_memory(void) {
  PRINT_TEST()

  memory_pool_t memory_pool;
  memory_pool_init(&memory_pool, 10);

  void *pointer = memory_pool_alloc(&memory_pool, 10);

  assert(pointer != NULL);
  assert(memory_pool.allocated == 10);

  memory_pool_free(&memory_pool, pointer, 10);
  assert(memory_pool.allocated == 0);
}

static void it_allocates_memory_in_parts(void) {
  PRINT_TEST()

  memory_pool_t memory_pool;
  memory_pool_init(&memory_pool, 10);

  void *pointer1 = memory_pool_alloc(&memory_pool, 3);
  void *pointer2 = memory_pool_alloc(&memory_pool, 3);
  void *pointer3 = memory_pool_alloc(&memory_pool, 4);

  assert(pointer1 != NULL);
  assert(pointer2 != NULL);
  assert(pointer3 != NULL);
  assert(memory_pool.allocated == 10);

  memory_pool_free(&memory_pool, pointer1, 3);
  memory_pool_free(&memory_pool, pointer2, 3);
  memory_pool_free(&memory_pool, pointer3, 4);
  assert(memory_pool.allocated == 0);
}

static void it_fails_to_allocated_when_at_allocated(void) {
  PRINT_TEST()

  memory_pool_t memory_pool;
  memory_pool_init(&memory_pool, 10);

  void *pointer1 = memory_pool_alloc(&memory_pool, 10);
  void *pointer2 = memory_pool_alloc(&memory_pool, 1);

  assert(pointer1 != NULL);
  assert(pointer2 == NULL);
  assert(memory_pool.allocated == 10);

  memory_pool_free(&memory_pool, pointer1, 10);
  assert(memory_pool.allocated == 0);
}

static void it_allocates_after_freed_from_allocated(void) {
  PRINT_TEST()

  memory_pool_t memory_pool;
  memory_pool_init(&memory_pool, 10);

  void *pointer1 = memory_pool_alloc(&memory_pool, 10);

  memory_pool_free(&memory_pool, pointer1, 10);

  void *pointer2 = memory_pool_alloc(&memory_pool, 1);

  assert(pointer2 != NULL);
  assert(memory_pool.allocated == 1);

  memory_pool_free(&memory_pool, pointer2, 1);
}

int main() {
  it_allocates_whole_memory();
  it_allocates_memory_in_parts();
  it_fails_to_allocated_when_at_allocated();
  it_allocates_after_freed_from_allocated();

  return 0;
}
