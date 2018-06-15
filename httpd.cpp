#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "httpd.h"
#include "Practical.h"
#include "Framer.h"

#define MAXPENDING  5

using namespace std;

void start_httpd(unsigned short port, string doc_root)
{
	cerr << "Starting server (port: " << port <<
		", doc_root: " << doc_root << ")" << endl;

  // Create socket for incoming connections
  int servSock; // Socket descriptor for server
  if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithSystemMessage("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr;			// Local address
  memset(&servAddr, 0, sizeof(servAddr));	// Zero out structure
  servAddr.sin_family = AF_INET;		// IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// Any incoming interface
  servAddr.sin_port = htons(port);		// Given port	

  // Bind to the local address
  if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithSystemMessage("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if (listen(servSock, MAXPENDING) < 0)
    DieWithSystemMessage("listen() failed");

  for (;;) { // Run forever
    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
      DieWithSystemMessage("socket() failed");

    // clntSock is connected to a client!

    char clntName[INET_ADDRSTRLEN]; // String to contain client address
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
        sizeof(clntName)) != NULL)
      printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
      puts("Unable to get client address");

    // Create separate memory for client argument
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(
        sizeof(struct ThreadArgs));
    if (threadArgs == NULL)
      DieWithSystemMessage("malloc() failed");
    threadArgs->clntSock = clntSock;
    threadArgs->doc_root = doc_root;

    // Create client thread
    pthread_t threadID;
    int returnValue = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
    if (returnValue != 0)
      DieWithUserMessage("pthread_create() failed", strerror(returnValue));
    printf("with thread %ld\n", (long int) threadArgs);
  }
  // Not reached
}
