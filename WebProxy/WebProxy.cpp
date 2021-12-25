/*
 File name : WebProxy.cpp
 Version: Final Version
 Author Name: Mohamed Yassin
*/

/*
Introduction:
This is the most up-to-date version of the web proxy I have so far. 
I have attached helpful comments and some documentation to explain how the code works to best of my understanding.
While this is a C++ file, I initially wrote most of it in C, but switched to C++
as I thought using a vector would be easier for the dynamic word blocking. 
*/



#include <stdio.h> //mainly for printf
#include <sys/socket.h> //needed for socket programming
#include <arpa/inet.h> //used for inet_ntoa to check if hostname was succesfully converted to IP
#include <string.h> //for C strings
#include <unistd.h> //Not sure, but included just in case (Dr. Carey used it in his server for assignment 0)
#include <netinet/in.h> //for sockaddr_in structure
#include <signal.h> //Not sure, but included just in case (Dr. Carey used it in his server for assignment 0)
#include <stdlib.h> //Included because initially, I was doing dynamic blocking in C++
#include <netdb.h> //for hostent structure
#include <vector> //for vectors, used in dynamic blocking
#include <string> //for C++ strings. 
using namespace std;

//First let's make a server-size socket that will wait for my
//browser to connect to it. 
//web browser sends a GET request to server side, parse the URL
//from this GET request and convert the URL to IP.
//Make a new socket that will act as a client at this point.
//Use IP and send GET request to that specific server.	
//Retrieve the results from web server to client, send that to
//server, then send that back to the browser.


