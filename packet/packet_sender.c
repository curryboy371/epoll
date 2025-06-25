#include "packet_sender.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "server_context.h"

#include <protobuf-c/protobuf-c.h>

int send_packet(int client_fd, PacketCommand cmd, const void* message) {

    const ProtobufCMessage* pb = (const ProtobufCMessage*)message;

    if (!pb->descriptor) {
        printf("pb->descriptor is NULL!\n");
        return -1;
    }


    size_t body_size = protobuf_c_message_get_packed_size(pb);
    if (body_size > MAX_BODY_SIZE) {
        printf("Body size overflow!\n");
        return -1;
    }

    uint8_t packet_buf[BUFFER_SIZE];
    uint16_t total_len = PACKET_HEADER_SIZE + body_size;

    uint16_t net_len = htons(total_len);
    uint16_t net_cmd = htons((uint16_t)cmd);
    memcpy(packet_buf, &net_len, 2);
    memcpy(packet_buf + 2, &net_cmd, 2);

    // 직렬화
    protobuf_c_message_pack(pb, packet_buf + 4);

    printf("send packet cmd %d\n", cmd);
    return session_send(&server_ctx.session, client_fd, packet_buf, total_len);
}

int broadcast_session_packet(PacketCommand cmd, const void* message) {

    const ProtobufCMessage* pb = (const ProtobufCMessage*)message;
    size_t body_size = protobuf_c_message_get_packed_size(pb);
    if (body_size > MAX_BODY_SIZE) {
        printf("Body size overflow!\n");
        return -1;
    }

    uint8_t packet_buf[BUFFER_SIZE];
    uint16_t total_len = PACKET_HEADER_SIZE + body_size;

    uint16_t net_len = htons(total_len);
    uint16_t net_cmd = htons((uint16_t)cmd);
    memcpy(packet_buf, &net_len, 2);
    memcpy(packet_buf + 2, &net_cmd, 2);

    // 직렬화
    protobuf_c_message_pack(pb, packet_buf + 4);

    printf("send session broadcast packet cmd %d\n", cmd);
    session_broadcast(&server_ctx.session, packet_buf, total_len);
    return 0;
}


int broadcast_user_packet(PacketCommand cmd, const void* message) {

    const ProtobufCMessage* pb = (const ProtobufCMessage*)message;
    size_t body_size = protobuf_c_message_get_packed_size(pb);
    if (body_size > MAX_BODY_SIZE) {
        printf("Body size overflow!\n");
        return -1;
    }

    uint8_t packet_buf[BUFFER_SIZE];
    uint16_t total_len = PACKET_HEADER_SIZE + body_size;

    uint16_t net_len = htons(total_len);
    uint16_t net_cmd = htons((uint16_t)cmd);
    memcpy(packet_buf, &net_len, 2);
    memcpy(packet_buf + 2, &net_cmd, 2);

    // 직렬화
    protobuf_c_message_pack(pb, packet_buf + 4);

    printf("send user broadcast packet cmd %d\n", cmd);
    user_manager_broadcast(&server_ctx.user, &server_ctx.session, packet_buf, total_len);
    return 0;
}

int broadcast_user_packet_exept(PacketCommand cmd, const void* message, const int except_fd) {

    const ProtobufCMessage* pb = (const ProtobufCMessage*)message;

    size_t body_size = protobuf_c_message_get_packed_size(pb);
    if (body_size > MAX_BODY_SIZE) {
        printf("Body size overflow!\n");
        return -1;
    }

    uint8_t packet_buf[BUFFER_SIZE];
    uint16_t total_len = PACKET_HEADER_SIZE + body_size;

    uint16_t net_len = htons(total_len);
    uint16_t net_cmd = htons((uint16_t)cmd);
    memcpy(packet_buf, &net_len, 2);
    memcpy(packet_buf + 2, &net_cmd, 2);

    // 직렬화
    protobuf_c_message_pack(pb, packet_buf + 4);

    printf("send user broadcast packet cmd %d\n", cmd);
    user_manager_broadcast_except(&server_ctx.user, except_fd, &server_ctx.session, packet_buf, total_len);
    return 0;

}