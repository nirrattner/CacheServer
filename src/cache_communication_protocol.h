#ifndef _CACHE_COMMUNICATION_PROTOCOL_H
#define _CACHE_COMMUNICATION_PROTOCOL_H

typedef enum {
  REQUEST_TYPE__NONE = 0,
  REQUEST_TYPE__PING,
  REQUEST_TYPE__GET,
  REQUEST_TYPE__PUT,
  REQUEST_TYPE__DELETE,
} request_type_t;

typedef enum {
  RESPONSE_TYPE__NONE = 0,
  RESPONSE_TYPE__PONG,
  RESPONSE_TYPE__OK,
  RESPONSE_TYPE__VALUE,
  RESPONSE_TYPE__NOT_FOUND,
  RESPONSE_TYPE__UNSUPPORTED_VERSION,
  RESPONSE_TYPE__UNSUPPORTED_TYPE,
} response_type_t;

typedef enum {
  REQUEST_FLAG__NON_TERMINATING,
} request_flag_t;

#pragma pack(1)
typedef struct {
  uint32_t version;
  uint32_t flags;
  request_type_t type;
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

