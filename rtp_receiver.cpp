#include "rtp_receiver.h"
#include "z_udp.h"
#include "h264_nal.h"

int recv_rtp(const char* ip,const char* port,recv_data_callback callback){
    int server_fd = udp_server(ip,port);
    if(server_fd < 0){
        fprintf(stderr,"create udp client failed!\n");
        return -1;
    }

    size_t BUFFER_SIZE = 1024 * 64;
    uint8_t* buffer = new uint8_t[BUFFER_SIZE];

    h264_nalu nalu;
    nalu.start_code_len = 4;

    while(true){

        struct sockaddr_in addr;
        int addr_len=sizeof(struct sockaddr_in);

        int len = udp_recvfrom(server_fd,buffer,BUFFER_SIZE,(sockaddr*)&addr,(socklen_t*)&addr_len);
        if(len <= 0){
            fprintf(stderr,"recv data failed! cclient will exit!\n");
            break;
        }

        // 用于指向有效数据的起始位置
        uint8_t* ptr = buffer;

        rtp_header_t* rtpHeader = (rtp_header*)ptr;
        ptr += RTP_HEADER_SIZE;

        len -= RTP_HEADER_SIZE;

        assert(rtpHeader->csrc_len <= CSRC_ITEM_MAX_NUM);
        if(rtpHeader->csrc_len > 0){
            ptr += rtpHeader->csrc_len * CSRC_ITEM_SIZE;
            len -= rtpHeader->csrc_len * CSRC_ITEM_SIZE;
        }

        fu_indicator_t* fuIndicator= (fu_indicator_t*)ptr;

        if(fuIndicator->type == 28){ // 一个Nalu分散在多个RTP包中

            fu_header_t* fuHeader  = (fu_header_t*)ptr + FU_INDICATOR_SIZE;

            len -= (FU_INDICATOR_SIZE + FU_HEADER_SIZE);

            ptr += (FU_INDICATOR_SIZE + FU_HEADER_SIZE);

            if(fuHeader->s == 1 && fuHeader->e == 0){
                uint8_t h264_nal_header = 0;

#ifdef RTP_STANDARD
                h264_nal_header |= fuIndicator->f;
                h264_nal_header |= (fuIndicator->nri) << 1;
                h264_nal_header |= (fuHeader->type) << 3;
#else
                h264_nal_header = (fuHeader->type & 0x1f)
                        | (fuIndicator->nri << 5)
                        | (fuIndicator->f << 7);
#endif
                nalu.buf[0] = h264_nal_header;
                nalu.nal_len += 1;

                memcpy(nalu.buf + nalu.nal_len,ptr,len);

                nalu.nal_len += len;
            }
            else if(fuHeader->s == 0 && fuHeader->e == 1){
                memcpy(nalu.buf + nalu.nal_len,ptr,len);
                nalu.nal_len += len;

                callback(nalu.buf,nalu.nal_len);

                nalu.nal_len = 0;
            }
            else{
                memcpy(nalu.buf + nalu.nal_len,ptr,len);
                nalu.nal_len += len;
            }
        }
        else{
            uint8_t h264_nal_header = 0;

#ifdef RTP_STANDARD
            unsigned char c = ((unsigned char*)fuIndicator)[0];
            h264_nal_header = c;
#else
            h264_nal_header = (fuIndicator->type & 0x1f)
                    | (fuIndicator->nri << 5)
                    | (fuIndicator->f << 7);
#endif

            nalu.buf[0] = h264_nal_header;
            nalu.nal_len += 1;

            len -= (FU_INDICATOR_SIZE );
            ptr += (FU_INDICATOR_SIZE );

            memcpy(nalu.buf + nalu.nal_len,ptr,len);
            nalu.nal_len += len;

            callback(nalu.buf,nalu.nal_len);

            nalu.nal_len = 0;
        }
    }

    delete[] buffer;
    return 0;
}

