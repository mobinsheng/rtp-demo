#include "h264_nal.h"
#include <memory.h>

static int h264_get_nal(const unsigned char* buf,const unsigned int size,h264_nalu* nalu,bool lastOne = false);
/**
 * 查找startcode
 */
static int FindStartCode3 (const unsigned char *Buf){
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //0x000001?
    else return 1;
}

/**
 * 查找startcode，
 */
static int FindStartCode4 (const unsigned char *Buf){
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//0x00000001?
    else return 1;
}

int NaluStartCodeLen(const unsigned char* pucBuf){
    int startCode = 3;
    if(pucBuf[0] == 0 && pucBuf[1] == 0 &&pucBuf[2] == 0 && pucBuf[3] == 1){
        startCode = 4;
    }
    else if(pucBuf[0] == 0 && pucBuf[1] == 0 &&pucBuf[2] == 1 ){
        startCode = 3;
    }
    else{
        startCode = 0;
    }
    return startCode;
}

bool H264Reader::ReadNalu(uint8_t *nal, int *len){
    h264_nalu nalu;
    bool ret = ReadNalu(&nalu);
    if(ret == false){
        return false;
    }
    *len = nalu.nal_len;
    memcpy(nal,nalu.buf,nalu.nal_len);
    return true;
}

bool H264Reader::ReadNalu(h264_nalu *nalu){
    bool eof = true;

    while (true) {
        if(ReadDataFromFile()){
            eof = false;
        }

        size_t dataLen = end - start;

        // 文件已经读完，buffer也没有数据，那么返回false
        if(eof && dataLen == 0){
            return false;
        }

        int ret = h264_get_nal(start,dataLen,nalu);

        // 读取一个nalu失败，可能是数据不够造成的
        if(ret == 0){
            // 判断一下是不是已经到文件末尾了，那么剩下的数据全部作为一个nalu处理
            if(eof){
                ret = h264_get_nal(start,(end - start),nalu,true);
                start = end;
                if(ret == 0){
                    return false;
                }
                else{
                    return true;
                }
            }
            else{
                // 文件中还有数据，那么继续把数据读取到buffer中
                continue;
            }
        }
        else{
            // 读取成功
            start += ret;
            return true;
        }
    }
}

int H264Reader::ReadDataFromFile(){
    if(feof(fp_in)){
        return 0;
    }

    // 如果end位置已经指向buffer末尾，那么把数据往前移动
    if(end == buf + capacity){
        size_t dataLen = end - start;
        memmove(buf,start,dataLen);
        start = buf;
        end = start + dataLen;
    }

    // buffer剩余的空间
    size_t leftLen = capacity - (end - start);

    // 如果buffer剩余的空间大于4096,那么尝试读取4096字节的数据
    // 否则就把buffer剩下的空间读满
    size_t tryReadLen = leftLen;
    if(leftLen > 1024 * 4){
        tryReadLen = 1024 * 4;
    }
    else{
        tryReadLen = leftLen;
    }

    // 实际读取到的数据长度
    int realReadLen = fread(end,1,tryReadLen,fp_in);
    if(realReadLen < 0){
        return -1;
    }

    end += realReadLen;

    return realReadLen;

}

void H264Reader_Test(const char* srcFileName,const char* dstFileName){
    FILE* fp_src = fopen(srcFileName,"rb");
    FILE* fp_dst = fopen(dstFileName,"wb");
    H264Reader reader(fp_src);
    h264_nalu nalu;
    while (reader.ReadNalu(&nalu)) {
        if(nalu.start_code_len == 3){
            fputc(0,fp_dst);
            fputc(0,fp_dst);
            fputc(1,fp_dst);
        }
        else{
            fputc(0,fp_dst);
            fputc(0,fp_dst);
            fputc(0,fp_dst);
            fputc(1,fp_dst);
        }
        fwrite(nalu.buf,1,nalu.nal_len,fp_dst);
    }
    fflush(fp_dst);
    fclose(fp_dst);
    fclose(fp_src);
}

static int h264_get_nal(const unsigned char* buf,const unsigned int size,h264_nalu* nalu,bool lastOne){
    int i = 0;
    while(i<size)
    {
        // 寻找nalu的startcode
        int startCodeLen = 0;
        if((i + 2 < size) && FindStartCode3(&buf[i]) ){
            startCodeLen = 3;
        }
        else if((i + 3 < size) && FindStartCode4(&buf[i])){
            startCodeLen = 4;
        }
        else{
            ++i;
            continue;
        }
        // 此时i指向startcode的第一个字节

        nalu->start_code_len = startCodeLen;

        // 此时i指向nalu的头部
        i += startCodeLen;

        // begin指向nalu的头部
        int begin = i;
        int end = i;
        int nextStartCodeLen = 0;

        // 如果不是最后一个nalu，那么需要找到下一个nalu的startcode才能确定nalu的长度
        if(lastOne == false){

            //  找到下一个nalu的startcode，如果找到，那么end指向下一个nalu的startcode
            while (end<size){
                nextStartCodeLen = 0;
                if((end + 2 < size) && FindStartCode3(&buf[end]) ){
                    nextStartCodeLen = 3;
                    break;
                }
                else if((end + 3 < size) && FindStartCode4(&buf[end])){
                    nextStartCodeLen = 4;
                    break;
                }
                else{
                    ++end;
                    continue;
                }
            }

            // 如果没有找到，表示buf中的数据不够（因为已经明确指示这不是最后一个nalu），返回false
            if(nextStartCodeLen == 0){
                return 0;
            }
        }
        else{
            // 如果是最后一个nalu，那么剩下的数据全部属于一个nalu
            end = size;
        }


        int dataLen = end- begin;
        nalu->nal_unit_type = ( EnNaluType)(buf[begin]&0x1f);
        nalu->nal_ref_idc = ( EnNaluPriority)(buf[begin] & 0x60);
        nalu->nal_header = buf[begin];

        nalu->nal_len = dataLen;
        memcpy(nalu->buf,&buf[begin],nalu->nal_len);

        return (end);
    }
    return 0;
}
