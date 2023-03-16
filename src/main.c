#include <stdio.h>
#include <unistd.h>

#include "cache_server.h"

#define CAPACITY_BYTES (1024)
#define ACTIVE_CONNECTION_LIMIT (16)
#define LISTEN_IP_ADDRESS ("127.0.0.1")
#define LISTEN_PORT (8888)

int main() {
  uint8_t result = cache_server_open(
      CAPACITY_BYTES,
      ACTIVE_CONNECTION_LIMIT,
      LISTEN_IP_ADDRESS,
      LISTEN_PORT);

  if (result == 1) {
    printf("Failed to open server\n");
    return 1;
  }

  while (1) {
    cache_server_proc();
    sleep(1);
    printf(".\n");
  }
}

