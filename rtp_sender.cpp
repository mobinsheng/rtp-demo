#include "rtp_sender.h"
#include "z_udp.h"
#include "z_bits.h"
#include "h264_nal.h"

int send_rtp(const char*  ip,const char* port,const char* filename){
    int fd = udp_client(ip,port);
    if(fd < 0){
        printf("create client failed!\n");
        return -1;
    }
    int on = 1;
    /*
     * 设置广播属性
     */
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
        fprintf(stderr, "initSvr: Socket options set error.\n");
        return -1;
    }

    FILE* fp = fopen(filename,"rb");
    if(fp == NULL){
        return -1;
    }

    const int frameRate = 5;

    uint16_t seq_num = 0;
    uint32_t time_stamp = 0;

    uint8_t* nalu = new uint8_t[NAL_MAX_SIZE];
    uint8_t* send_data = new uint8_t[NAL_MAX_SIZE + 64];
    size_t send_len = 0;

    int len = 0;
    bool flag = false;

    H264Reader reader(fp);

    while(reader.ReadNalu(nalu,&len)){

        rtp_header_t* rtp_hdr = 0;
        fu_indicator_t* fu_ind = NULL;
        fu_header_t* fu_hdr = NULL;

        if(len <= RTP_PAYLOAD_MAX_SIZE){
            rtp_hdr = (rtp_header_t*)send_data;
            fu_ind = (fu_indicator_t*)send_data + RTP_HEADER_SIZE;

            rtp_hdr->csrc_len = 0;
            rtp_hdr->extension = 0;
            //rtp_hdr->marker = 0;
            rtp_hdr->padding = 0;
            rtp_hdr->version = 2;
            rtp_hdr->payload_type = H264_Type;
            rtp_hdr->seq_no = htons(++seq_num % UINT16_MAX);
            rtp_hdr->timestamp = htonl(time_stamp);
            rtp_hdr->ssrc = htonl( SSRC_NUM);

#ifdef RTP_STANDARD
            *((unsigned char*)fu_ind) = nalu[0];
            //fu_ind->f = (nalu[0] & 0x01) ;        /* bit0 */
            //fu_ind->nri = (nalu[0] & 0x03) >> 1;      /* bit1~2 */
            //fu_ind->type = (nalu[0] & 0xF1) >> 3;
#else
            fu_ind->f = (nalu[0] & 0x80) >> 7;        /* bit0 */
            fu_ind->nri = (nalu[0] & 0x60) >> 5;      /* bit1~2 */
            fu_ind->type = (nalu[0] & 0x1f);
#endif

            memcpy(send_data + RTP_HEADER_SIZE + FU_INDICATOR_SIZE,
                   nalu + 1,len - 1);

            send_len = RTP_HEADER_SIZE + FU_INDICATOR_SIZE + len - 1;

            udp_send(fd,send_data,send_len);


        }
        else{
            int num = 0;

            if(len % RTP_PAYLOAD_MAX_SIZE == 0){
                num = len / RTP_PAYLOAD_MAX_SIZE;
            }
            else{
                num = len / RTP_PAYLOAD_MAX_SIZE + 1;
            }

            uint8_t* data = nalu + 1;
            for(int i = 0; i < num; ++i){
                rtp_hdr = (rtp_header_t*)send_data;
                fu_ind = (fu_indicator_t*)send_data + RTP_HEADER_SIZE;
                fu_hdr = (fu_header_t*)send_data + RTP_HEADER_SIZE + FU_INDICATOR_SIZE;

                rtp_hdr->csrc_len = 0;
                rtp_hdr->extension = 0;
                rtp_hdr->marker = 0;
                rtp_hdr->padding = 0;
                rtp_hdr->payload_type = H264_Type;
                rtp_hdr->seq_no = htons(++seq_num % UINT16_MAX);
                rtp_hdr->ssrc = htonl( SSRC_NUM);
                rtp_hdr->timestamp = htonl(time_stamp);
                rtp_hdr->version = 2;

#ifdef RTP_STANDARD
                fu_ind->f = (nalu[0] & 0x01) ;        /* bit0 */
                fu_ind->nri = (nalu[0] & 0x06) >> 1;      /* bit1~2 */
                fu_ind->type = 28;
#else
                fu_ind->f = (nalu[0] & 0x80) >> 7;
                fu_ind->nri = (nalu[0] & 0x60) >> 5;
                fu_ind->type = 28;
#endif

#ifdef RTP_STANDARD
                fu_hdr->type = (nalu[0] & 0xF1) >> 3;
#else
                fu_hdr->type = nalu[0] & 0x1f;
#endif

                fu_hdr->s = 0;
                fu_hdr->e = 0;
                fu_hdr->r = 0;

                size_t realLen = RTP_PAYLOAD_MAX_SIZE;
                if(i == 0){
                    fu_hdr->s = 1;
                    fu_hdr->e = 0;
                    realLen -= 1; //  因为nalu头部已经被塞到fu字段中了
                }
                else if(i == num -1){
                    rtp_hdr->marker = 1;

                    fu_hdr->s = 0;
                    fu_hdr->e = 1;

                    if(len % RTP_PAYLOAD_MAX_SIZE != 0){
                        realLen = len % RTP_PAYLOAD_MAX_SIZE;
                    }
                }
                else{
                    fu_hdr->s = 0;
                    fu_hdr->e = 0;
                }

                memcpy(send_data + RTP_HEADER_SIZE + FU_INDICATOR_SIZE + FU_HEADER_SIZE,
                       data,
                       realLen);

                data += realLen;

                send_len = RTP_HEADER_SIZE + FU_INDICATOR_SIZE + FU_HEADER_SIZE + realLen;

                udp_send(fd,send_data,send_len);
            }

        }
        usleep(10);
        time_stamp += (90000 / frameRate);
    }
    return 0;
}
