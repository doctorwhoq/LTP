#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <pthread.h>
#include <sys/select.h>
#include <time.h>
#include <dirent.h>

const int INDEX_PORT = 15000;
const char* INDEX_HOST = "192.168.0.100";
const int DEFAULT_SIZE = 1024;
const int DEFAULT_LENGTH = 20;
const int MAX_CLIENTS = 5;
void * handleSynThread(void *);
void * handleReqThread(void *);

int main(){
    setvbuf (stdout, NULL, _IONBF, 0);
    int listenSock;
    int numberOfClient;
    struct sockaddr_in indexHost;
    struct sockaddr_in clientAddr;
    socklen_t addr_size;


    addr_size = sizeof(indexHost);

    pthread_t synThread;
    pthread_t reqThread;

    listenSock = socket(AF_INET,SOCK_STREAM,0);
    int enable = 1;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0)
    	perror("setsockopt(SO_REUSEADDR) failed");
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEPORT, (const char*)&enable, sizeof(int)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");
    if(listenSock < 0){
        printf("Create socket failed \n");
    }
    indexHost.sin_family = AF_INET;
    indexHost.sin_port = htons(INDEX_PORT);
    indexHost.sin_addr.s_addr =htonl(INADDR_ANY);;
    memset(indexHost.sin_zero, '\0', sizeof indexHost.sin_zero);  
    bind(listenSock, (struct sockaddr *) &indexHost, sizeof(indexHost));
    if(listen(listenSock, MAX_CLIENTS) == 0){
        printf("Index Host running at : %s port %d \n",INDEX_HOST,INDEX_PORT);
    }
    else {
        printf("Listening failed, server stopped \n");
    }
    int* acceptedSocket;
     /* accepting new connections*/
    while(1){

        acceptedSocket = malloc(sizeof(int));
        *acceptedSocket = accept(listenSock, (struct sockaddr *) &clientAddr, &addr_size);
        if (errno == EINTR) continue;
            else;// perror("accept error");
        printf("New connection accepted\n");    
        if(1){
            pthread_create(&synThread, NULL ,&handleSynThread,(void *)acceptedSocket);
        }
        else{
            pthread_create(&reqThread, NULL ,&handleReqThread,(void *)acceptedSocket);
        }    
        
        
    }
    return 0;
}
void * handleSynThread(void *socketInfo){
    printf("Handle Syn Thread ");
    int i;
    //printf("haha");
    int socketId = *((int *)socketInfo);
    int readBytes = recv(socketId,&i,sizeof(i),0);
    char* foundAddresses = "127.0.0.13";
    int foundPort = 1983;
    int repliesResult = send(socketId,foundAddresses,sizeof(foundAddresses) + 2,0);// get all the lteer
    int repliesResult2 = send(socketId,&foundPort,sizeof(foundPort),0);
    printf(" INT recevied is %d",i);

    return NULL;
}
void * handleReqThread(void *socketInfo){
    printf("Handle Req Thread");
    return NULL;
}