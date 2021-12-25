/*
 * Author: Mohamed Yassin
 * File Name: TCPClient.cpp
*/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

#define PORT 41059 //to connect to port 41059 that TCP server is running on
#define MAX 100
using namespace std;

int main (int argc, char *argv[])
{
	
	char serverReply[MAX];
	char messageSent[MAX];
	int mysocket1; //socket that will connect to TCP server
	int bytes, len, done;
	struct sockaddr_in address; //struct sockaddr_in for indirection server
	char c;
	bool voted = false; //When client first, starts, it has not voted yet, so this is false.
	bool inVotingProcess = false; //This will be set to true while the client is in the process of voting.
	int key; //This is where the encryption key will be stored so that the client can use it to encrypt the vote.
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[1])); //using command line arguments when running client to connect to indirection server
	address.sin_addr.s_addr = inet_addr(argv[2]);
	//Socket Creation
	mysocket1 = socket(AF_INET, SOCK_STREAM, 0);
	if (mysocket1 == -1){
		printf("socket() call failed\n");
	}
	int status;
	int status2;
	//Connecting to Indirection Server
	status = connect(mysocket1, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	if (status == -1){
		printf("connect() call failed, trying other IP address\n");
		address.sin_addr.s_addr = inet_addr("136.159.5.25");
		status2 = connect(mysocket1, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
		if (status2 == -1){
			printf("connect() call failed\n");
			exit(0);
		}
	}
	//These are the welcome messages when the client first connects to the Indirection server
	printf("Connected\n");
	printf("Welcome to the Indirection Server! Here we offer 3 services:\n	Translator\n	Currency Converter\n	Voting Service\n");
	printf("To pick a service, type one of the following commands: | Translator | Currency Converter | Vote |\n");
	printf("To exit at any time, use the | Exit | command\n");
	done = 0;
	//Main Client loop, client loops after performing a request
	while (!done)
	{
		//Clear the message sent to the indirection server and the message received at the beginning of each loop
		bzero(serverReply, MAX);
		bzero(messageSent, MAX);
		len = 0;
		while( (c = getchar()) != '\n' )
		{
			messageSent[len] = c;
			len++;
		}
		/* make sure the message is null-terminated in C */
		messageSent[len] = '\0';	
		/* send it to the server via the socket */
		//If client requests to see voting summary and not yet voted, send a message
		if ((strncmp(messageSent, "Voting Summary", 14) == 0) && voted == false){
				printf("You cannot view the voting results until you voted.\n");
				continue;
		}
		//if client wants to vote again and has already voted, send a message
		if ((strncmp(messageSent, "Secure Voting", 13) == 0) && voted == true){
				printf("You have already voted.\n");
				continue;
		}
		//If client is in voting process, checks the ID of the candidate the client entered, then encrypts it, and changes inVotingProcess to be true
		if (inVotingProcess == true) {
			int candidateID = atoi(messageSent);
			int encryptedVote = candidateID * key;
			bzero(messageSent, MAX);
			strcpy(messageSent, to_string(encryptedVote).c_str());
			inVotingProcess = false;
			printf("\n");
		}
		//Client sends requests to indirection server
		send(mysocket1, messageSent, strlen(messageSent), 0);
		
		/* see what the server sends back */
		/*
		There are various strncmp functions here that print the client instructions based on the request they want to send to the server, or based on the reply they
		receive from the server
		*/
		if((bytes = recv(mysocket1, serverReply, MAX, 0)) > 0 )
		{
			if (strncmp(serverReply, "10", 2) != 0){ //As long as the server does not reply with the encryption key, print the server's reply ( we don't need client to see key )
			printf("%s", serverReply);
			printf("\n");
			}
			/* make sure the message is null-terminated in C */
			serverReply[bytes] = '\0';
			if (strncmp(messageSent, "Translator", 10) == 0)
			printf("Enter an English Word: ");
			if (strncmp(messageSent, "Currency Converter", 18) == 0)
			printf("Enter request in this order:  AmountOfMoney(NO SYMBOLS) SourceCurrency DestinationCurrency\n");
			if (strncmp(serverReply, "Your vote", 9) == 0) //Once server replies with "Your vote has been processed, change voted to be true so that can they see voting summary
					voted = true;
			if (strncmp(serverReply, "French translation", 18) == 0)
				printf("Enter an English Word: ");
			if (strncmp(serverReply, "Invalid request!", 16) == 0)
				printf("Enter an English Word: ");
			if (strncmp(serverReply, "10", 2) == 0)
				{
					key = atoi(serverReply); //initialize key with the integer version of the server reply, so key = 10
					printf("Enter the ID number of the candidate you wish to vote for: ");
					inVotingProcess = true; //If server replies with indirection key, voting process is now true
				}
			if (strncmp(serverReply, "10", 2) == 0){
				continue;
			}
	    }
		else
		{
			/* an error condition if the server dies unexpectedly */
			printf("Oh my! Server seems to have failed!\n");
			close(mysocket1);
			exit(1);
		}
		//Very important, if client wants to disconnect to indirection server at any time, they must type Exit , and it will close connection to the Indirection server
		if( strncmp(messageSent, "Exit", 4) == 0 )
			done = 1;
		/* check to see if done this session */
	}
	close(mysocket1);
	exit(0);
}
