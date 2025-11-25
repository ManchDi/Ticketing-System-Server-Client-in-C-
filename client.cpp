#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <chrono>
#include <random>
#include <ctime>
#include <cstring>
#include <cstdint>

using namespace std;
#define BUFFER_SIZE 1024
#define PORT_NUMBER 5437
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_TIMEOUT 5
bool automatic=false;
bool defaultVals=true;
bool badFile=false;
int rows, cols;
int delays[3]={3,5,7};

map<string, string> parseINI(const string& filename) {
    map<string, string> data;
    ifstream file(filename);
    if (!file){
        cout << "Failed to open file: " << filename << endl;
        badFile=true;
        return data;
    }

    string line;
    string section;
    while (getline(file, line)) {
        istringstream iss(line);
        string key, value, equals;
        if (line.empty()) {
            continue;
        }
        if (line[0] == '[') {
            continue;
        } else if (iss >> key >>equals >>value) {
           // printf("data[%s] = [%s]",key, value);
            data[key] = value;
        }
    }

    file.close();
    return data;
}
 std::pair<int, int> getRandom(){	
	std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::mt19937 gen(tp.time_since_epoch().count());
    std::uniform_int_distribution<int> dist(1, cols);
    int x = dist(gen);
    std::mt19937 gen1(tp.time_since_epoch().count()+2);
    std::uniform_int_distribution<int> dist1(1, rows);
    int y = dist1(gen1);
	return std::make_pair(x, y);
}
 int getRandomInt(){
	std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
    std::mt19937 gen(tp.time_since_epoch().count());
    std::uniform_int_distribution<int> dist(1, 3);
    int x = dist(gen);
	return x;
}


int main(int argc, char *argv[]){
	int sockfd = 0, n = 0, port=PORT_NUMBER, timeout = DEFAULT_TIMEOUT;
	string ip=DEFAULT_IP;
	char recvBuff[BUFFER_SIZE];
	struct sockaddr_in serv_addr;
	map<string, string> iniData;

	if((argc<2) || (argc>3)){
		printf("\n Usage: %s <ip of server> \n",argv[0]);
			return 1;
	}
	string mode=argv[1];
	if(mode=="automatic") automatic=true;
	else if(mode=="manual"){} else {printf("Invalid format\nmake sure to include \"manual\" or \"automatic\"\n");return 1;} 
	if(argc>2) defaultVals=false;

	if(automatic) printf("\nAUTO PURCHASE IS ON\n");
	
	// Fetch data
	if (!defaultVals){
	 	iniData= parseINI(argv[2]);
	 	if(badFile) return 1;
    	 ip = iniData["IP"];
    	 cout<<ip<<endl;
    	 port = stoi(iniData["Port"]);
    	 timeout = stoi(iniData["Timeout"]);
	}

	//creating socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	//setting up serv struct and connecting to it
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0){
		printf("\n inet_pton error occured\n");
		return 1;
	}

	//connecting and timing out if needed
	int x=connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(x< 0){
		printf("\n ***_Error: Connection failed_***\n");
		int count=3;
		
		while(count>0){
			int innerCount=100;
			printf("\n...waiting %d seconds before trying again \n",timeout);
			sleep(timeout);
			printf("...connecting again-> ");
			while(innerCount>0){
				x=connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				if( x >= 0){
					printf("success...\n");
					count=-1;
					break;
				} else {printf("\n Connection failed. Error: %s \n", strerror(errno));innerCount--;}
			}
			if(count>0){
				printf("failed...%d\n",x);
				count--;
			} else break;
			
		}
		if (count==0) {close (sockfd);return 1;}
	}
	//
	//vector for range values from server
	int exitFlag=0;
	vector<string> args;

	//if in auto mode, we need the range for values.
	if(automatic){
		char sendBuff[BUFFER_SIZE];
		sprintf(sendBuff, "range");
		if (write(sockfd, sendBuff, strlen(sendBuff)) < 0) {
       		printf("\nMessage asking the range -failed\n");
       		fflush(stdout);
        	return 1;
  		}
  		//read buffer
  		memset(recvBuff, '0',sizeof(recvBuff));
  		if(n = read(sockfd, recvBuff, sizeof(recvBuff)-1)<0){
  			printf("\nreading range from the server -failed\n");
  			return 1;
  		} else {
  			char* tokens = strtok(recvBuff, " ");
        //filling the list with our tokens
        	while (tokens != NULL) {        
            	args.push_back(tokens);
            	//getting next token
            	tokens = strtok(NULL, " "); 
        	}
  			rows =stoi(args[0]);
  			cols =stoi(args[1]);
  			//cout<<"\nour received buffer: "<< rows <<" "<< cols<<endl;
  		}
  	}


	while (!exitFlag) {
		char input[BUFFER_SIZE];

		if(automatic){
			printf("\nPurchasing ticket -> ");
			fflush(stdout);
			
			std::pair<int, int> coords=getRandom();
			
			int x=coords.first;
			int y=coords.second;
			char buffer[BUFFER_SIZE];
			
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%d %d",x,y);
			if (write(sockfd, buffer, strlen(buffer)) < 0) {
        	    printf("Message sending -failed\n");
       		    fflush(stdout);
        	    break;
        	}
        	int w=getRandomInt();
        	//printf("\nwait is: %d",w);
			fflush(stdout);
        	sleep(0.5);
		}

		else{    
		// Read input from the client's console
    		printf("\nEnter seat or \"disc\": ");
    		fflush(stdout);  // Flush the output buffer to ensure prompt display
    		fgets(input, sizeof(input), stdin);

    	// Remove trailing newline character from the input just in case
    		size_t len = strlen(input);
    		if (len > 0 && input[len - 1] == '\n') {
    		    input[len - 1] = '\0';
    		}
    		if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
        		close(sockfd);
        	    break;
        	}

        //send the input
    		if (write(sockfd, input, strlen(input)) < 0) {
        	    printf("Message sending failed\n");
       		     fflush(stdout);
        	    continue;
        	}
    	}


        //read the response
        memset(recvBuff, '0',sizeof(recvBuff));
        n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
			if (n > 0) {
                    recvBuff[n] = 0;
                    if(fputs(recvBuff, stdout) == EOF){
                        printf("\n Error : Fputs error\n");
                    }
                    if (recvBuff[n-3]=='e'&&recvBuff[n-2]=='r'){
                    	printf("\nServer closed the connection...\n");
                    	break;
                    }
                } else if (n == 0) {
                 // Connection closed by the server
                    printf("\nServer closed the connection\n");
                    break;
                } else {
                // Error occurred during read
                    printf("\nError: Read error\n");
                    break;
                }        
	}    
	close(sockfd);
	return 0;
}