#ifndef RTP_RECEIVER_H
#define RTP_RECEIVER_H
#include <stdio.h>
#include <stdint.h>
#include "rtp_head.h"

typedef void (*recv_data_callback)(uint8_t* nalu,size_t len);

int recv_rtp(const char* ip,const char* port,recv_data_callback callback);

#endif // RTP_RECEIVER_H
