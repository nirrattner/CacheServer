#include <assert.h>

#include "entry_header.h"

#define ENTRY_HEADER_LOCK_CAPACITY ((1 << 20) - 1)

entry_header_lock_event_t entry_header_acquire_lock(entry_header_t *entry_header) {
  assert(entry_header->locks < ENTRY_HEADER_LOCK_CAPACITY);

  entry_header->locks++;
  return ENTRY_HEADER_LOCK_EVENT__NONE;
}

entry_header_lock_event_t entry_header_release_lock(entry_header_t *entry_header) {
  assert(entry_header->locks >= 0);

  entry_header->locks--;
  if (entry_header->locks == 0) {
    return ENTRY_HEADER_LOCK_EVENT__WRITES_UNBLOCKED;
  }
  return ENTRY_HEADER_LOCK_EVENT__NONE;
}

