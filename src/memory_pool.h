#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

typedef struct memory_fragment {
  uintptr_t start;
  size_t size;
  memory_fragment *next;
} memory_fragment_t;

typedef struct {
  void *memory;
  memory_fragment_t *head;
  size_t occupied;
  size_t capacity;
} memory_pool_t;

void memory_pool_init(memory_pool_t *pool, size_t size);
void *memory_pool_alloc(memory_pool_t *pool, size_t size);
void memory_pool_free(memory_pool_t *pool, size_t size);

#endif

