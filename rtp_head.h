#ifndef RTP_HEAD_H
#define RTP_HEAD_H

#include <stdint.h>

#define NAL_MAX_SIZE                              4000000 /// NALU的最大长度
#define RTP_PAYLOAD_MAX_SIZE             1400 /// RTP包载荷的最大长度，载荷加上头部应该小于MTU，这样才传输的过程中才不会被分包
#define SSRC_NUM                                     10 /// 同步信源标识符，可以随机选择，不同的会议参与者，不能有相同的SSRC
#define RTP_HEADER_SIZE                        12 /// RTP固定头部的长度（字节）
#define FU_INDICATOR_SIZE                     1
#define FU_HEADER_SIZE                          1
#define CSRC_ITEM_SIZE                           4
#define CSRC_ITEM_MAX_NUM                 16

#define RTP_STANDARD //RTP标准字节顺序

enum MediaType{
    G711_Type = 8, /// AUDIO
    H264_Type = 96, /// VIDEO
};

typedef struct rtp_header {
#ifdef RTP_STANDARD
    /* big-endian */
    /* byte 0 */
    uint8_t version:        2;  /* bit: 0~1 */
    uint8_t padding:        1;  /* bit: 2*/
    uint8_t extension:      1;  /* bit: 3 */
    uint8_t csrc_len:       4;  /* bit: 4~7 */

    /* byte 1 */
    uint8_t marker:         1;  /* bit: 0 */
    uint8_t payload_type:   7;  /* bit: 1~7 */

    /* bytes 2, 3 */
    uint16_t seq_no;
    /* bytes 4-7 */
    uint32_t timestamp;
    /* bytes 8-11 */
    uint32_t ssrc;
#else
    /* little-endian */
    /* byte 0 */
    uint8_t csrc_len:       4;  /* bit: 0~3 */
    uint8_t extension:      1;  /* bit: 4 */
    uint8_t padding:        1;  /* bit: 5*/
    uint8_t version:        2;  /* bit: 6~7 */
    /* byte 1 */
    uint8_t payload_type:   7;  /* bit: 0~6 */
    uint8_t marker:         1;  /* bit: 7 */
    /* bytes 2, 3 */
    uint16_t seq_no;
    /* bytes 4-7 */
    uint32_t timestamp;
    /* bytes 8-11 */
    uint32_t ssrc;
#endif

} __attribute__ ((packed)) rtp_header_t; /* 12 bytes */

typedef struct nalu_header {
#ifdef RTP_STANDARD
    /* byte 0 */
    uint8_t f:      1;  /* bit: 0 */
    uint8_t nri:    2;  /* bit: 1~2 */
    uint8_t type:   5;  /* bit: 3~7 */
#else
    /* byte 0 */
    uint8_t type:   5;  /* bit: 0~4 */
    uint8_t nri:    2;  /* bit: 5~6 */
    uint8_t f:      1;  /* bit: 7 */
#endif
} __attribute__ ((packed)) nalu_header_t; /* 1 bytes */

typedef struct nalu {
    int startcodeprefix_len;
    unsigned len;             /* Length of the NAL unit (Excluding the start code, which does not belong to the NALU) */
    unsigned max_size;        /* Nal Unit Buffer size */
    int forbidden_bit;        /* should be always FALSE */
    int nal_reference_idc;    /* NALU_PRIORITY_xxxx */
    int nal_unit_type;        /* NALU_TYPE_xxxx */
    char *buf;                /* contains the first byte followed by the EBSP */
    unsigned short lost_packets;  /* true, if packet loss is detected */
} nalu_t;


typedef struct fu_indicator {
#ifdef RTP_STANDARD
    /* byte 0 */
    uint8_t f:      1;
    uint8_t nri:    2;
    uint8_t type:   5;
#else
    /* byte 0 */
    uint8_t type:   5;
    uint8_t nri:    2;
    uint8_t f:      1;
#endif

} __attribute__ ((packed)) fu_indicator_t; /* 1 bytes */

typedef struct fu_header {
#ifdef RTP_STANDARD
    /* byte 0 */
    uint8_t s:      1;
    uint8_t e:      1;
    uint8_t r:      1;
    uint8_t type:   5;
#else
    /* byte 0 */
    uint8_t type:   5;
    uint8_t r:      1;
    uint8_t e:      1;
    uint8_t s:      1;
#endif
} __attribute__ ((packed)) fu_header_t; /* 1 bytes */

typedef struct rtp_package {
    rtp_header_t rtp_package_header;
    uint8_t *rtp_load;
} rtp_t;

#endif // RTP_HEAD_H
