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
const char* INDEX_HOST =  "192.168.0.100";
const int INDEX_PORT = 15000;
const char* LOCAL_FILE = "Public/";
const char* LIST_FILE = "index.txt";
const int MAX_CONNECTING_CLIENTS = 5;
const char* CLIENT_NAME = "Client number ";
const char* SYNREQ = "REQ_TO_SYNC";
const int SYNREQ_SIZE = 12;
const int DOWNREQ_SIZE = 12;
const char* DOWNREQ = "REQ_TO_DOWN"; 
const int SYN_TIME = 60 ;


void *handleIncomingFileTransfer(void * socketInfo);
void *synchronizeFolder();
void *downloadFile();



int main(int argc, char *argv[])
{
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
    if(fileTransferSocket < 0 )
    {
        printf("Error creating socket %d \n ", BIND_PORT_CLIENT_1);
    }
    int enable = 1;
    if (setsockopt(fileTransferSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0)
    	perror("setsockopt(SO_REUSEADDR) failed");
    if (setsockopt(fileTransferSocket, SOL_SOCKET, SO_REUSEPORT, (const char*)&enable, sizeof(int)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");

    //bind socket to all local interfaces 
    bind(fileTransferSocket,(struct sockaddr *)&thisHost, sizeof(thisHost));
    if(listen(fileTransferSocket,MAX_CONNECTING_CLIENTS) == 0)
    {
        char hostName[20];
        //strcpy(hostName,CLIENT_NAME);
        //int hostNameLength;
        if(gethostname(hostName,sizeof(hostName)))
        {
            //printf("Get host name successfully  \n ");
        }
        hostName[20] = '0';
        printf("Client %s  : UP and RUNNING ! \n Listening on %d \n",hostName,BIND_PORT_CLIENT_1);
    }
    else
        printf("Error on listening \n");


    // Run backgroud Synchronize 
    pthread_create(&synchronizeThread,NULL,&synchronizeFolder,NULL);
    // RUn background client waiting for dowloading data 
    pthread_create(&downloadThread,NULL,&downloadFile,NULL);



    //Server preparation 
    addr_size = sizeof(requestingHost);
    // Going live as a server
    while(1)
    {
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
int connectToServerFunction(int* socketToUpdate,char* serverAddress,int port)
{
    struct sockaddr_in indexServerAddr; 
    socklen_t server_address_size;
    
    int connectStatus;
    // allocation of socket 
    // create a socket Ipv4, TCP , TCP'S protocol
    *socketToUpdate = socket(AF_INET,SOCK_STREAM,0);
    if(*socketToUpdate < 0 )
    {
        printf("Socket Creation Error \n");
        exit(0);
    }
    // Create target server IPv4, TCP, PORT ,IP ADDRESS     
    indexServerAddr.sin_family = AF_INET;
    indexServerAddr.sin_port = htons(port);
    indexServerAddr.sin_addr.s_addr = inet_addr(serverAddress);
    server_address_size = sizeof(indexServerAddr);
    // Connect to server
    connectStatus = connect(*socketToUpdate,(struct sockaddr *)&indexServerAddr,server_address_size);
    return connectStatus;
}
void *handleIncomingFileTransfer(void *socketInfo)
{
    pthread_detach(pthread_self());
    printf("Thread created id %ld for handling requesting data\n \n",pthread_self());
    return NULL;
}

void *synchronizeFolder()
{
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
    while(1)
    {
        //continous updates of file    
        // update every minute 
        if(time_taken > SYN_TIME*2 )
        {
            time1 = clock();
            printf("Updating ..%d\n",++updateCount);
            // update file list into index file
            f = fopen(LIST_FILE,"w");
            if(f == NULL){
                printf("Error opening file");
            }
            // Open folder
            pDir = opendir (LOCAL_FILE);
            if (pDir == NULL) {
                printf ("Cannot open directory '%s'\n", LOCAL_FILE);
                return NULL;
            }
            //getcwd(cwd, sizeof(cwd));
            // printf("Current working directory %s/%s\n",cwd,LOCAL_FILE);
            // Listing files 
            while ((pDirent = readdir(pDir)) != NULL) {
                if((strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0 || (*pDirent->d_name) == '.' )){
                    continue;
                }else{
                    fprintf(f,"%s\n",pDirent->d_name);
                }   
            }
            //Connect and update index file to server
            int socketToUpdate;
            if( connectToServerFunction(&socketToUpdate,INDEX_HOST,INDEX_PORT) < 0){
                printf("Update stopped . Server couldnt respond \n");
                return;
            }
            else {
                printf("Updating to Server in process \n");
            }
             printf("%d**\n",write(socketToUpdate,SYNREQ,SYNREQ_SIZE));
            
            close(socketToUpdate);
            fclose(f);
            closedir (pDir);    
            
        }
        time2 = clock();
        time_taken = ((double)(time2 - time1))/CLOCKS_PER_SEC; 
    }
    //END 
    
    return NULL;

}
void *downloadFile()
{
    pthread_detach(pthread_self());
    printf("Thread created id %ld for downloading data\n \n",pthread_self());
    int socketToDownload;
    struct sockaddr_in indexServerAddr; 
    socklen_t server_address_size;
    int connectStatus;
    if(connectToServerFunction(&socketToDownload,INDEX_HOST,INDEX_PORT) < 0 ){
        printf("Connect failed \n");
        return ;
    } else {
         printf("IndexServer Connected\n");
    }   
   // int size = sizeof(DOWNREQ)/sizeof(DOWNREQ[0]);
    printf("%d**",write(socketToDownload,DOWNREQ,DOWNREQ_SIZE));
    time_t time1 = clock();
    time_t time2 = clock();
    //char buffer[15];
    char buffer[50];
    bzero(buffer, sizeof(buffer));
    int i;

    while(1)
    {
        
        printf("Ready Upload list to Server, enter code \n");
        scanf("%d",&i);
        write(socketToDownload,&i,sizeof(i));
        //write(socketToDownload,"Hello \n",50);
        //printf("%d@@@@",sendFile(LIST_FILE, socketToDownload));
       
    }
    /*char *addr;
    int port;
    
    port = getPeerAddr("test.txt",&addr);
    printf("\n%s", addr);
    printf("\n%d", port);*/
    return NULL;
}


int sendFile(char* fileName, int socket) // has sent file_size b4
{
    int size = 0, maxTransUnit = 1240;
    char segment[1240] = {0};

    int totalSize = 0;
    FILE *file = fopen(fileName, "r");
    if(file == NULL) 
    {
        char cwd[100];      
        if (getcwd(cwd, sizeof(cwd)) != NULL) 
        {
            //printf("Current working dir: %s\n", cwd);
        } else 
        {
            //perror(" dkm xa hoigetcwd() error");
            return 0;
        }
        //perror(" fopen ");
        printf(" \t \t \t File not found %ld : %s !! \n \n \n",sizeof(fileName)/sizeof(char),fileName);
        totalSize = 0;
        write(socket, &totalSize, sizeof(totalSize));
        return 0;     
    }
    else 
    {
        fseek(file, 0L, SEEK_END);
        totalSize = ftell(file);
        //write(socket, &totalSize, sizeof(totalSize));
        fclose(file);
        if (totalSize > 0)
        {
            file = fopen(fileName, "r");
            while (sizeof(segment) <= maxTransUnit)
            {
                maxTransUnit = fread(segment, 1, 1240, file);
                segment[maxTransUnit] = 0;
                write(socket, segment, maxTransUnit);
                size += maxTransUnit;
            }
            printf("\t \t \t Sent %s  ! \n   ",fileName); 
            fclose(file);
            return 1;
        }
    }
    return 1;
}
int receiveFile(char* fileName,int file_size, int socket){
	clock_t time = 0;
	int maxTransUnit = 1240;
	int size = 0, totalSize = 0;
    char segment[1240] = {0};
	char str[80];
	//fileName[0] = 'R';
	totalSize = file_size;
    //read(socket, &totalSize, sizeof(totalSize));
    if(totalSize <= 0) {
        printf("Partner response with file size = 0 \n");
        return 0;
    } else {
		
        FILE *file = fopen(fileName, "w");
		time = clock();
		//printf(" File %s is opened for writing %d bytes \n",fileName,totalSize);
        while(sizeof(segment) <= maxTransUnit) {
            maxTransUnit = read(socket, segment, sizeof(segment));
            segment[maxTransUnit] = 0;
            fwrite(segment, 1, maxTransUnit, file);
            size += maxTransUnit;
        }
		time  = clock() - time;
		double time_taken = ((double)time)/CLOCKS_PER_SEC;
        printf("Received %d bytes in %lf seconds \n\n\n\n",size, time_taken);
        fclose(file);
        return 1;
    }
}
int getPeerAddr(char *peerHasFile, char **addr)
{
    char *portChar;
    int port=0;
    FILE *fp = fopen(peerHasFile, "r");
    if(fp == NULL)
    {
        printf("fopen failed\n\n");
    }
    char * getFirstPeerAddr = NULL;

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if((read = getline(&line, &len, fp)) != -1)
    {
        //printf("%s", line);
    }

    *addr = strtok(line, ":");
    portChar = strtok(NULL, ":");
    portChar = strtok(portChar, ".");
    port = atoi(portChar);
    return port;

}


