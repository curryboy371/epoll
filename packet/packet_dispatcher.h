#ifndef PACKET_DISPATCHER_H

#define PACKET_DISPATCHER_H

#include "define.h"


void worker_dispatcher(int client_fd, const uint8_t* data, size_t len);
void system_dispatcher(int client_fd, const uint8_t* data, size_t len);

#endif // PACKET_DISPATCHER_H