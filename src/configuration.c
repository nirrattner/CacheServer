#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configuration.h"

typedef enum {
  DATA_TYPE__INT64 = 0,
  DATA_TYPE__STRING,
} data_type_t;

typedef enum {
  PARSE_STATE__AWAITING_KEY = 0,
  PARSE_STATE__AWAITING_VALUE,
} parse_state_t;

typedef struct {
  void *values[NUM_CONFIGURATION_TYPE];
} configuration_context_t;

static configuration_context_t context;

static data_type_t data_types[NUM_CONFIGURATION_TYPE] = {
  DATA_TYPE__INT64, // ACCEPT_PERIOD_MICROS
  DATA_TYPE__INT64, // CONNECTION_ACTIVE_LIMIT
  DATA_TYPE__INT64, // CONNECTION_BACKLOG_LIMIT
  DATA_TYPE__INT64, // CONNECTION_TIMEOUT_MICROS
  DATA_TYPE__INT64, // ENTRY_CAPACITY_BYTES
  DATA_TYPE__INT64, // ENTRY_EXPIRY_MICROS
  DATA_TYPE__STRING, // IP_ADDRESS
  DATA_TYPE__INT64, // PORT
};

static int64_t default_accept_period_micros = 1000;
static int64_t default_connection_active_limit = 1024;
static int64_t default_connection_backlog_limit = 32;
static int64_t default_connection_timeout_micros = 15000;
static int64_t default_entry_capacity_bytes = 1024 * 1024;
static int64_t default_entry_expiry_micros = 3600000;
static uint8_t default_ip_address[] = "127.0.0.1";
static int64_t default_port = 8888;

static void *default_values[NUM_CONFIGURATION_TYPE] = {
  &default_accept_period_micros,
  &default_connection_active_limit,
  &default_connection_backlog_limit,
  &default_connection_timeout_micros,
  &default_entry_capacity_bytes,
  &default_entry_expiry_micros,
  default_ip_address,
  &default_port,
};

static configuration_type_t parse_key(const char *input) {
  if (strcmp(input, "--accept-period-micros") == 0) {
    return CONFIGURATION_TYPE__ACCEPT_PERIOD_MICROS;
  }

  if (strcmp(input, "--active-connection-limit") == 0) {
    return CONFIGURATION_TYPE__CONNECTION_ACTIVE_LIMIT;
  }

  if (strcmp(input, "--connection-backlog-limit") == 0) {
    return CONFIGURATION_TYPE__CONNECTION_BACKLOG_LIMIT;
  }

  if (strcmp(input, "--connection-timeout-micros") == 0) {
    return CONFIGURATION_TYPE__CONNECTION_TIMEOUT_MICROS;
  }

  if (strcmp(input, "--entry-capacity-bytes") == 0) {
    return CONFIGURATION_TYPE__ENTRY_CAPACITY_BYTES;
  }

  if (strcmp(input, "--entry-expiry-micros") == 0) {
    return CONFIGURATION_TYPE__ENTRY_EXPIRY_MICROS;
  }

  if (strcmp(input, "--ip-address") == 0) {
    return CONFIGURATION_TYPE__IP_ADDRESS;
  }

  if (strcmp(input, "--port") == 0) {
    return CONFIGURATION_TYPE__PORT;
  }

  printf("Unrecognized configuration (%s)\n", input);
  exit(1);
}

static void parse_value(const char *input, configuration_type_t type) {
  uint32_t size;
  int64_t value;

  if (context.values[type]) {
    free(context.values[type]);
  }

  switch (data_types[type]) {
    case DATA_TYPE__INT64:
      context.values[type] = malloc(sizeof(int64_t));
      value = atoll(input);
      if (value == 0) {
        printf("WARN: Unable to parse configuration %u\n", type);
      }
      *((int64_t *)context.values[type]) = value;
      break;

    case DATA_TYPE__STRING:
      // TODO: Validate size
      size = strlen(input) + 1;
      context.values[type] = malloc(size);
      strcpy((char *)context.values[type], input);
      ((uint8_t *)context.values[type])[size - 1] = 0;
      break;

    default:
      printf("Unsupported parse type %u\n", type);
      assert(0);
      break;
  }
}

void configuration_parse(int argc, const char **argv) {
  parse_state_t state = PARSE_STATE__AWAITING_KEY;
  configuration_type_t type;
  int index;

  memset(context.values, 0, sizeof(context.values));

  for (index = 1; index < argc; index++) {
    switch (state) {
      case PARSE_STATE__AWAITING_KEY:
        type = parse_key(argv[index]);
        state = PARSE_STATE__AWAITING_VALUE;
        break;

      case PARSE_STATE__AWAITING_VALUE:
        state = PARSE_STATE__AWAITING_KEY;
        parse_value(argv[index], type);
        break;
    }
  }

  for (index = 0; index < NUM_CONFIGURATION_TYPE; index++) {
    if (context.values[index] == NULL) {
      context.values[index] = default_values[index];
    }
  }
}

int64_t configuration_get_int(configuration_type_t type) {
  return *((int64_t *)context.values[type]);
}

char *configuration_get_string(configuration_type_t type) {
  return (char *)context.values[type];
}

