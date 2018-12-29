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




const int BIND_PORT_CLIENT_1 = 11000;
const int DEFAULT_SIZE = 1024;
const int DEFAULT_LENGTH = 20;
const char* INDEX_HOST =  "127.0.0.1";
const int INDEX_PORT = 15000;
const char* LOCAL_FILE = "Public/";
const char* LIST_FILE = "index.txt";
const int MAX_CONNECTING_CLIENTS = 5;
const char* CLIENT_NAME = "Client number ";
const int SYN_TIME = 60 ;


void *handleIncomingFileTransfer(void * socketInfo);
void *synchronizeFolder();
void *downloadFile();



int main(int argc, char *argv[]){
    // Threading info
    pthread_t synchronizeThread;
    pthread_t fileTransferThread;
    pthread_t downloadThread;

    // Clear sysout buffer
    setvbuf (stdout, NULL, _IONBF, 0);
    // Binding info declaration
    int fileTransferSocket;
    int buffer[DEFAULT_SIZE];
    struct sockaddr_in thisHost;
    struct sockaddr_in requestingHost;
    socklen_t addr_size ;
    int *transferSocket;

    //Address binding preparation

    thisHost.sin_family =  AF_INET;
    thisHost.sin_port = BIND_PORT_CLIENT_1;
    thisHost.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(thisHost.sin_zero,'\0',sizeof(thisHost.sin_zero));
    fileTransferSocket = socket(AF_INET,SOCK_STREAM,0);
    if(fileTransferSocket < 0 ){
        printf("Error creating socket %d \n ", BIND_PORT_CLIENT_1);
    }
    int enable = 1;
    if (setsockopt(fileTransferSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0)
    	perror("setsockopt(SO_REUSEADDR) failed");
    if (setsockopt(fileTransferSocket, SOL_SOCKET, SO_REUSEPORT, (const char*)&enable, sizeof(int)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");

    //bind socket to all local interfaces 
    bind(fileTransferSocket,(struct sockaddr *)&thisHost, sizeof(thisHost));
    if(listen(fileTransferSocket,MAX_CONNECTING_CLIENTS) == 0){
        char hostName[20];
        //strcpy(hostName,CLIENT_NAME);
        //int hostNameLength;
        if(gethostname(hostName,sizeof(hostName))){
            //printf("Get host name successfully  \n ");
        }
        hostName[20] = '0';
        printf("Client %s  : UP and RUNNING ! \n Listening on %d \n",hostName,BIND_PORT_CLIENT_1);
    }
    else {
        printf("Error on listening \n");
    }


    // Run backgroud Synchronize 
    pthread_create(&synchronizeThread,NULL,&synchronizeFolder,NULL);
    // RUn background client waiting for dowloading data 
    pthread_create(&downloadThread,NULL,&downloadFile,NULL);



    //Server preparation 
    addr_size = sizeof(requestingHost);
    // Going live as a server
    while(1){
        transferSocket = malloc(sizeof(int));
        *transferSocket = accept(fileTransferSocket,(struct sockaddr*)&requestingHost,&addr_size);
        if (errno == EINTR) continue;
            else;// perror("accept error");
        pthread_create(&fileTransferThread, NULL ,&handleIncomingFileTransfer,(void *)transferSocket);

    }





    
    /*
    //variable define 
    int* updateSocket;
    struct sockaddr_in indexServerAddr;
    //operate as a server 


    // setup Transfer socket
    if (connectToServer(updateSocket,indexServerAddr) < 0 ) {
        printf("Connect failed \n");
        exit(0);
    }
    else { // connected Successfully
        // get file list
        // update file index.txt
        //send file over to index server 
    }
    // create a listening port ready for file transfer
     while(1){

     }
    
    */
    return 0;
}
int connectToServer(int* socketToUpdate,struct sockaddr_in indexServerAddr){
   
    socklen_t server_address_size;
    int connectStatus;
    // allocation of socket 

    socketToUpdate = malloc(sizeof(int));
    // create a socket Ipv4, TCP , TCP'S protocol
    *socketToUpdate = socket(AF_INET,SOCK_STREAM,0);
    if(*socketToUpdate < 0 )
        {
            printf("Socket Creation Error \n");
            exit(0);
        }
    // Create target server IPv4, TCP, PORT ,IP ADDRESS     
    indexServerAddr.sin_family = AF_INET;
    indexServerAddr.sin_port = htons(INDEX_PORT);
    indexServerAddr.sin_addr.s_addr = inet_addr(INDEX_HOST);
    server_address_size = sizeof(indexServerAddr);
    // Connect to server
    connectStatus = connect(*socketToUpdate,(struct sockaddr *)&indexServerAddr,server_address_size);
    return connectStatus;
}
void *handleIncomingFileTransfer(void *socketInfo){
    pthread_detach(pthread_self());
    printf("Thread created id %ld for handling requesting data\n \n",pthread_self());
    return NULL;
}
void *synchronizeFolder(){
    pthread_detach(pthread_self());
    printf("Thread created id %ld for synchronizing data\n \n",pthread_self());
    struct dirent *pDirent;
    DIR *pDir;
    char cwd[100]; 
    FILE *f;
    clock_t time1 = clock();
    clock_t time2 = clock();
    double time_taken = 121;
    int updateCount = 0;
    // open Index file
    while(1){
        //continous updates of file    
        
       
        // update every minute 
        if(time_taken > SYN_TIME*2 ){

            time1 = clock();
            printf("Updating ..%d\n",++updateCount);
            
            // update file list into index file
            f = fopen(LIST_FILE,"w");
            if(f == NULL)
            {
                printf("Error opening file");
            }
            // Open folder
            pDir = opendir (LOCAL_FILE);
            if (pDir == NULL) {
                printf ("Cannot open directory '%s'\n", LOCAL_FILE);
                return NULL;
            }
            getcwd(cwd, sizeof(cwd));
           // printf("Current working directory %s/%s\n",cwd,LOCAL_FILE);
            // Listing files 
            while ((pDirent = readdir(pDir)) != NULL) {
                if((strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0 || (*pDirent->d_name) == '.' )){
                }
                else{
                        //printf ("%s\n", (*pDirent).d_name);
                        fprintf(f,"%s\n",pDirent->d_name);
                } 
            }
            fclose(f);
            closedir (pDir);    
            
        }
        time2 = clock();
        time_taken = ((double)(time2 - time1))/CLOCKS_PER_SEC; 
    }
    //END 
    
    return NULL;

}
void *downloadFile(){
    pthread_detach(pthread_self());
    printf("Thread created id %ld for downloading data\n \n",pthread_self());
    int socketToDownload;
    struct sockaddr_in indexServerAddr; 
    socklen_t server_address_size;
    int connectStatus;
    // allocation of socket 

    // create a socket Ipv4, TCP , TCP'S protocol
    socketToDownload = socket(AF_INET,SOCK_STREAM,0);
    if(socketToDownload < 0 )
        {
            printf("Socket Creation Error \n");
            exit(0);
        }
    // Create target server IPv4, TCP, PORT ,IP ADDRESS     
    indexServerAddr.sin_family = AF_INET;
    indexServerAddr.sin_port = htons(INDEX_PORT);
    indexServerAddr.sin_addr.s_addr = inet_addr(INDEX_HOST);
    server_address_size = sizeof(indexServerAddr);
    // Connect to server
    connectStatus = connect(socketToDownload,(struct sockaddr *)&indexServerAddr,server_address_size);
    if(connectStatus <0 ){
        printf("Connect failed \n");
    }
    else {
        printf("IndexServer Connected\n");
    }
    time_t time1 = clock();
    time_t time2 = clock();
    while(1){
        int i;
        printf("Nhap code \n");
        char buffer[15];
        scanf("%d",&i);
        int port;
        int sentBytes = send(socketToDownload,&i,sizeof(i),0);
        int getRepliesResult = recv(socketToDownload,buffer,sizeof(buffer),0);
        int getRepliesPort = recv(socketToDownload,&port,sizeof(port),0);
        printf("%s:Hello:port:%d\n",buffer,port);
        printf("Ready to connect to new host %s %d\n`",buffer,port);
        /*
        time2 = clock();{
            if(time2 -time1 > (10*CLOCKS_PER_SEC))
            {
                time1 = clock();
                printf("Doctor \n");
            }
        }*/
    }
    return NULL;

}



