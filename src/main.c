#include <stdio.h>
#include <unistd.h>

#include "cache_server.h"
#include "configuration.h"

int main(int argc, const char **argv) {
  configuration_parse(argc, argv);

  uint8_t result = cache_server_open();

  if (result == 1) {
    printf("Failed to open server\n");
    return 1;
  }

  while (1) {
    cache_server_proc();
  }
}

