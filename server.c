#include <stdio.h>

#include <sys/socket.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <pthread.h>

#include <mongoc.h>

#define SERVER          "mongodb://localhost:27017/"



#define BUFSIZE 1024

#define MAX_CLNT 256



void error_handle(char * msg);

void * handle_clt(void * arg);

void send_msg(char * msg, int len, int sock);



int clnt_cnt=0;

int clnt_socks[MAX_CLNT];

pthread_mutex_t mutx;



int main(int argc, char* argv[]){

    int serv_sock;

    int client_sock;

    int str_len;

    char message[BUFSIZE];

 



    struct sockaddr_in serv_addr;

    struct sockaddr_in client_addr;

    int client_addr_size;

    pthread_t t_id;

    

    if(argc != 2)  

    {

        printf("%s <port>\n", argv[0]);

        exit(1);

    }



    pthread_mutex_init(&mutx,NULL);//mutex 초기화

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);



    if(serv_sock==-1){

        error_handle("socket() error");

    }



    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family=AF_INET;

    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    serv_addr.sin_port = htons(atoi(argv[1]));



    if(bind(serv_sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){

        error_handle("bind() error");

    }

    

    if(listen(serv_sock,5)==-1){

        error_handle("listen() error\n");

    }

    while (1)

    {

        client_addr_size= sizeof(client_addr);

        client_sock= accept(serv_sock, (struct sockaddr*)&client_addr , &client_addr_size);

        if(client_sock==-1){

            error_handle("accept() error\n");

        }

        pthread_mutex_lock(&mutx); //mutex lock 

        clnt_socks[clnt_cnt++]= client_sock;

        pthread_mutex_unlock(&mutx);



        pthread_create(&t_id,NULL,handle_clt,(void*)&client_sock);

        pthread_detach(t_id);

        printf("connetted ip : %s \n",inet_ntoa(client_addr.sin_addr));



    }

    close(serv_sock);

    return 0;

}



void * handle_clt(void *arg){

    int client_sock =*((int*)arg);

    int str_len = 0, i;

    char msg[BUFSIZE];



 

    while ((str_len=read(client_sock,msg,sizeof(msg)))!=0)

    {

        send_msg(msg,str_len,client_sock);

    }



    pthread_mutex_lock(&mutx);

    for (i = 0; i < clnt_cnt; i++)

    {

        if(client_sock==clnt_socks[i]){

            while (i++<clnt_cnt-1)

            {

                clnt_socks[i]=clnt_socks[i+1];

            }

            break;

        }

    }

    clnt_cnt--;

    pthread_mutex_unlock(&mutx);

    close(client_sock);

    return NULL;

    

}

void send_msg(char *msg,int len,int sock){

    mongoc_uri_t *uri;

    mongoc_client_t *client;

    mongoc_database_t *database;

    mongoc_collection_t *collection;

    bson_t *command, reply, *insert;

    bson_error_t error;

    char *str;

    bool retval;

    bson_oid_t oid;

    bson_t *doc;



    mongoc_init ();



    int i;

    client = mongoc_client_new (SERVER);   



    mongoc_client_set_appname (client, "admin");

    database = mongoc_client_get_database (client, "test");//db name

    collection = mongoc_client_get_collection (client, "test", "coll_name");



    pthread_mutex_lock(&mutx);

    for (i=0; i < clnt_cnt; i++)

    {

        if(clnt_socks[i]!=sock){ //글 보낸 클라한테 보내주지 않는다.

            write(clnt_socks[i], msg,len);



        }

    }

    doc = bson_new ();

    bson_oid_init (&oid, NULL);

    BSON_APPEND_UTF8 (doc, "msg", msg);

   if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {

        fprintf (stderr, "%s\n", error.message);

        }

    pthread_mutex_unlock(&mutx);

}



void error_handle(char *msg){

    fputs(msg, stderr);

    fputc('\n', stderr);

    exit(1);

}

