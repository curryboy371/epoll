#include "recv_buffer.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

void recv_buffer_init(RecvBuffer* rb) {
    rb->read_offset = rb->write_offset = 0;
}

// 수신된 data를 버퍼에 누적하는 함수
// 각 클라이언트의 buffer에 누적함
// STREAM data이므로 이 함수가 호출되는 상황에서 패킷이 잘려있을 수도, 여러 패킷이 붙어있을 수도 있음
// 따라서 먼저 버퍼로 수신해놓고 이후 패킷을 열어 처리함
void recv_buffer_append(RecvBuffer* rb, const uint8_t* data, size_t len) {

    if (rb->write_offset + len > RECV_BUFFER_SIZE) {
        printf("Recv buffer overflow!\n");
        return;
    }

    // 기존 누적 버퍼 뒤에 새로운 data를 붙임
    memcpy(rb->buffer + rb->write_offset, data, len);
    rb->write_offset += len;

}

// 버퍼를 확인하여 패킷이 완전히 받아졌는지 확인하는 함수
ParseResult recv_buffer_extract_packet(RecvBuffer* rb, uint8_t* out_packet, size_t* out_len) {
    // 최소 data 체크 (header size)
    size_t available = rb->write_offset - rb->read_offset;
    if (available < 2)
        return PARSE_INCOMPLETE;

    // buffer data에서 header를 꺼내 패킷의 길이를 확인
    uint16_t net_len;
    memcpy(&net_len, rb->buffer + rb->read_offset, 2);
    uint16_t total_len = ntohs(net_len); // network to host byte

    // 비정상적인 패킷
    if (total_len > RECV_BUFFER_SIZE)
        return PARSE_ERROR;

    // 패킷 미완성
    if (available < total_len)
        return PARSE_INCOMPLETE;

    // 패킷 완성
    memcpy(out_packet, rb->buffer + rb->read_offset, total_len);
    *out_len = total_len;

    rb->read_offset += total_len;

    // 버퍼를 다 읽었다면 초기화
    if (rb->read_offset == rb->write_offset) {
        recv_buffer_init(rb);
    }

    return PARSE_COMPLETE;
}