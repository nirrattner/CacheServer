#include <assert.h>
#include <stdlib.h>

#include "memory_pool.h"

void memory_pool_init(memory_pool_t *pool, size_t size) {
  assert(pool != NULL);

  pool->allocated = 0;
  pool->capacity = size;
}

void *memory_pool_alloc(memory_pool_t *pool, size_t size) {
  assert(pool != NULL);

  if (pool->allocated + size > pool->capacity) {
    return NULL;
  }

  void *pointer = malloc(size);

  if (pointer != NULL) {
    pool->allocated += size;
  }

  return pointer;
}

void memory_pool_free(memory_pool_t *pool, void *pointer, size_t size) {
  assert(pool != NULL);

  pool->allocated -= size;
  free(pointer);
}

