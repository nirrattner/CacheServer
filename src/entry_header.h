#ifndef _ENTRY_HEADER_H
#define _ENTRY_HEADER_H

// TODO: Add read lock to prevent deletions during read

typedef struct {
  uint64_t active : 1;
  uint64_t end_of_buffer : 1;
  uint64_t expiry : 62;
  uint16_t key_size;
  uint32_t value_size;
} entry_header_t;

#endif

