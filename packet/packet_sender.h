#ifndef PACKET_SENDER_H
#define PACKET_SENDER_H

#include "define.h"
#include "packet.h"

int send_packet(int client_fd, PacketCommand cmd, const void* message);

int broadcast_session_packet(PacketCommand cmd, const void* message);
int broadcast_user_packet(PacketCommand cmd, const void* message);

#endif // PACKET_SENDER_H