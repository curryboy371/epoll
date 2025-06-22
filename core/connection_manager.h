#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "define.h"

void connection_handle_accept();
void connection_handle_read(int client_fd);

#endif //CONNECTION_MANAGER_H