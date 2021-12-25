/*
 * Author: Mohamed Yassin
 * File Name: CurrencyConverter.cpp
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <map>
#include <string>
using namespace std;

#define bufferSize 1024
#define CURRENCYPORT 42079
int main(int argc, char const *argv[])
{
	int currencySock; //Socket for currency converter
	char buffer[bufferSize];
	char *wrongInput = "Wrong Input Format!";
	struct sockaddr_in serverAddress, clientAddress;
	map<string, double> Currencies; //Map Data Structure to store currencies and their respective conversion rates relative to the Canadian Dollar
    Currencies.insert(pair<string,double>("CAD",1));
    Currencies.insert(pair<string,double>("USD",0.79));
    Currencies.insert(pair<string,double>("UK",0.59));
    Currencies.insert(pair<string,double>("EURO",0.70));
    Currencies.insert(pair<string,double>("BTC",0.000016));
	char s[2] = " "; //Needed to parse Client request
	char *token; //Needed to parse Client request
	//All these doubles and strings are just used to store intermediate values in the conversion operation
	double amount; 
	string sourceCurrency;
	string convertCurrency;
	double source;
	double convert;
	double convertedMoney;
	string finalResult; //Holds the final result to be sent back to the client
	// Creating socket file descriptor
	if ( (currencySock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		printf("socket creation failed");
		exit(1);
	}
	memset(&serverAddress, 0, sizeof(serverAddress));
	memset(&clientAddress, 0, sizeof(clientAddress));
	
	//setting sockaddr_in struct with correct info
	 serverAddress.sin_family = AF_INET; // IPv4
	 serverAddress.sin_addr.s_addr = INADDR_ANY;
     serverAddress.sin_port = htons(CURRENCYPORT);
	 printf("test\n");
	 //Binding
	 if (bind(currencySock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	 {
		 printf("bind failed\n");
		 exit(1);
	 }
	 printf("test\n");
	 printf("hey\n");
	 int len, n;
	 //Main loop, Currency Converter stays on until it is turned off
	 for (; ;){
		 memset(buffer, 0, sizeof(buffer)); //Clear message received from indirection server at the beginning of each loop
		 len = sizeof(clientAddress);
		 printf("hello!\n");
		 n = recvfrom(currencySock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&clientAddress,(socklen_t *)&len); //receive client request from indirection server
		 buffer[n] = '\0';
		 token = strtok(buffer,s); //delimit the client requests at the spaces such as "12231 CAD USD" to retrieve individual components 
		 if (token == NULL){ //if at any point token == NULL, this means client request is the wrong format because there's no spaces, so send error message saying wrong input
			sendto(currencySock, (const char *)wrongInput, strlen(wrongInput), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			continue;
		 }
		 amount = atof(token); //amount is first component (atof converts strng to double value)
		 token = strtok(NULL,s);
		 if (token == NULL){
			sendto(currencySock, (const char *)wrongInput, strlen(wrongInput), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			continue;
		 }
		 sourceCurrency =  token; //source currency is second component
		 token = strtok(NULL,s);
		 if (token == NULL){
			sendto(currencySock, (const char *)wrongInput, strlen(wrongInput), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			continue;
		 }
		 convertCurrency = token; //destination currency is third component
		 //if source currency is not a supported currency in the converter, send the wrong input message yet again
		 if (sourceCurrency.compare("CAD") != 0 && sourceCurrency.compare("USD") != 0 && sourceCurrency.compare("UK") != 0 && sourceCurrency.compare ("EURO") != 0 && sourceCurrency.compare("BTC") != 0){
            sendto(currencySock, (const char *)wrongInput, strlen(wrongInput), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			continue;
		 }
		 //if destination currency is not a supported currency in the converter, send the wrong input message yet again
		 if (convertCurrency.compare("CAD") != 0 && convertCurrency.compare("USD") != 0 && convertCurrency.compare("UK") != 0 && convertCurrency.compare ("EURO") != 0 && convertCurrency.compare("BTC") != 0){
			sendto(currencySock, (const char *)wrongInput, strlen(wrongInput), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			continue;
		 }
		 //Code to perform the conversion by finding rates in map, doing a calculation, then putting the result back into a c string to send to indirection server
		 source = Currencies.find(sourceCurrency)->second;
		 convert = Currencies.find(convertCurrency)->second;
		 convertedMoney = (convert/source)*amount;
		 finalResult.append(to_string(convertedMoney));
		 finalResult.append(" ");
		 finalResult.append(convertCurrency);
		 sendto(currencySock, (const char *)finalResult.c_str(), strlen(finalResult.c_str()),MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len); //send result back to indirection server to be sent to client
		 printf("%f",convertedMoney);
		 finalResult.clear();
		 printf("hi\n");
	 }	 
	 return 0;
}