#include <stdio.h>
#include <stdlib.h>
#include "z_udp.h"
#include "rtp_head.h"
#include "rtp_receiver.h"
#include "rtp_sender.h"
#include <pthread.h>

//ffmpeg -f x11grab -s xga -r 5  -i :0.0+0+0 -f h264 rtp://233.233.233.223:6666
//ffplay -f h264 rtp://233.233.233.3:6666

#define SERVER_IP "127.0.0.1"//"233.233.233.223"
#define PORT "6666"
FILE* fp_out;
bool is_little_endian(){
    unsigned int a=0x11223344;
    char b;
    b=*(char *)&a;
    if(b==0x11)
    {
        return false;
    }else if(b==0x44)
    {
        return true;
    }
}

static void nalu_process(uint8_t* nalu,size_t len){
    fputc(0, fp_out);
    fputc(0, fp_out);
    fputc(0, fp_out);
    fputc(1, fp_out);
    fwrite(nalu,1,len,fp_out);
    printf("get nalu\n");
}


void* server_thread(void* param){

    fp_out = fopen("/home/mbs/out.h264","wb");

    recv_rtp(SERVER_IP,PORT,nalu_process);

    fflush(fp_out);
    fclose(fp_out);
    return 0;
}

void* client_thread(void* param){
    send_rtp(SERVER_IP,PORT,"/home/mbs/Videos/video.h264");
    return 0;
}

int main(){
    pthread_t h1,h2;
    pthread_create(&h1,NULL,server_thread,NULL);
    pthread_create(&h1,NULL,client_thread,NULL);
    pthread_join(h1,NULL);
    pthread_join(h2,NULL);
    return 0;
}