/*
REQUIRES: const char* and vector<string> arguments. 
PROMISES: iterates through vector<string> and uses strstr method to find matching words in URL and the
list of blocked words. Returns true if any matches are found, false if URL contains no blocked words. 
*/
bool findBlockedWordInURL(const char* parsedURL, vector<string> blockedList)
{
    vector<string>::iterator it = blockedList.begin();
    while (it != blockedList.end())
    {
		//printf(it->c_str());
		//printf(it->c_str());
		//printf(parsedURL);
         if ((strstr(parsedURL, (*it).c_str()))) //c_str() converts a string to a C-string
        {
            return true;
        }
        ++it;
    }
    return false;
}
/*
REQUIRES: char[] and vector<string> arguments.
PROMISES: This is for the bonus marks section of the assignment, almost the same as findBlockedWordInURL but it compares the list of blockedWords to the
HTTP response from the website. Returns true if any matches are found, false if HTTP response contains no blocked words.
I disabled this because a day after I got it to work, it randomly stopped working and i can not figure out why.
*/
/*
bool findBlockedWordInResponse(char webResponse[], vector<string> blockedList)
{
    vector<string>::iterator it = blockedList.begin();
    while (it != blockedList.end())
    {
         if ((strstr(webResponse, (*it).c_str())))
        {
            return true;
			printf("true");
        }
        ++it;
    }
    return false;
}
*/
int main (int argc, char *argv[])
{
	
	int parentSock, childSock, telnParent, telnChild; //parentSock and childSock: for proxy server. telnParent, telnChild: for dynamic blocking configuration
	int c;
	struct sockaddr_in server, client, teln; //server for proxy server implementation, and teln for blocking configuration on different port
	struct timeval sockTimeOut; //timeval struct to help with setting timeouts on recv() call later on.
	int opt = 1;
	vector<string> blockedContent; //List of blocked words.
	char msgFromBrowser[20000]; //Storing HTTP messages from the Browser
	char msgFromWebServer[20000]; //Storing HTTP responses from the Website
	char blockRequests[1000]; //Storing BLOCK or UNBLOCK commands from the telnet client.
	
	//Server-side implementation of proxy
	//Creating socket;
	parentSock = socket(AF_INET, SOCK_STREAM, 0);
	if (parentSock == -1)
	{
		printf("Could not create socket\n");
	}
	//Allows reuse of local addresses for binding, so bind() call stops failing)
	if (setsockopt(parentSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		return 1;
	}
	//Creating Socket to listen for blocking configuration from telnet 
	telnParent = socket(AF_INET, SOCK_STREAM, 0);
	if (telnParent == -1)
	{
		printf("Could not create socket\n");
	}
	//Allows reuse of local addresses for binding, so bind() call stops failing)
	if (setsockopt(telnParent, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		return 1;
	}
	//Preparing sockaddr_in structures
	server.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	server.sin_addr.s_addr = INADDR_ANY; //Any IP address on the local machine
	server.sin_port = htons(42019); //Use port 42019 to listen for browser connection
	
	teln.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
	teln.sin_addr.s_addr = INADDR_ANY; //Any IP address on the local machine
	teln.sin_port = htons(42018); //use port 42018 to listen for telnet client for blocking configuration
	

	//Binding for server-side implentation of proxy, returns > 0 if bind succeeds
	if (bind(parentSock, (struct sockaddr *)&server, sizeof(server)) < 0) 
	{
		printf("bind() call failed\n");
		return 1;
	}
	printf("bind done\n");
	//Listening for server-side implementation of proxy, returns > 0 if listen succeeds
	int status;
	status = listen(parentSock, 5);
	if (status == -1)
	{
		printf("listen() call failed\n");
		return 1;
	}
	//Binding for server-side implentation of blocking configuration, returns > 0 if listening succeeds
	if (bind(telnParent, (struct sockaddr *)&teln, sizeof(teln)) < 0)
	{
		printf("bind() call failed\n");
		return 1;
	}
	//Listening for server-side implementation of blocking configuration, returns > 0 if listening succeeds
	int telnStatus;
	telnStatus = listen(telnParent, 5);
	if (telnStatus == -1)
	{
		printf("listen() call failed\n");
	}
	//Accept incoming connection (need to connect telnet client at this stage to advance further into the program
	//Notice that this is outside of the main for loop below, so that the blocking configuration stays connected throughout the duration of the program
	printf("Listening...\n");
	printf("Please connect blocking configuration...\n");
	c = sizeof(struct sockaddr_in);
		telnChild = accept(telnParent, (struct sockaddr *)&client, (socklen_t*)&c);
		if (telnChild < 0 )
		{
			printf("accept failed\n");
			return 1;
		}
		puts ("Connection accepted");
	
	/*
	Using the timeval struct from earlier to set the recv() telnet timeout to 0.25 seconds, meaning that if nothing is received from the telnet client for 0.25 seconds
	the program can advance further. 0.25 seconds seemed like a reasonable time for keeping browsers relatively speedy, but can be sped up even further by reducing
	tv_usec to 100,000 microseconds for example
	*/
	sockTimeOut.tv_sec = 0;
	sockTimeOut.tv_usec = 250000;
		if (setsockopt(telnChild, SOL_SOCKET, SO_RCVTIMEO, (char*) &sockTimeOut, sizeof sockTimeOut) < 0)
		{
			printf("setsockopt() call failed\n");
			return 1;
		}
	//Main loop, servers loop to listen for requests from clients
	for (; ;){	
	memset(msgFromBrowser, 0, strlen(msgFromBrowser)); //Clearing HTTP requests char array at the beginning of each loop to ensure accurate requests.
	memset(msgFromWebServer,0, strlen(msgFromWebServer));//Clearing HTTP responses char array at the beginning of each loop to ensure accurate responses.
	childSock = accept(parentSock, (struct sockaddr *)&client, (socklen_t*)&c); //Connection Acceptance for client to server of proxy (from Browser)
		if (childSock < 0 )
		{
			printf("accept failed\n");
			return 1;
		}
	
		if (recv(telnChild, blockRequests, 1000, 0) < 0) //Any BLOCK or UNBLOCK commands from the blocking configuration are received here and saved into blockRequests
		{
			printf("Hi");
		}
		//printf(blockRequests);
	    if (recv(childSock, msgFromBrowser, 20000, 0) < 0) //HTTP requests from browser are received here and saved into msgFromBrowser char array
		{
			printf("recv() call failed\n");
			return 1;
		}
		
		//printf(msgFromBrowser);
		//parse HTTP requeset to extract IP of hostname and url after /
		//All of this section is for parsing the URL
		const char *firstPart = "http://"; 
		const char *secondPart = "/";
		const char *space = " ";
		const char *post = "POST"; //to block POST commands that ruined results
		const char *fireFoxError = "detectportal.firefox.com"; //firefox link that was ruining results
		const char *blockSignal = "BLOCK ";
		const char *unblockSignal = "UNBLOCK";
		const char errorResponse[1000] = "GET /~carey/CPSC441/ass1/error.html HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n"; //hardcoded link to error page
		char *firstPartParsed;
        char *secondPartParsed;
        char *thirdPartParsed;
		char parsedHost[100];
		char parsedURL[100];
		char *pointerToParsedHost = parsedHost;
		char *pointerToParsedURL = parsedURL;
		char *blockfirstPartParsed;
		if (strstr(msgFromBrowser, post)) //To block any POST requests that were ruining the expected results
		{
			continue;
		}
		if (strstr(msgFromBrowser, fireFoxError)) //To block weird firefox messages that were ruining expected results
		{
			continue;
		}
		if ((strstr(blockRequests, blockSignal))) //uses strstr to compare what telnet client is sending to check if its a block command by comparing to *blockSignal
		{
			blockfirstPartParsed = strstr(blockRequests, blockSignal);
			blockfirstPartParsed += 6;
			char delim[2] = "!"; //used as a delimiter to get accurate word results
			char *tok;
			tok = strtok(blockfirstPartParsed, delim);
			blockedContent.push_back(tok); //if its a real BLOCK command, push back the word to end of vector<string> blockedContent 
		}
		if ((strstr(blockRequests, unblockSignal))) //uses strstr to compare what telnet client is sending to check if it is an unblock command by comparing to *unblockSignal
		{
			blockedContent.pop_back(); //if its a real UNBLOCK command, empties blockedContent
		}
		/*
		if the request is a real valid HTTP request, then parse URL and parse Host, URL will be used to check for blocked content, and Host will be 
		used in gethostbynameto get IP address of destination
		*/
		if ((strstr(msgFromBrowser, firstPart)) != NULL) 
		{
		firstPartParsed = strstr(msgFromBrowser, firstPart) + strlen(firstPart);
		secondPartParsed = strstr(firstPartParsed, secondPart);
		thirdPartParsed = strstr(secondPartParsed, space);
		strncpy(pointerToParsedHost, firstPartParsed, (strlen(firstPartParsed) - strlen(secondPartParsed))); //parsed Host
		strncpy(pointerToParsedURL, secondPartParsed, (strlen(secondPartParsed) - strlen(thirdPartParsed))); //parsed URL
		}
		else 
		{
			continue;
		}
		//printf("%s\n", parsedHost);
		//printf("%s\n", parsedURL);
		//Ok, parsedHost is now what i can convert to IP, and parsedURL is what i can use in the GET HTTP requests
		//make a new socket and struct for client that will send stuff to the server
		
		int proxyClientSocket; //socket for the proxy client that interacts with the web sites
		struct sockaddr_in webaddress; //webaddress sockaddr_in struct, used to specify port 80
		struct hostent *addy; //type hostent struct to use gethostbyname
		addy = gethostbyname(parsedHost); //converts hostname to IP address
		memset(&webaddress, 0, sizeof(webaddress));  //initialize address to 0
		webaddress.sin_family = AF_INET; //corresponds to IPv4 protocol as communication domain
		webaddress.sin_port = htons(80); //port 80 for HTTP
		bcopy((char *) addy->h_addr, (char *) &webaddress.sin_addr.s_addr, addy->h_length); //to put IP in sin_addr.s_addr (from powerpoint slides)
		//printf(inet_ntoa(webaddress.sin_addr));
		
		proxyClientSocket = socket(AF_INET, SOCK_STREAM, 0); //Socket creation for web proxy Client-side 
		if (proxyClientSocket == -1)
		{
			printf("socket() call failed\n");
			return 1;
		}
		int status;
		status = connect(proxyClientSocket, (struct sockaddr *)&webaddress, sizeof(struct sockaddr_in)); //Connection request to website
		if (status == -1){
			printf("connect() call failed\n");
			return 1;
		}
		printf(msgFromBrowser);
		//printf(parsedURL);
			if ((findBlockedWordInURL(pointerToParsedURL, blockedContent)) == true) //if URL has blocked words, change the msgFromBrowser (which is the HTTP request) to be a request for an error page
		{
				memset(msgFromBrowser, 0, strlen(msgFromBrowser));
				strcpy(msgFromBrowser, errorResponse);
		}
		printf(msgFromBrowser);
		printf("Connected\n");
		if (send(proxyClientSocket, msgFromBrowser, strlen(msgFromBrowser), 0 ) < 0) //Sending HTTP request to website
		{
			printf("Send failed\n");
			return 1;
		}
		//Receive a reply from the server (website)
		int websiteData = 0;
		while ((websiteData = recv(proxyClientSocket, msgFromWebServer, 20000 , 0)) > 0) //while loop so that images can load, websiteData holds the number of bytes
		{
		if (send(childSock, msgFromWebServer, websiteData, 0) < 0) //send reply from server back from proxy to client (browser)
		{
			printf("send failed\n");
			return 1;
		}
		}
			close(proxyClientSocket); //close the socket that connects to the website at the end of each loop
			close(childSock); //close the socket that is accepted from browser to proxy server at the end of each loop
	}
	close(telnChild); //Close all other sockets here, once program ends, to prevent bind() error failed errors.
	close(parentSock);
	close(telnParent);
	return 0;
}