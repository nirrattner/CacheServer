#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <sys/socket.h>

typedef enum {
  CONNECTION_STATE__DISCONNECTED = 0,
} connection_state_t;

typedef struct {
  connection_state_t state;
  int socket_file_descriptor;
} connection_t;

#endif

