/*
 * Author: Mohamed Yassin
 * File Name: TCPServer.cpp
*/
#include <stdio.h> //mainly for printf
#include <sys/socket.h> //needed for socket programming
#include <arpa/inet.h> 
#include <string.h> //for C strings
#include <unistd.h> //Not sure, but included just in case (Dr. Carey used it in his server for assignment 0)
#include <netinet/in.h> //for sockaddr_in structure
#include <signal.h> //Not sure, but included just in case (Dr. Carey used it in his server for assignment 0)
#include <stdlib.h> 
#include <netdb.h> 
#include <vector> 
#include <string> //for C++ strings. 

using namespace std;
#define PORT 41059 //this is the port the Indirection server uses to listen for clients 
#define TRANSLATORPORT 42061 //Translator UDP Microserver Port
#define CURRENCYPORT 42079 //Currency Converter UDP Microserver Port
#define VOTINGPORT 42083 //Voting UDP Microserver Port
#define MAX 500
#define bufferSize 1024
/* Global variable */

int main (int argc, char *argv[])
{
	int serverSock, clientSock; //sockets for the Indirection Server and to connect client
	int tSock, curSock, vSock; //sockets for the UDP microservers
	char buffer[MAX];
	int opt = 1;
	int c, pid;
	struct sockaddr_in server, client, translatorServer, currencyServer, votingServer; //sockaddr_in structs for indirection server and the three UDP microservers
	struct timeval sockTimeOut; //timeval struct to help setting timeouts when Microservers are offline
	char messageIn[MAX];
	char messageSent[MAX];
	char *serverDown = "The server is currently down, please try again later!";
	//Server-side implementation of Indirection Server
	//Creating socket;
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
	{
		printf("Could not create socket\n");
	}
	//Creating socket for UDP Client-Side connection to Translator Service
	tSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (tSock == -1)
	{
		printf("Could not set up UDP socket\n");
	}
	//Creating socket for UDP Client-Side connection to Currency Converter Service
	curSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (curSock == -1)
	{
		printf("Could not set up UDP socket\n");
	}
	//Creating socket for UDP Client-Side connection to Voting Service
	vSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (vSock == -1)
	{
		printf("Could not set up UDP socket\n");
	}
	//Allows reuse of local addresses for binding, so bind() call stops failing)
	if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		return 1;
	}
	//Preparing sockaddr_in structures for the indirection server as well as the three UDP microservers
	
	server.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Any IP address on the local machine
	server.sin_port = htons(PORT); //Use port 41059 to listen for client connection
	
	memset(&translatorServer, 0, sizeof(translatorServer));
	translatorServer.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	translatorServer.sin_port = htons(TRANSLATORPORT); //PORT 42061
	translatorServer.sin_addr.s_addr = inet_addr("136.159.5.25");  //All the microservers will be running on IP address 136.159.5.25
	
	memset(&currencyServer, 0, sizeof(currencyServer));
	currencyServer.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	currencyServer.sin_port = htons(CURRENCYPORT); //PORT 42079
	currencyServer.sin_addr.s_addr = inet_addr("136.159.5.25"); 
	
	memset(&votingServer, 0, sizeof(votingServer));
	votingServer.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	votingServer.sin_port = htons(VOTINGPORT); //Port 42083
	votingServer.sin_addr.s_addr = inet_addr("136.159.5.25"); 
	
	//Binding for server-side implentation of Indirection server, returns > 0 if bind succeeds
	if (bind(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0) 
	{
		printf("bind() call failed\n");
		return 1;
	}
	//Listening for server-side implementation of Indirection Server, returns > 0 if listen succeeds
	int status;
	status = listen(serverSock, 5);
	if (status == -1)
	{
		printf("listen() call failed\n");
		return 1;
	}
	//Setting timeouts for the UDP microserver sockets, if the timeout is reached, will send a message to inform client services are currently offline
	sockTimeOut.tv_sec = 3;
	sockTimeOut.tv_usec = 0;
		if (setsockopt(tSock, SOL_SOCKET, SO_RCVTIMEO, (char*) &sockTimeOut, sizeof sockTimeOut) < 0)
		{
			printf("setsockopt() call failed\n");
			return 1;
		}
		if (setsockopt(curSock, SOL_SOCKET, SO_RCVTIMEO, (char*) &sockTimeOut, sizeof sockTimeOut) < 0)
		{
			printf("setsockopt() call failed\n");
			return 1;
		}
		if (setsockopt(vSock, SOL_SOCKET, SO_RCVTIMEO, (char*) &sockTimeOut, sizeof sockTimeOut) < 0)
		{
			printf("setsockopt() call failed\n");
			return 1;
		}
	//Just a message to let myself know that the server is now listening for clients
	fprintf(stderr, "Welcome! I am the Indirection Server \n");
    fprintf(stderr, "Server listening on TCP port %d...\n\n", PORT);
	c = sizeof(struct sockaddr_in);
	//Main server loop, server loops forever listening for requests from clients 
	for (; ;){
		clientSock = accept(serverSock, (struct sockaddr *)&client, (socklen_t*)&c); //Connection Acceptance for client to server of Indirection Server
		if (clientSock < 0 )
		{
			printf("accept failed\n");
			return 1;
		}
		puts("Connection accepted\n");
		/* try to create a child process to deal with this new client */
		pid = fork();

		/* use process id (pid) returned by fork to decide what to do next */
		if( pid < 0 )
		{
	    fprintf(stderr, "server: fork() call failed!\n");
	    exit(1);
		}
		else if( pid == 0 )
		{
			/* this is the child process doing this part */

			/* don't need the parent listener socket that was inherited */
			close(serverSock);

			/* obtain the message from this client */
			int bytes;
			int done = 0;
			bool goodRequest;
			while(!done )
			{
				goodRequest = false; //this is false by default, if the message received from client is a valid service, it will change to true, else sends an error message to client 
				bzero(messageIn, MAX); //Clear message from client received at beginning of each server loop 
				bzero(messageSent, MAX); //Clear message sent to client and microservers at the beginning of each server loop
				bytes = recv(clientSock, messageIn, MAX, 0); //Receives requests from client here
				/* print out the received message */
				printf("Child process received %d bytes with command: <%s>\n", bytes, messageIn);
				//If message received from client is exit, done = 1 so the server exits main loop and sends a goodbye message to client.
				if (strncmp(messageIn, "Exit", 4) == 0 )
				{
					goodRequest = true;
					sprintf(messageSent, "Goodbye! See you next time.\n");
					done = 1;
				}
				//Main menu, using strncmp to see what services client wants
				
				//Currency Converter
				else if (strncmp(messageIn, "Currency Converter", 18) == 0 )
				{
					memset(buffer, 0, sizeof(buffer)); //Clear message received from the UDP microservers
					int n, len;
					goodRequest = true;
					sprintf(messageSent, "Welcome to the Currency Converter Service!\n"); //Short welcome message to make client feel welcome 
					if(send(clientSock, messageSent, strlen(messageSent), 0) < 0)
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					int currencyDone = 0;
					while (!currencyDone){ //Main loop for Currency Converter, stays in the service until client requests to go Back to main menu
						bzero(messageIn, MAX);
						bzero(messageSent, MAX);
						bytes = recv(clientSock, messageIn, MAX, 0);
						if (strncmp(messageIn, "Back", 4) == 0) //If client sends the word Back at any point, return back to main menu to select services and leave currency Converter Service
						{
							sprintf(messageSent, "Thanks for using the currency converter service, you can now pick other services again!\n");
							currencyDone = 1;
							continue;
						}
						sendto(curSock, (const char *)messageIn, strlen(messageIn), MSG_CONFIRM, (const struct sockaddr *)&currencyServer, sizeof(currencyServer)); //Send client request to currency converter
						printf("Currency conversion request sent.\n");
						n = recvfrom(curSock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&currencyServer,(socklen_t *)&len); //Receive results from currency converter
						if (n == -1) //basically if receive times out, n will be negative one since recvfrom didnt receive anything, send message to client saying service is down
						{
							if (send(clientSock, serverDown, strlen(serverDown), 0) < 0)
								fprintf(stderr, "Send failed on connection\n");
							currencyDone = 1;
							continue; 
						}
						buffer[n] = '\0';
						bzero(messageSent, MAX);  //empty messageSent 
						sprintf(messageSent, buffer);//put buffer in message sent
						if(send(clientSock, messageSent, strlen(messageSent), 0) < 0) //Send the response from currency converter to the client
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					}
				}
				//Translator
				else if (strncmp(messageIn, "Translator", 10) == 0 )
				{
					memset(buffer, 0, sizeof(buffer)); //empty message received from UDP microservers
					int n, len;
					goodRequest = true; 
					sprintf(messageSent, "Welcome to the Translator Service!\n"); //short welcome message
					if(send(clientSock, messageSent, strlen(messageSent), 0) < 0) //send welcome message to client
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					int translatorDone = 0;
					//main loop for translator, stays until client wants to return to the main menu
					while (!translatorDone){
						bzero(messageIn, MAX); //Empty message received from client at the beginning of each loop
						bzero(messageSent, MAX);//Empty message sent to client and microservers are the begining of each loop
						bytes = recv(clientSock, messageIn, MAX, 0);
						if (strncmp(messageIn, "Back", 4) == 0) //to return to main menu
						{
							sprintf(messageSent, "Thanks for using the translator service, you can now pick other services again!\n");
							translatorDone = 1;
							continue;
						}
						sendto(tSock, (const char *)messageIn, strlen(messageIn), MSG_CONFIRM, (const struct sockaddr *)&translatorServer, sizeof(translatorServer)); //send client request to translator service
						printf("Translate Request sent.\n");
						n = recvfrom(tSock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&translatorServer,(socklen_t *)&len); //receive response from translator service
						if (n == -1) //send service offline message to client if nothing received from microserver 
						{
							if (send(clientSock, serverDown, strlen(serverDown), 0) < 0)
								fprintf(stderr, "Send failed on connection\n");
							translatorDone = 1;
							continue;
						}
						buffer[n] = '\0';
						bzero(messageSent, MAX);
						sprintf(messageSent, buffer);
						if(send(clientSock, messageSent, strlen(messageSent), 0) < 0) //Send response from translator to the client
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					}
				}
				//Voting Service
				else if (strncmp(messageIn, "Vote", 4) == 0 )
				{
					memset(buffer, 0, sizeof(buffer)); //clear message received from UDP microservers
					int n, len;
					goodRequest = true;
					sprintf(messageSent, "Commands for voting: Show Candidates | Secure Voting | Voting Summary \n"); //Some helpful commands for the voting service
					if(send(clientSock, messageSent, strlen(messageSent), 0) < 0)
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					int voteDone = 0;
					//Loop for voting service, stays here until client wants to go Back to the main menu
					while (!voteDone){
						bzero(messageIn, MAX); //clear message received from client at the beginning of each loop
						bzero(messageSent, MAX); //clear messages sent to client and voting service at the beginning of each loop
						bytes = recv(clientSock, messageIn, MAX, 0);
						if (strncmp(messageIn, "Back", 4) == 0)//when client wants to go back to main menu
						{
							sprintf(messageSent, "Thanks for using the voting service, you can now pick other services again!\n");
							voteDone = 1;
							continue;
						}
						sendto(vSock, (const char *)messageIn, strlen(messageIn), MSG_CONFIRM, (const struct sockaddr *)&votingServer, sizeof(votingServer)); //send client request to voting service
						printf("Request sent.\n");
						n = recvfrom(vSock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&votingServer,(socklen_t *)&len); //receive response request from voting service
						if (n == -1)
						{
							if (send(clientSock, serverDown, strlen(serverDown), 0) < 0) //send error message if service is done
								fprintf(stderr, "Send failed on connection\n");
							voteDone = 1;
							continue;
						}
						buffer[n] = '\0';
						bzero(messageSent, MAX);
						sprintf(messageSent, buffer);
						if(send(clientSock, messageSent, strlen(messageSent), 0) < 0) //send response to client from voting service
						{
							fprintf(stderr, "Send failed on connection\n");
						}
					}
				}
				if (goodRequest == false) //if client says something that's not a valid command, just return a friendly message saying their command is unrecognized
				{
					sprintf(messageSent, "Unrecognized command, please refer to the manual and try again!.\n");
				}
				if(send(clientSock, messageSent, strlen(messageSent), 0) < 0)
				{
					fprintf(stderr, "Send failed on connection\n");
				}
				/* send the result message back to the client */
			}
			/* when client is no longer sending information to us, */
			/* the socket can be closed and the child process terminated */
			fprintf(stderr, "Shutting down child process for connection\n");
			close(clientSock);
			exit(0);
		}
		else
		{
			/* the parent process is the one doing this part */
			fprintf(stderr, "Created child process to handle that client\n");
			fprintf(stderr, "Parent going back to job of listening...\n\n");
			/* parent doesn't need the childsockfd */
			close(clientSock);
		}
	}
	return 0;
}