#include <stdlib.h>

#include "memory_pool.h"

void memory_pool_init(memory_pool_t *pool, size_t size) {
  assert(pool != NULL);

  pool->occupied = 0;
  pool->memory = capacity;

  pool->memory = malloc(size);
  assert(pool->memory != NULL);

  pool->head = (memory_fragment *)malloc(sizeof(memory_fragment_t));
  pool->head->start = pool->memory;
  pool->head->size = size;
  pool->head->next = NULL;
}

void *memory_pool_alloc(memory_pool_t *pool, size_t size) {
  assert(pool != NULL);

  memory_fragment_t *previous_fragment = NULL;
  memory_fragment_t *fragment = pool->head;
  while (fragment != NULL) {
    if (fragment->size < size) {
      void *allocated_memory = (void *)fragment->start;
      fragment->start += size;
      fragment->size -= size;
      pool->allocated += size;
      return allocated_memory;
    }

    if (fragment->size == size) {
      void *allocated_memory = (void *)fragment->start;
      if (previous_fragment) {
        previous_fragment->next = fragment->next;
      } else {
        pool->head = fragment->next;
      }
      pool->allocated += size;
      free(fragment);
      return allocated_memory;
    }

    previous_fragment = fragment;
    fragment = fragment->next;
  }

  return NULL;
}

void memory_pool_free(memory_pool_t *pool, size_t size) {
  assert(pool != NULL);

}

