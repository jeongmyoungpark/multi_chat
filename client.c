#include<stdio.h>

#include<stdlib.h>

#include<arpa/inet.h>

#include<sys/socket.h>

#include<unistd.h>

#include<string.h>

#include <pthread.h>

#include <mongoc.h>



#define NAME_SIZE 20

# define BUFSIZE 100



void error_handle(char *message);

void * send_msg(void * arg);

void * recv_msg(void * arg);

char name[NAME_SIZE]="[DEFAULT]";

char msg[BUFSIZE];



int main(int argc, char* argv[]){

    mongoc_init ();

    int client_socket;

    pthread_t snd_thread,rcv_thread;

    void * thread_return;

    struct sockaddr_in serv_addr;

    if(argc != 4)

    {

        printf("%s <IP> <PORT> <NAME>\n", argv[0]);

        exit(1);

    }

    sprintf(name,"[%s]",argv[3]);

    client_socket = socket(PF_INET,SOCK_STREAM,0);



    if(client_socket==-1){

        error_handle("socket() error");

    }



    memset(&serv_addr, 0 , sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);

    serv_addr.sin_port = htons(atoi(argv[2]));



    if(connect(client_socket, (struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){

        error_handle("connect() error");

    }



    printf("채팅 시작했습니다\n");



    pthread_create(&snd_thread,NULL,send_msg,(void*)&client_socket);

    pthread_create(&snd_thread,NULL,recv_msg,(void*)&client_socket);

    pthread_join(snd_thread, &thread_return);

    pthread_join(rcv_thread, &thread_return);

    close(client_socket);

    return 0;

}



void * send_msg(void * arg){

    int sock = *((int*)arg);

    char name_msg[NAME_SIZE+BUFSIZE];

    while (1)

    {

        fgets(msg,BUFSIZE,stdin);

        if(strcmp(msg,"/quit\n")==0){//문자열이 같은지 비교

           close(sock);

           //exit(0);

           return 0;

        }

        else{

            sprintf(name_msg,"%s %s",name,msg);

            write(sock,name_msg,strlen(name_msg));

        }

    }

    return NULL;

}

void * recv_msg(void * arg){

    int sock = *((int*)arg);

    char name_msg[NAME_SIZE+BUFSIZE];

    int str_len;

    while (1)

    {

        str_len=read(sock,name_msg,NAME_SIZE+BUFSIZE-1);

        if(str_len==-1){

            return (void*)-1;

        }

        name_msg[str_len]=0;

        fputs(name_msg,stdout);

    }

    return NULL;

}

void error_handle(char *message){

    fputs(message, stderr);

    fputc('\n', stderr);

    exit(1);

}

