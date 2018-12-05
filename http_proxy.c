#include "srv_header.h"

U32 clnt_cnt = 0;
U32 clnt_socks[MAX_CLNT];

U32 main(S32 argc, U8 * argv[])
{
    S32 serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    S32 clnt_adr_size;
    pthread_t t_id;

    if(argc!=2){
        printf("Usage : %s <port> [-b]\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL);
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1) printError("bind() error");
    if(listen(serv_sock, 20) == -1) printError("listen() error");

    clnt_adr_size=sizeof(clnt_adr);
    while(1)
    {
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_size);
        printf("Connection Request : %s:%d\n", inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);

    }

    close(serv_sock);
    pthread_mutex_destroy(&mutx);

    return 0;
}

void * handle_clnt(void * sock){
    U32 clnt_sock = *(U32*)sock;
    printf("clnt_sock : %d\n", clnt_sock);
    U32 str_len = 0, i;
    U8 msg[BUFSIZE], hostname[HOSTNAMELEN];

    while((str_len = read(clnt_sock, msg, sizeof(msg))) !=0) {
        msg[str_len] = '\0';
        if(isHttp(msg, str_len, hostname)) {
            send_msg(msg, str_len, clnt_sock, hostname);
            memset(msg, 0, BUFSIZE);
        }
    }

    pthread_mutex_lock(&mutx);

   for(i = 0; i<clnt_cnt; i++){
        if(clnt_sock == clnt_socks[i]){
            while(i++<clnt_cnt - 1) clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;

}
void send_msg(U8 * msg, U32 len, U32 clnt_sock, U8 * hostname){
    S32 sockets[2];
    struct sockaddr_in serv_addr;
    pthread_t rcv_thread;
    struct hostent * tmp = gethostbyname(hostname);
    sockets[1] = clnt_sock;
    sockets[0] = socket(PF_INET, SOCK_STREAM, 0);
    strcpy(hostname, inet_ntoa(*(struct in_addr*)tmp->h_addr_list[0]));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(hostname);
    serv_addr.sin_port=htons(80);

    if(connect(sockets[0], (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)printError("connect() error");

    pthread_create(&rcv_thread, NULL, rcvMsg, (void*)sockets);

    pthread_mutex_lock(&mutx);

    write(sockets[0], msg, len);

    pthread_mutex_unlock(&mutx);
}


void rcvMsg(void* sock){
    S32 toSrvsock= ((U32*)sock)[0];
    S8 fromSrvMsg[BUFSIZE];
    S32 str_len;

    pthread_mutex_lock(&mutx);
    str_len = read(toSrvsock, fromSrvMsg, BUFSIZE);
    if(str_len == 0) return (void*)-1;
    fromSrvMsg[str_len] = '\0';
    write(((U32*)sock)[1], fromSrvMsg, str_len);
    pthread_mutex_unlock(&mutx);

    close(toSrvsock);
}
