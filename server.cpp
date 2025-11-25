#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <thread>
#include <signal.h>
#include <iostream>
#include <sys/select.h>
using namespace std;
#define PI 4.0*atan(1.0)
#define BUFFER_SIZE 1024
#define PORT_NUMBER 5437
#define MAX_JOBS 10
struct Job{
    int cock;
    int jobID;
};

struct threadStruct{
    int** sharedData;
    int connfd;
};
Job jobQueue[MAX_JOBS];
int rows=10;
int seats=10;
int totalSeats;
int threadCounter;
int serverOn=0;
bool terminateFlag=false;

pthread_mutex_t jobMutex;
void drawMap(int** data);

void* clientHandler(void* arg) {
    threadStruct* args = (threadStruct*)arg;
    int** data = args->sharedData;
    int connfd = args->connfd;
    char sendBuff[BUFFER_SIZE];
    char recvBuff[BUFFER_SIZE];
    int bytesRead;
    int r=rows; int s=seats;
    while (1) {
        // Check for termination signal and incoming data
        pthread_mutex_lock(&jobMutex);
        bool k=terminateFlag;
        pthread_mutex_unlock(&jobMutex);
         // Set up file descriptor set for select
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(connfd, &readFds);

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000000; // 500 milliseconds

        if (k) {
            pthread_mutex_lock(&jobMutex);
            printf("client tried to buy tickets but they are gone\n");
            pthread_mutex_unlock(&jobMutex);
            fflush(stdout);
            char response[BUFFER_SIZE];
            sprintf(response, "Sorry, but we are out of tickets, try again later\n");
            write(connfd, response, strlen(response));
            //pthread_mutex_unlock(&jobMutex);
            break;
        }

        int selectResult = select(connfd + 1, &readFds, nullptr, nullptr, &timeout);
        if (selectResult == -1) {
            printf("Error in select\n");
            fflush(stdout);
            break;
        } else if (selectResult == 0) {
            // Timeout occurred, check for termination signal
            printf("\ntimed out\n");
            fflush(stdout);
            continue;
        }
        memset(&recvBuff, '0', sizeof(recvBuff));
        bytesRead = read(connfd, recvBuff, sizeof(recvBuff) - 1);
        if (bytesRead > 0) {
            recvBuff[bytesRead] = 0;  // Null-terminate the received data
            if (strcmp(recvBuff, "disc") == 0) {
                printf("Closing connection with %d\n", connfd);
                fflush(stdout);
                break;
            } else if(strcmp(recvBuff, "range") == 0){
                char response[BUFFER_SIZE];
                sprintf(response, " %d %d ", r,s);            
                write(connfd, response, strlen(response));
                continue;
            } 
            else {
            // Parse the integers from the purchase
                int num1, num2;
                
                if (sscanf(recvBuff, "%d %d", &num1, &num2) == 2) {
            // "int, int" command
                    char response[BUFFER_SIZE];
                    if((num1>r||num1<1)||(num2>s||num2<1)){
                        sprintf(response,"\nInvalid seats\nrows must be in range from (1-%d)\nseats must be in range from (1-%d)\n",r,s);
                        write(connfd, response, strlen(response));
                    } else {
                        //printf("read the data %d %d ", num1, num2);
                        //fflush(stdout);
                        pthread_mutex_lock(&jobMutex);
                        if(data[num1-1][num2-1]==0){
                            data[num1-1][num2-1]=1;
                            totalSeats--;
                            int t=totalSeats;
                            system("clear");
                            drawMap(data);
                            pthread_mutex_unlock(&jobMutex);
                            //printf("\nsold ticket at [%d][%d]\nTickets left:%d\n",num1,num2,t);
                            //fflush(stdout);                            
                            sprintf(response, "ticket at (%d,%d) was successfully purchased\n", num2,num1);
                            write(connfd, response, strlen(response));
                        } else {
                            pthread_mutex_unlock(&jobMutex);
                            sprintf(response, "the seat at (%d,%d) was taken\n",num1,num2);
                            write(connfd, response, strlen(response));
                        }  
                    }
                      
                } else {
                // Invalid command format
                    //printf("Invalid command format\n");
                    sprintf(sendBuff,"Invalid command format\nTo purchase a ticket type the row and seat numbers - example:10 23\nOr type \"disc\" to disconnect from the server \n");
                    write(connfd, sendBuff, strlen(sendBuff));
                    continue;
                }
            }
        } else if (bytesRead < 0) {
            printf("Error reading bytes on server\n");
            
        } else { printf("Client disconnected\n");
            break;}

        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Close the connection
    close(connfd);

    return NULL;
}
void* listener(void* arg){
    pthread_t* threadArray = static_cast<pthread_t*>(arg);
    while(1){
        pthread_mutex_lock(&jobMutex);
        if(totalSeats<=0){
            terminateFlag=true;
            pthread_mutex_unlock(&jobMutex);
            printf("\nout of seats-shutting the server.\n");
            break;
        } 
        pthread_mutex_unlock(&jobMutex);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    }   
    return NULL;
} 
void drawMap(int** data){
    printf("\nTickets left: %d\n_",totalSeats);
    for(int i=0; i<rows; i++){
        if(i<9) printf("_%d__",i+1);
        else printf("_%d_",i+1);
    }
    for(int i=0; i<rows; i++){
        printf("\n|");
        for(int j=0; j<seats; j++) {
            printf("_%d_|", data[i][j]);
        }printf("__%d",i+1);    
    }
    printf("\n");
}

int main(int argc, char *argv[]){
     int listenfd = 0;
    // Initialize job variables
    if((argc>3)||(argc==2)){
        printf("\ninvalid parameters.should be-> ./s rows seats\n");
        return 1;
    }
    if(argc>1){
       if(atoi(argv[1])>40) printf("Max number of rows is 40, using default value for rows (10).\n");else rows=stoi(argv[1]);
       if(atoi(argv[2])>40) printf("Max number of seats is 40, using default values for cols (10).\n");else seats=stoi(argv[2]);
    }
     
    totalSeats=rows*seats;
    int** data = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        data[i] = new int[seats];
    }
    for (int i=0; i<rows; i++){
        for (int j=0; j<seats; j++){
            data[i][j]=0;
        }
    }
   // memset(&data, '0', sizeof(data));
    pthread_mutex_init(&jobMutex, NULL);
    struct sockaddr_in serv_addr;

// Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

// Set up server address
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);

    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

// Bind socket to the server address
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

// Listen for incoming connections
    if (listen(listenfd, 100) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("*** Server started ***:\n");

    pthread_t workerThreads[16];
    pthread_t listenThread;
    pthread_create(&listenThread, NULL, listener, static_cast<void*>(workerThreads));
    threadCounter = 0;

    while (serverOn==0) {
        // Accept new connections
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if (connfd >= 0) {
        // Create a new thread to handle the connection
            threadStruct* args = new threadStruct;
            args->sharedData = data;
            args->connfd = connfd;
            pthread_t thread;
            //int* connfdPtr = (int*)malloc(sizeof(int));
            //*connfdPtr = connfd;
            pthread_create(&thread, NULL, clientHandler, args);
            workerThreads[threadCounter] = thread;
            threadCounter++;
        } else {
            printf("Error occurred while accepting new connection");
            // Error occurred while accepting new connection
            // ... (handle the error)
        }
    }
    for (int i = 0; i < threadCounter; i++) {
        pthread_join(workerThreads[i], NULL);
    }
        pthread_join(listenThread, NULL);
    

    // Cleanup the mutex and condition variable
    pthread_mutex_destroy(&jobMutex);
    

}

            
            

