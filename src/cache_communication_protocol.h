#ifndef _CACHE_COMMUNICATION_PROTOCOL_H
#define _CACHE_COMMUNICATION_PROTOCOL_H

typedef enum {
  REQUEST_TYPE__HEARTBEAT = 0,
  REQUEST_TYPE__GET,
  REQUEST_TYPE__PUT,
  REQUEST_TYPE__DELETE,
} request_type_t;

typedef enum {
  RESPONSE_TYPE__OK = 0,
} response_type_t;

#pragma pack(1)
typedef struct {
  uint16_t key_size;
} get_request_header_t;

typedef struct {
  uint16_t key_size;
  uint32_t value_size;
} put_request_header_t;

typedef struct {
  uint16_t key_size;
} delete_request_header_t;
#pragma pack

#endif

