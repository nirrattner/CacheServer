#ifndef _ENTRY_HEADER_H
#define _ENTRY_HEADER_H

#include <stdint.h>

// TODO: Add read lock to prevent deletions during read
// TODO: Can fit key/value sizes into 32 bits -- 8 bits key size/24 bits value size?

typedef enum {
  ENTRY_HEADER_LOCK_EVENT__NONE = 0,
  ENTRY_HEADER_LOCK_EVENT__BLOCKED,
  ENTRY_HEADER_LOCK_EVENT__READS_UNBLOCKED,
  ENTRY_HEADER_LOCK_EVENT__WRITES_UNBLOCKED,
} entry_header_lock_event_t;

typedef struct {
  uint64_t active : 1;
  uint64_t end_of_buffer : 1;
  uint64_t expiry : 62;
  uint32_t value_size;
  uint16_t key_size;
  uint16_t locks;
} entry_header_t;

entry_header_lock_event_t entry_header_acquire_lock(entry_header_t *entry_header);
entry_header_lock_event_t entry_header_release_lock(entry_header_t *entry_header);

#endif

