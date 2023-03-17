#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <stdint.h>

// TODO: Write unit tests

typedef enum {
  CONFIGURATION_TYPE__ACCEPT_PERIOD_MICROS = 0,
  CONFIGURATION_TYPE__CONNECTION_ACTIVE_LIMIT,
  CONFIGURATION_TYPE__CONNECTION_BACKLOG_LIMIT,
  CONFIGURATION_TYPE__CONNECTION_TIMEOUT_MICROS,
  CONFIGURATION_TYPE__ENTRY_CAPACITY_BYTES,
  CONFIGURATION_TYPE__ENTRY_EXPIRY_MICROS,
  CONFIGURATION_TYPE__IP_ADDRESS,
  CONFIGURATION_TYPE__PORT,

  NUM_CONFIGURATION_TYPE,
} configuration_type_t;

void configuration_parse(int argc, const char **argv);
int64_t configuration_get_int(configuration_type_t type);
char *configuration_get_string(configuration_type_t type);

#endif

