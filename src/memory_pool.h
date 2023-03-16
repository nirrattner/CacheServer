#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

typedef struct {
  size_t allocated;
  size_t capacity;
} memory_pool_t;

void memory_pool_init(memory_pool_t *pool, size_t size);
void *memory_pool_alloc(memory_pool_t *pool, size_t size);
void memory_pool_free(memory_pool_t *pool, void *pointer, size_t size);

#endif

