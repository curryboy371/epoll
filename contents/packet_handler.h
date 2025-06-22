#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include "define.h"

void handle_chat_message(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_response(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_response(int client_fd, const uint8_t* body, size_t body_len);

void handle_admin_message(int client_fd, const uint8_t* body, size_t body_len);




#endif // PACKET_HANDLER_H