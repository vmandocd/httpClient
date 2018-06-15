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

using namespace std;

void start_httpdPool(unsigned short port, string doc_root, long int poolSize)
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
  if (listen(servSock, poolSize+1) < 0)
    DieWithSystemMessage("listen() failed");

  // Allocate memory for threads based on poolSize
  pthread_t * threads = (pthread_t *) malloc(poolSize * sizeof(pthread_t));

  for (int i = 0; i < poolSize; i++) {

    // Create separate memory for client argument
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(
        sizeof(struct ThreadArgs));

    if (threadArgs == NULL)
      DieWithSystemMessage("malloc() failed");

    threadArgs->clntSock = servSock;
    threadArgs->doc_root = doc_root;

    // Create client thread
    int returnValue = pthread_create(&threads[i], NULL, ThreadMainPool, threadArgs);

    if (returnValue != 0)
      DieWithUserMessage("pthread_create() failed", strerror(returnValue));

    printf("with thread %ld\n", (long int) threads[i]);
  }

  for (int i = 0; i < poolSize; i++) {
  	pthread_join(threads[i], NULL);
  }

  free(threads);
  // Not reached
}
