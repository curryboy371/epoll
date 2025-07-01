#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include "define.h"
#include "packet.h"
#include "task/task_queue.h"

#include <protobuf-c/protobuf-c.h>

size_t make_packet(uint16_t cmd, const ProtobufCMessage* msg, uint8_t* out_buf);

Task make_send_task(int target_fd, uint16_t cmd, const ProtobufCMessage* msg);

void enqueue_broadcast_packet(uint16_t cmd, const ProtobufCMessage* msg);
void enqueue_exept_broadcast_packet(uint16_t cmd, const ProtobufCMessage* msg, int except_fd);

void handle_chat_message(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_login_response(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_request(int client_fd, const uint8_t* body, size_t body_len);
void handle_join_response(int client_fd, const uint8_t* body, size_t body_len);




void handle_admin_message(int client_fd, const uint8_t* body, size_t body_len);
void handle_chat_command(int client_fd, const uint8_t* body, size_t body_len);

void handle_change_name_request(int client_fd, const uint8_t* body, size_t body_len);


void send_join_notice(User* user_info, int exept_fd);
void send_leave_notify(User* user_info);



#endif // PACKET_HANDLER_H