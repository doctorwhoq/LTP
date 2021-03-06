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




const int BIND_PORT_CLIENT_3 = 11000;
const int DEFAULT_SIZE = 1024;
const int DEFAULT_NAME_SIZE = 30;
const int DEFAULT_LENGTH = 20;
const int REQ_SIZE = 12;
const int SYN_TIME = 10 ;
const int findFirst = 0;
const int INDEX_PORT = 15000;
const char* INDEX_HOST =  "192.168.43.87";
const char* LOCAL_FILE = "Public/";
const char* LIST_FILE = "index.txt";
const int MAX_CONNECTING_CLIENTS = 5;
const char* CLIENT_NAME = "Client number ";
const char* SEARCH_RES = "SearchResult:";
const char* SYNREQ = "REQ_TO_SYNC";
const char* FOUNDS = "SEA_I_FOUND";
const char* FOUNDN = "SEA_N_FOUND";
const char* LOG = "Logs/";
const char* DOWNREQ = "REQ_TO_DOWN"; 
const char* DEAD = "Peer OFFLINE";
const char* ALIVE = "Peer ONLINE";





void *handleIncomingFileTransfer(void * socketInfo);
void *synchronizeFolder();
void *downloadFile();
int sendFile(const char*,int );
int receiveFile(char*,int);
int getPeerAddr(char*,char**,int);
int getPeersResult(char*);
int checkAlive(char*,int*);





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
    addr_size = sizeof(thisHost);;
    int* transferSocket;

    //Address binding preparation
     fileTransferSocket = socket(AF_INET,SOCK_STREAM,0);
    thisHost.sin_family =  AF_INET;
    thisHost.sin_port = htons(BIND_PORT_CLIENT_3);
    thisHost.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(thisHost.sin_zero,'\0',sizeof(thisHost.sin_zero));

    printf("\n----------Xin chao, day la chuong trinh gui va nhan file giua cac peer----------");
    printf("\n----Chuong trinh se tu dong update danh sach chia se file len server moi 60s----");
   
    if(fileTransferSocket < 0 )
    {
        printf("Error creating socket %d \n ", BIND_PORT_CLIENT_3);
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
        printf("\n-----------------------Client %s  : UP and RUNNING ! -------------------------\n---------------Listening on %d----------------\n\n\n",hostName,BIND_PORT_CLIENT_3);
    }
    else
        printf("Error on listening \n");


    // Run backgroud Synchronize 
    pthread_create(&synchronizeThread,NULL,&synchronizeFolder,NULL);
    sleep(1);
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
int connectToServerFunction(int* socketToUpdate,const char* serverAddress,int port)
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
    if(connectStatus < 0){
        //perror(" Connect unsuccessful ");
    }
    return connectStatus;
}
void *handleIncomingFileTransfer(void *socketInfo)
{
    pthread_detach(pthread_self());
    //printf("Thread created id %ld for handling requesting data ^^^^^^\n \n",pthread_self());
    int i;
    int socketId = *((int *)socketInfo);
    char buffer[DEFAULT_NAME_SIZE];
    bzero(buffer, sizeof(buffer));
    int readResult = read(socketId,buffer,sizeof(buffer));
    char temp[40];
    strcpy(temp,LOCAL_FILE);
    strcat(temp,buffer);
    sendFile(temp,socketId);
    printf("File sent \n");
    close(socketId);
    return NULL;
}

