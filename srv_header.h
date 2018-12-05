#ifndef SRV_HEADER_H
#define SRV_HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFSIZE 10000
#define MAX_CLNT 100
#define HOSTNAMELEN 100

typedef int64_t S64;
typedef int32_t S32;
typedef int16_t S16;
typedef int8_t S8;

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

void * handle_clnt(void * arg);
void send_msg(U8 * msg, U32 len, U32 clnt_sock, U8 * hostname);
void rcvMsg(void * sock);
U8 isHttp(unsigned char * msg, U32 len, U8 * hostname);

pthread_mutex_t mutx;


U8 isHttp(unsigned char * msg, U32 len, U8 * hostname){
    U32 i;
    U8 flag = 0;
    for(i = 0; i<len - 6; i++){
        if(!strncmp(msg + i, "Host: ", 6)){
            U32 hostIdx  = i + 6, hostLen = 0;

            while(*(msg+hostIdx) != '\x0d'){
                hostname[hostLen++] = *(msg+hostIdx);
                hostIdx++;
            }
            hostname[hostLen]='\0';
            flag = 1;
        }
    }

    return flag;
}

void printError(U8 * errstr){
    fprintf(stderr, "%s\n", errstr);
    exit(1);
}

#endif // SRV_HEADER_H

