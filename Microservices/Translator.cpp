/*
 * Author: Mohamed Yassin
 * File Name: Translator.cpp
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;

#define bufferSize 1024
#define TRANSLATORPORT 42061
int main(int argc, char const *argv[])
{
	int translatorSock; //socket for Translator
	char buffer[bufferSize];
	//translations for words
	char *hello = "French translation: Bonjour";
	char *day = "French translation: Jour";
	char *strong = "French translation: Fort";
	char *dog = "French translation: Chien";
	char *love = "French translation: Amour";
	char *invalid = "Invalid request!\n"; //Error message when a not-yet translatable word is sent.
	struct sockaddr_in serverAddress, clientAddress;
	
	// Creating socket file descriptor
	if ( (translatorSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		printf("socket creation failed");
		exit(1);
	}
	memset(&serverAddress, 0, sizeof(serverAddress));
	memset(&clientAddress, 0, sizeof(clientAddress));
	
	//setting sockaddr_in struct with correct info
	 serverAddress.sin_family = AF_INET; // IPv4
	 serverAddress.sin_addr.s_addr = INADDR_ANY;
     serverAddress.sin_port = htons(TRANSLATORPORT);
	 printf("test\n");
	 //binding
	 if (bind(translatorSock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	 {
		 printf("bind failed\n");
		 exit(1);
	 }
	 printf("test\n");
	 printf("hey\n");
	 int len, n;
	 //main loop, stays on until it is turned off.
	 for (; ;){
		 memset(buffer, 0, sizeof(buffer)); //clear message received from indirection server at the beginning of each loop 
		 len = sizeof(clientAddress);
		 printf("hello!\n");
		 n = recvfrom(translatorSock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&clientAddress,(socklen_t *)&len); //receives client request from indirection server
		 buffer[n] = '\0';
		 printf("hi\n");
		 //Various strncmp messages to check word received and then send translations back to indirection server to be sent to client using sendto
		 if (strncmp(buffer, "Hello", 5) == 0) 
		 {
			  sendto(translatorSock, (const char *)hello, strlen(hello),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			  printf("Message sent back\n");
		 }
		 else if (strncmp(buffer, "Day", 3) == 0)
		 {
			  sendto(translatorSock, (const char *)day, strlen(day),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			  printf("Message sent back\n");
		 }
		 else if (strncmp(buffer, "Strong", 5) == 0)
		 {
			  sendto(translatorSock, (const char *)strong, strlen(strong),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			  printf("Message sent back\n");
		 }
		 else if (strncmp(buffer, "Dog", 3) == 0)
		 {
			  sendto(translatorSock, (const char *)dog, strlen(dog),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			  printf("Message sent back\n");
		 }
		 else if (strncmp(buffer, "Love", 4) == 0)
		 {
			  sendto(translatorSock, (const char *)love, strlen(love),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			  printf("Message sent back\n");
		 }
		 //If word to be translated is not a valid word, just send the error message "Invalid request!" to indirection server to be sent to client
		 else {
		 sendto(translatorSock, (const char *)invalid, strlen(invalid),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
		 printf("Message sent back\n");
		 }
	 }	 
	 return 0;
}