#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include "define.h"

#define RECV_BUFFER_SIZE 4096

typedef struct {
    uint8_t buffer[RECV_BUFFER_SIZE];
    size_t read_offset;  // 읽기 시작 위치 (read offset)
    size_t write_offset;  // 쓰기 위치 (write offset)

} RecvBuffer;

void recv_buffer_init(RecvBuffer* rb);
void recv_buffer_append(RecvBuffer* rb, const uint8_t* data, size_t len);
ParseResult recv_buffer_extract_packet(RecvBuffer* rb, uint8_t* out_packet, size_t* out_len);

#endif