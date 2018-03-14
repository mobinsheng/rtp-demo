#ifndef H264_NAL_H
#define H264_NAL_H

#include <stdio.h>
#include <stdint.h>

typedef enum {
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12,
} EnNaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIRITY_LOW         = 1,
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_HIGHEST    = 3
} EnNaluPriority;

struct h264_nalu {
    h264_nalu(){
        buf = new unsigned char[1024 * 1024]; /// 有可能会不够大！对于高清视频，nalu可能非常大
        start_code_len = 0;
        nal_len = 0;
        nal_header = 0;
    }
    ~h264_nalu(){
        delete[] buf;
    }

    int start_code_len; ///< startcode 的长度
    int nal_len; ///< nalu的长度（除去startcode之后）
    unsigned char nal_header; ///< nalu的头部（只有一个字节）
    EnNaluPriority nal_ref_idc; ///< 优先级
    EnNaluType nal_unit_type;///< 类型
    unsigned char* buf; ///nalu header + nalu payload
} ;

struct H264Reader{
    H264Reader(FILE* fp){
        buf = 0;
        capacity = 1024 * 1024 * 4;
        buf = new uint8_t[capacity];

        fp_in = fp;
        start = buf;
        end = buf;
    }

    ~H264Reader(){
        delete[] buf;
    }

    // 读取一个nalu
    bool ReadNalu(uint8_t *nal, int *len);
    bool ReadNalu(h264_nalu* nal);
private:
    int ReadDataFromFile();
    // 缓冲区
    uint8_t* buf;
    // 缓冲区的容量
    size_t capacity;
    // 指向有效数据的起始位置（第一个字节）
    uint8_t* start;
    // 指向有效数据的下一个位置（最后一个有效数据的下一个字节）
    uint8_t* end;
    FILE* fp_in;
};

void H264Reader_Test(const char* srcFileName,const char* dstFileName);
#endif // H264_NAL_H