void *synchronizeFolder()
{
    pthread_detach(pthread_self());
    //printf("Thread created id %ld for synchronizing data\n \n",pthread_self());
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
        if(time_taken > SYN_TIME )
        {
            time1 = clock();
            updateCount= updateCount+ 1;
            //printf("----->Updating ..%d\n",updateCount);
            // update file list into index file
            FILE *file = fopen(LIST_FILE,"r");
            fseek(file, 0L, SEEK_END);
            int preSize = ftell(file);
            fclose(file);
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
            while ((pDirent = readdir(pDir)) != NULL) 
            {
                if((strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0 || (*pDirent->d_name) == '.' ))
                {
                    continue;
                }
                else
                {
                    fprintf(f,"%s\n",pDirent->d_name);
                }   
            }
            rewind(file);
            file = fopen(LIST_FILE,"r");
            fseek(file, 0L, SEEK_END);
            int aftSize = ftell(file);
            fclose(file);
            fclose(f);

            closedir (pDir);   
            //CHeck if file size has changed and update file index to server
            //printf("%d----%d\n",preSize,aftSize);
            if(aftSize != preSize || updateCount == 1){
                //printf("File has been updated");
                 //Connect and update index file to server
                int socketToUpdate;
                if( connectToServerFunction(&socketToUpdate,INDEX_HOST,INDEX_PORT) < 0)
                {
                    printf("Update stopped . Server couldn't respond \n");
                    return NULL;
                }
                else {
                    //printf("Updating to Server in process \n");
                }
                write(socketToUpdate,SYNREQ,REQ_SIZE);

                char *updateVer = "1.1";
                //updateVer[2] = (char) updateCount;
                write(socketToUpdate,&updateCount,sizeof(int));
                write(socketToUpdate,&BIND_PORT_CLIENT_3,sizeof(BIND_PORT_CLIENT_3));
                //printf("%d@@@@",sendFile(LIST_FILE, socketToUpdate));
                
                if(sendFile(LIST_FILE,socketToUpdate) == 0 ){
                    printf("Update to server failed , thread stopped\n");
                    return NULL;
                }
                else{
                    printf("=====Synchronizing Complete\n");
                }
                
                close(socketToUpdate);
            }
            else {
                //printf("No changes in file");
            }
            
             
            
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
    //printf("Thread created id %ld for downloading data\n \n",pthread_self());
    int socketToSearch;
    if(connectToServerFunction(&socketToSearch,INDEX_HOST,INDEX_PORT) < 0 ){
        printf("Connect failed \n");
        return NULL;
    } else {
         printf("\n------------IndexServer Connected-----------\n");
    }   
   // int size = sizeof(DOWNREQ)/sizeof(DOWNREQ[0]);
    write(socketToSearch,DOWNREQ,REQ_SIZE);
    time_t time1;
    time_t time2;
    //char buffer[15];

    
    int i;
    char selection[DEFAULT_NAME_SIZE];
    bzero(selection, sizeof(selection));
    while(1)
    {
        //ENter file name so that server can search for it
        
		printf("--------Enter the file name you want to download-------- : \n") ;
		fflush(stdin);
		scanf("%s",selection);
        time1 = clock();
		if('\n' == selection[strlen(selection) - 1]) //remove \n
				selection[strlen(selection) - 1] = '\0';
		if(strcmp(selection,"QUIT") == 0)
			break;
		printf("=======You entered :  %s \n",selection);

        int sentBytes = send(socketToSearch,selection,DEFAULT_NAME_SIZE,0);
        
        // get search result from server 
        char result[40];
        bzero(result,sizeof(result));
        int searchRes = read(socketToSearch,result,REQ_SIZE);
        if(strcmp(result,FOUNDN) == 0){
            printf("=========File %s not found \n",selection);
            time2 =clock();
        }
        else {
             bzero(result,sizeof(result));
            //strcpy(result,SEARCH_RES);
            //strcat(result,selection);
            strcpy(result,LOG);
            strcat(result,SEARCH_RES);
            strcat(result,selection);
            printf("\n----------------------------------------------------\n");
            printf("--------------Getting search results------------------ \n");
            // save file
            receiveFile(result,socketToSearch);
            //char desIp[DEFAULT_NAME_SIZE];
            char* desIp;
            int selected;
            int numberOfPeers = getPeersResult(result);
            int recSelection;
            //get search result
            printf("Located file on %d peer\n" ,numberOfPeers);
            int currentStatus[numberOfPeers+1];
            
            checkAlive(result,currentStatus);
            // allow user to choose peer to download
            // recommend auto select
            for(int i = 1;i <= numberOfPeers;i++){
                if(currentStatus[i] == 1){
                    recSelection = i;
                    selected = recSelection;
                    break;
                }
                //printf("Status : %d\n",currentStatus[i]);
            }
            if(findFirst == 0){
                printf("Select peer to download file from : \n");
                scanf("%d",&selected);
            }
            
            
            if(currentStatus[selected] == 0){
                printf("You selected an offline peer,download cant be continue \n, canceling your request...\n");
                continue;
            }
            int desPort = getPeerAddr(result,&desIp,selected);
            printf("-----New target to download file :  %s:%d\n",desIp,desPort);
            // new Target machine aquired, connecting 
            int socketToDownload;
            
            if(connectToServerFunction(&socketToDownload,desIp,desPort) < 0){
                printf("-----Connect to target machine failed ...\n");
                time2= clock();
            }
            else {
                printf("-----Connected\n");
                //Downloading
                write(socketToDownload,selection,sizeof(selection));
                char temp2[40];
                strcpy(temp2,LOCAL_FILE);
                strcat(temp2,selection);
                if(receiveFile(temp2,socketToDownload)== 1)
                    printf("-----File %s has been downloaded\n",selection);
                else {
                    printf("File empty \n");
                }
                time2 = clock();    
            }
            

        }
        double time_taken = ((double)(time2 - time1))/CLOCKS_PER_SEC;
        printf("=====>Request has been handled in %lf seconds\n",time_taken);       
        
       
    }
    /*char *addr;
    int port;
    
    port = getPeerAddr("test.txt",&addr);
    printf("\n%s", addr);
    printf("\n%d", port);*/
    return NULL;
}

int sendFile(const char* fileName, int socket) // has sent file_size b4
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
            printf("Current working dir: %s\n", cwd);
        } else 
        {
            perror(" dkm xa hoigetcwd() error");
            return 0;
        }
        //perror(" fopen ");
        printf(" \t \t \t File not found %ld : %s !! \n \n \n",sizeof(fileName)/sizeof(char),fileName);
        totalSize = 0;
        write(socket, &totalSize, sizeof(totalSize));
        fclose(file);
        return 0;     
    }
    else 
    {
        //printf("FUCKFUCK");
        fseek(file, 0L, SEEK_END);
        totalSize = ftell(file);
        write(socket, &totalSize, sizeof(totalSize));
        rewind(file);
        if (totalSize > 0)
        {
            //printf("FUck");
            //file = fopen(fileName, "r");
            while (sizeof(segment) <= maxTransUnit)
            {
                maxTransUnit = fread(segment, 1, 1240, file);
                segment[maxTransUnit] = 0;
                write(socket, segment, maxTransUnit);
                size += maxTransUnit;
            }
            printf("=====Sent %d bytes:%s!\n",totalSize,fileName); 
            fclose(file);
            return 1;
        }
        else {
            //printf("%s has size = %d\n",fileName,totalSize);
            return 0;
            
        }
    }

    return 1;
}

