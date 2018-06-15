#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string>

// Handle error with user msg
void DieWithUserMessage(const char *msg, const char *detail);
// Handle error with sys msg
void DieWithSystemMessage(const char *msg);
// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
void HandleTCPClient(int clntSocket, std::string doc_root);
// Create and connect a new TCP client socket
int SetupTCPClientSocket(const char *server, const char *service);
// Main program of a thread with a thread pool
void *ThreadMainPool (void *arg);
// Main program of a thread
void *ThreadMain (void *arg);

// Structure of arguments to pass to client thread
struct ThreadArgs {
  int clntSock;    // Socket descripter for client
  std::string doc_root; // Document root for path
};

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 8192
};

#endif // PRACTICAL_H_
