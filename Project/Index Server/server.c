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
const char* SYNREQ = "REQ_TO_SYNC";
const char* DOWNREQ = "REQ_TO_DOWN";
const int SYNREQ_SIZE = 12;
const int DOWNREQ_SIZE = 12; 
void * handleSynThread(void *);
void * handleReqThread(void *);

void saveClientAddr(const char *fileName, char *addr, char* portAndSizeFile)
{
    //save client addr as IP:PORT.txt\n

    //cannot pointing string literal, use strdup to use strtok
    char *str = strdup(portAndSizeFile);
    char* getPort = strtok(str, ":");
    
    strcpy(fileName, addr);
    strcat(fileName, ":");
    strcat(fileName, getPort);
    strcat(fileName, ".txt");
    strcat(fileName, "\n");
    return;
}

int main()
{
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
    if(listenSock < 0)
        printf("Create socket failed \n");
    
    indexHost.sin_family = AF_INET;
    indexHost.sin_port = htons(INDEX_PORT);
    indexHost.sin_addr.s_addr =htonl(INADDR_ANY);;
    memset(indexHost.sin_zero, '\0', sizeof indexHost.sin_zero);  
    bind(listenSock, (struct sockaddr *) &indexHost, sizeof(indexHost));

    if(listen(listenSock, MAX_CLIENTS) == 0)
        printf("Index Host running at : %s port %d \n",INDEX_HOST,INDEX_PORT);
    else 
        printf("Listening failed, server stopped \n");

    int* acceptedSocket;
     /* accepting new connections*/
    while(1)
    {

        acceptedSocket = malloc(sizeof(int));
        *acceptedSocket = accept(listenSock, (struct sockaddr *) &clientAddr, &addr_size);
        if (errno == EINTR) continue;
            else;// perror("accept error");
        printf("------>New connection accepted\n");
        char buffer[12];
        bzero(buffer,sizeof(buffer));
        int cout =read(*acceptedSocket,buffer,DOWNREQ_SIZE);  
        printf("Current Request :%s\n",buffer);
          
        if(strcmp(buffer,SYNREQ) == 0){
            pthread_create(&synThread, NULL ,&handleSynThread,(void *)acceptedSocket);
        }
        else if ( strcmp(buffer,DOWNREQ) == 0) {
            //pthread_create(&reqThread, NULL ,&handleReqThread,(void *)acceptedSocket);
        }    
         
    }
    return 0;
}

void *handleSynThread(void *socketInfo)
{
    pthread_detach(pthread_self());
    printf("New thread created for Synchronizing \n");
    int i;
    int socketId = *((int *)socketInfo);
    char buffer[50];
    bzero(buffer, sizeof(buffer));
    int readResult = read(socketId,&i,sizeof(i));
    printf("Current update from client : %d\n",i);
    if(receiveFile("ClientIndexfile.txt",socketId) == 1) {
        printf("Synchronizing successfully \n");
    }else{
        printf("Syncronize failed \n");
    }
    printf("This thread has been closed \n");
    return NULL;
}

void * handleReqThread(void *socketInfo)
{
    printf("New thread created for Handling Requests\n");
    pthread_detach(pthread_self());
    
    int i;
    int socketId = *((int *)socketInfo);
    char buffer[50];
    bzero(buffer, sizeof(buffer));
    while(1)
    {
        
        int readResult = read(socketId,&i,sizeof(i));
        if(i == 0){
            printf("Client doesnt want to send anything \n");
            break;
        }
        if(readResult == 0 )
        {
            printf("  Client has closed its connection \n ");
            break;
            //fix client ctrl+c or buffer = ""
        }
        //receiveFile("ClientIndexfile.txt",20,socketId);
        printf("%d Sent code\n", i);
        
    }
    close(socketId);
    printf("Thread exited\n");
    free(socketInfo);
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
        } 
        else 
        {
            //perror("getcwd() error");
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
        rewind(file);
        write(socket, &totalSize, sizeof(totalSize));

        if (totalSize > 0)
        {
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
        printf("=====Received %d bytes in %lf seconds \n",size, time_taken);
        fclose(file);
        return 1;
    }
}


void createFileHasReqFileToClient(char fileName[])
{
    //int fd = *(int *)sockfd;
    FILE* peerHasFileList,*fcheck;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    peerHasFileList = fopen("FileToAnsReqToClient.txt", "w");
	fflush(peerHasFileList);
    if(peerHasFileList == NULL)
	{
		printf("\nError opening list file");
		exit(1);
	}

    //create file to read all file in share folder
	struct dirent *de;  // Pointer for directory entry 
  
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("./peerShare"); 
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        //return 0; 
    } 

    // for readdir() 
    while ((de = readdir(dr)) != NULL)
	{
        line = NULL;
        len = 0;

		if(!strcmp(de->d_name, "."))
		{
			continue;
		}
		else if(!strcmp(de->d_name, ".."))
		{
			continue;
		}
		else
		{
            char IP[100];
            strcpy(IP,"./peerShare/");
            strcat(IP, de->d_name);
            fcheck = fopen(IP, "r");
            if(fcheck == NULL)
            {
                printf("Error opening file");
            }
            else
            {
                while ((read = getline(&line, &len, fcheck)) != -1) 
                {
                    printf("%s", line);
                    
                    if(strcmp(fileName, line)==0)
                    {
                        printf("%s\n", de->d_name);
                        
                        fprintf(peerHasFileList, "%s\n", de->d_name);	
                    }
                }
    
                
            }
            fclose (fcheck);
			
		}
	}  
    closedir(dr);   
	fclose(peerHasFileList);

}