int receiveFile(char* fileName, int socket)
{
	clock_t time = 0;
	int maxTransUnit = 1240;
	int size = 0, totalSize = 0;
    char segment[1240] = {0};
	char str[80];
	//fileName[0] = 'R';
    read(socket, &totalSize, sizeof(totalSize));
    if(totalSize <= 0) 
    {
        printf("Partner response with file size = 0 \n");
        return 0;
    } 
    else 
    {
		
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
        fclose(file);
        printf("=====Received %d bytes in %lf seconds\n",size, time_taken);
        return 1;
    }
}

int getPeerAddr(char *peerHasFile, char **addr,int selected)
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
    int count = 0;
    do {
        
        read = getline(&line, &len, fp);
        count++;
        if(count == selected){
            break;
        }
    }
    while(read != -1);
   
    *addr = strtok(line, ":");
    
    portChar = strtok(NULL, ":");
    portChar = strtok(portChar, ".");
    port = atoi(portChar);
    fclose(fp);
   // printf("%s***\n",*addr);
    return port;

}

int getPeersResult(char * fileName){

        FILE *file = fopen(fileName, "r");
        if(file == NULL)
        {
            printf("Cannot search result \n");
            return 0;
        }

        //printf("\nNoi dung file client gui len:\n\n");
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        int count = 0;
        while ((read = getline(&line, &len, file)) != -1) 
        {
        //printf("Retrieved line of length %zu:\n", read);
            count ++;
            //printf("%d : %s %",count, line);
        }
        printf("\n");
        fclose(file);
        
        return count;
}

int checkAlive( char *resultFile,int *status){
    int port;
    
    char *desIp = NULL;
    char* portChar = NULL;
    int socketToCheck;
    //char* status = NULL;
    //bzero(desIp,sizeof(desIp));
   // bzero()
    FILE *file = fopen(resultFile, "r");
    if(file == NULL)
    {
        printf("Cannot search result \n");
                return 0;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    //printf("%s",resultFile);
    
    while ((read = getline(&line, &len, file)) != -1) 
    {
            //printf("Retrieved line of length %zu:\n", read);
        count ++;
        desIp = strtok(line, ":");
        portChar = strtok(NULL, ":");
        portChar = strtok(portChar, ".");
        port = atoi(portChar);
               //printf("%d : %s %",count, line);       
        int checkServer = connectToServerFunction(&socketToCheck,desIp,port);
        if(checkServer < 0 ){
            status[count] = 0;
            printf("%d. %s:%d %s \n",count,desIp,port,DEAD);
        }
        else {
            status[count] = 1;
            printf("%d. %s:%d %s \n",count,desIp,port,ALIVE);
            //Find the first alive peer to download
            if(findFirst == 1){
                break;
            }
        }
        
        
        //close(socketToCheck);        
    }
    fclose(file);       
           


}
