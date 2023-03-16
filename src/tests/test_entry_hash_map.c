#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entry_hash_map.h"
#include "test_util.h"

// printf("%s:%d\n", __FILE__, __LINE__);

static entry_header_t *generate_entry(const char *key, const char *value);

static void it_inits(void) {
  PRINT_TEST()

  entry_hash_map_t *hash_map = entry_hash_map_init();

  assert(hash_map != NULL);
  assert(hash_map->hashmap != NULL);

  entry_hash_map_deinit(hash_map);
}

static void it_gets_null_for_miss(void) {
  PRINT_TEST()

  entry_header_t *entry_key = generate_entry("key-1", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == NULL);

  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static void it_puts_and_gets(void) {
  PRINT_TEST()

  entry_header_t *entry_header = generate_entry("key-1", "value-1");
  entry_header_t *entry_key = generate_entry("key-1", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_hash_map_put(hash_map, entry_header);

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == entry_header);

  free(entry_header);
  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static void it_deletes(void) {
  PRINT_TEST()

  entry_header_t *entry_header = generate_entry("key-1", "value-1");
  entry_header_t *entry_key = generate_entry("key-1", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_hash_map_put(hash_map, entry_header);

  entry_hash_map_delete(hash_map, entry_key);

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == NULL);

  free(entry_header);
  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static void it_puts_multiple_and_gets(void) {
  PRINT_TEST()

  entry_header_t *entry_header_1 = generate_entry("key-1", "value-1");
  entry_header_t *entry_header_2 = generate_entry("key-2", "value-1");
  entry_header_t *entry_header_3 = generate_entry("key-3", "value-1");
  entry_header_t *entry_key = generate_entry("key-2", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_hash_map_put(hash_map, entry_header_1);
  entry_hash_map_put(hash_map, entry_header_2);
  entry_hash_map_put(hash_map, entry_header_3);

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == entry_header_2);

  free(entry_header_1);
  free(entry_header_2);
  free(entry_header_3);
  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static void it_puts_multiple_deletes_and_gets(void) {
  PRINT_TEST()

  entry_header_t *entry_header_1 = generate_entry("key-1", "value-1");
  entry_header_t *entry_header_2 = generate_entry("key-2", "value-1");
  entry_header_t *entry_header_3 = generate_entry("key-3", "value-1");
  entry_header_t *entry_key = generate_entry("key-2", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_hash_map_put(hash_map, entry_header_1);
  entry_hash_map_put(hash_map, entry_header_2);
  entry_hash_map_put(hash_map, entry_header_3);

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == entry_header_2);

  free(entry_header_1);
  free(entry_header_2);
  free(entry_header_3);
  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static void it_puts_multiple_deletes_and_gets_empty(void) {
  PRINT_TEST()

  entry_header_t *entry_header_1 = generate_entry("key-1", "value-1");
  entry_header_t *entry_header_2 = generate_entry("key-2", "value-1");
  entry_header_t *entry_header_3 = generate_entry("key-3", "value-1");
  entry_header_t *entry_key = generate_entry("key-2", "");

  entry_hash_map_t *hash_map = entry_hash_map_init();

  entry_hash_map_put(hash_map, entry_header_1);
  entry_hash_map_put(hash_map, entry_header_2);
  entry_hash_map_put(hash_map, entry_header_3);

  entry_hash_map_delete(hash_map, entry_key);

  entry_header_t *entry_header_get = entry_hash_map_get(hash_map, entry_key);

  assert(entry_header_get == NULL);

  free(entry_header_1);
  free(entry_header_2);
  free(entry_header_3);
  free(entry_key);
  entry_hash_map_deinit(hash_map);
}

static entry_header_t *generate_entry(const char *key, const char *value) {
  assert(key != NULL);
  assert(value != NULL);

  uint16_t key_size = strlen(key);
  uint32_t value_size = strlen(value);
  void *buffer = malloc(sizeof(entry_header_t) + key_size + value_size);

  assert(buffer != NULL);

  entry_header_t *entry_header = (entry_header_t *)buffer;
  entry_header->key_size = key_size;
  entry_header->value_size = value_size;
  memcpy(buffer + sizeof(entry_header_t), (void *)key, key_size);
  memcpy(buffer + sizeof(entry_header_t) + key_size, (void *)value, value_size);

  return entry_header;
}

int main() {
  it_inits();
  it_gets_null_for_miss();
  it_puts_and_gets();
  it_deletes();
  it_puts_multiple_and_gets();
  it_puts_multiple_deletes_and_gets();
  it_puts_multiple_deletes_and_gets_empty();

  return 0;
}


