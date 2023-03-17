#ifndef _CACHE_PROTOCOL_H
#define _CACHE_PROTOCOL_H

#define CACHE_PROTOCOL_VERSION (1)

typedef enum {
  REQUEST_TYPE__NONE = 0,
  REQUEST_TYPE__GET,
  REQUEST_TYPE__PUT,
  REQUEST_TYPE__DELETE,
  REQUEST_TYPE__PING,

  // TODO: Stats
  // REQUEST_TYPE__STATS,
} request_type_t;

typedef enum {
  RESPONSE_TYPE__NONE = 0,
  RESPONSE_TYPE__OK,
  RESPONSE_TYPE__VALUE,
  RESPONSE_TYPE__PONG,
  RESPONSE_TYPE__KEY_OVERSIZED,
  RESPONSE_TYPE__NOT_FOUND,
  RESPONSE_TYPE__OUT_OF_MEMORY,
  RESPONSE_TYPE__UNKNOWN_REQUEST,
  RESPONSE_TYPE__UNSUPPORTED_VERSION,
  RESPONSE_TYPE__VALUE_OVERSIZED,
} response_type_t;

typedef enum {
  REQUEST_FLAG__NONE = 0,
  REQUEST_FLAG__KEEP_ALIVE = (1 << 0),
} request_flag_t;

#pragma pack(1)
// TODO: Separate `version` into connection header?
typedef struct {
  uint16_t version;
  uint8_t flags;
  uint8_t type;
} request_header_t;

typedef struct {
  uint16_t key_size;
} key_arguments_t;

typedef struct {
  uint16_t key_size;
  uint32_t value_size;
} key_value_arguments_t;

typedef struct {
  uint32_t value_size;
} value_arguments_t;
#pragma pack()

#endif

