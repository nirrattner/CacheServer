#include <stdio.h>
#include <unistd.h>

#include "cache_server.h"

#define CAPACITY_BYTES (1024)
#define ACTIVE_CONNECTION_LIMIT (16)
#define LISTEN_IP_ADDRESS ("127.0.0.1")
#define LISTEN_PORT (8888)

int main() {
  cache_server_t *server = cache_server_init(
      CAPACITY_BYTES,
      ACTIVE_CONNECTION_LIMIT,
      LISTEN_IP_ADDRESS,
      LISTEN_PORT);

  while (1) {
    cache_server_proc(server);
    sleep(1);
    printf(".\n");
  }
}

