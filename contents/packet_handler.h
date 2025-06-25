#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include "define.h"

void handle_chat_message(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_response(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_response(int client_fd, const uint8_t* body, size_t body_len);

void handle_admin_message(int client_fd, const uint8_t* body, size_t body_len);

void handle_change_name_request(int client_fd, const uint8_t* body, size_t body_len);

void send_join_notice(User* user_info, int exept_fd);
void send_leave_notify(User* user_info);



#endif // PACKET_HANDLER_H