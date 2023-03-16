#include <sys/time.h>

#include "time_util.h"

#define SECONDS_TO_MICROS (1000000)

uint64_t time_get_timestamp(void) {
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * SECONDS_TO_MICROS + time_value.tv_usec;
}

