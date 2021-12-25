/*
 * Author: Mohamed Yassin
 * File Name: Voting.cpp
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

#define bufferSize 1024
#define VOTINGPORT 42083
//This prints the Candidates from the Map in a string format to send it to client later
string printCandidates(map<int,string>Candidates, string candidateList){
    map<int,string>::iterator it = Candidates.begin();
    while (it != Candidates.end()){
		candidateList.append(" ID: ");
        candidateList.append(to_string(it->first));
        candidateList.append(" , ");
        candidateList.append(it->second);
        candidateList.append(" | ");
        ++it;
    }
    return candidateList;
}
//This prints the voting results from the Map in a string format to send it to the client later
string printVotingResults(map<string,int>Candidates, string candidateList){
    map<string,int>::iterator it = Candidates.begin();
    while (it != Candidates.end()){
        candidateList.append(it->first + "'s Votes: ");
        candidateList.append(to_string(it->second));
        candidateList.append(" | ");
        ++it;
    }
    return candidateList;
}
//this goes in client ( i dont use this in this server, i just have it here from testing, kept here just in case...
int secureVote(int candidateID, int key)
{
    return candidateID * key;
}
//this goes in microserver
//Decrypt client's vote
int decryptVote(int encryptedVote, int encryptionKey)
{
    return encryptedVote/encryptionKey;
}
//Updates the Map with new vote.
void updateResults(map<int,string> Candidates, map<string,int> *CandidateVotes, int decryptedVote)
{
    string name = Candidates.find(decryptedVote)->second;
    CandidateVotes->find(name)->second++;
}


int main(int argc, char const *argv[])
{
	int votingSock; //Socket for voting service
	char buffer[bufferSize];
	struct sockaddr_in serverAddress, clientAddress;
	string candidateList;
    string votingSummary;
    map<int,string> Candidates; //Map for Candidates
    map<string,int> CandidateVotes; //Map for Candidates and their votes.
    Candidates.insert(pair<int,string>(1, "Anthony"));
    Candidates.insert(pair<int,string>(2, "Cristiano"));
    Candidates.insert(pair<int,string>(3, "Joel"));
    Candidates.insert(pair<int,string>(4, "Steph"));
    CandidateVotes.insert(pair<string,int>("Anthony",431));
    CandidateVotes.insert(pair<string,int>("Cristiano",393));
    CandidateVotes.insert(pair<string,int>("Joel",1646));
    CandidateVotes.insert(pair<string,int>("Steph",54));
    int encryptionKey = 10; //encryption key
	char *invalidRequest = "You have sent an invalid request!"; //invalid request message
	char *processedVote = "Your vote has been processed!"; //successful vote message
	
	// Creating socket file descriptor
	if ( (votingSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		printf("socket creation failed");
		exit(1);
	}
	memset(&serverAddress, 0, sizeof(serverAddress));
	memset(&clientAddress, 0, sizeof(clientAddress));
	//setting up the sockaddr_in struct
	 serverAddress.sin_family = AF_INET; // IPv4
	 serverAddress.sin_addr.s_addr = INADDR_ANY;
     serverAddress.sin_port = htons(VOTINGPORT);
	 printf("test\n");
	 //Binding
	 if (bind(votingSock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	 {
		 printf("bind failed\n");
		 exit(1);
	 }
	 printf("test\n");
	 printf("hey\n");
	 int len, n;
	 //Main voting system loop, server stays online till its turned off
	 for (; ;){
		 memset(buffer, 0, sizeof(buffer)); //clear message received from indirection server at the beginning of each loop
		 len = sizeof(clientAddress);
		 printf("hello!\n");
		 n = recvfrom(votingSock, (char *)buffer, bufferSize, MSG_WAITALL, (struct sockaddr *)&clientAddress,(socklen_t *)&len); //receive client request from indirection server
		 buffer[n] = '\0';
		 printf("hi\n");
		 //Various strncmps, if the command is valid, uses functions to perform client's request, and sends the results to indirection server
		 if (strncmp(buffer, "Show Candidates", 15) == 0){
			 candidateList.clear();
			 candidateList = printCandidates(Candidates, candidateList);
			 sendto(votingSock, (const char *)candidateList.c_str(), strlen(candidateList.c_str()), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			 continue;
		 }
		 if (strncmp(buffer, "Voting Summary", 14) == 0){
			 votingSummary.clear();
			 votingSummary = printVotingResults(CandidateVotes, votingSummary);
			 sendto(votingSock, (const char *)votingSummary.c_str(), strlen(votingSummary.c_str()), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			 continue;
		 }
		 if (strncmp(buffer, "Secure Voting", 13) == 0){
			 sendto(votingSock, (const char *)to_string(encryptionKey).c_str(), strlen(to_string(encryptionKey).c_str()), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			 continue;
		 }
		 else {
			 cout << buffer << endl;
			 //If message received is not a command or one of these numbers (possible votes) send invalid request error message
			 if (atoi(buffer) != 10 && atoi(buffer) != 20 && atoi(buffer) != 30 && atoi(buffer) != 40)
			 {
				 sendto(votingSock, (const char *)invalidRequest, strlen(invalidRequest), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
				 continue;
			 }
			 //This code is only accessible once the indirection server sends the client's vote, then it decrypts the vote, updates candidate votings Map, and sends vote processed message to indirection server.
			 int integerBuffer = atoi(buffer);
			 cout << integerBuffer;
			 int decryptedVote = decryptVote(integerBuffer, encryptionKey);
			 cout << decryptedVote;
			 updateResults(Candidates, &CandidateVotes, decryptedVote);
			 sendto(votingSock, (const char *)processedVote, strlen(processedVote), MSG_CONFIRM, (const struct sockaddr *) &clientAddress,len);
			 continue;
		 }
		 printf("Message sent back\n");
		 }	 
	 return 0;
}