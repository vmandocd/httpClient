#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sendfile.h>
#include "Practical.h"
#include "SendError.h"
#include "Framer.h"
#include "MsgEncoding.h"

#define MODIFIEDLEN 29
#define OK          200
#define CLNTERR     400
#define FORBIDDEN   403
#define NOTFOUND    404

  static const char *SERVERNAME   = "Neat\r\n";
  static const char *OKTXT        = "OK\r\n";
  static const char *CLNTERRTXT   = "Client Error\r\n";
  static const char *FORBIDDENTXT = "Forbidden\r\n";
  static const char *NOTFOUNDTXT  = "Not Found\r\n";
  static const char *HTML         = "html";
  static const char *JPG          = "jpg";
  static const char *PNG          = "png";
  static const char *HTMLTYPE     = "text/html\r\n";
  static const char *JPGTYPE      = "image/jpeg\r\n";
  static const char *PNGTYPE      = "image/png\r\n";
  static const char *defaultDoc   = "/";

void *ThreadMain(void *threadArgs) {
  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());

  // Extract socket file descriptor from argument
  int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
  std::string doc_root = ((struct ThreadArgs *) threadArgs)->doc_root;

  // Deallocate memory for argument
  free(threadArgs);

  HandleTCPClient(clntSock, doc_root);

  return (NULL);
}

void *ThreadMainPool(void *threadArgs) {
  // Extract socket file descriptor from argument
  int servSock = ((struct ThreadArgs *) threadArgs)->clntSock;
  std::string doc_root = ((struct ThreadArgs *) threadArgs)->doc_root;

  // Deallocate memory for argument
  free(threadArgs);

  for(;;) {
  	int clntSock = AcceptTCPConnection(servSock);
  	HandleTCPClient(clntSock, doc_root);
  }

  return (NULL);
}

// Handle the socket and recv from client to fill in buffer
void HandleTCPClient(int clntSocket, std::string doc_root) {
  char *typeCheck;                                 // Checks what type the content is
  char buffer[BUFSIZE];                            // Buffer for echo string
  char outBuf[BUFSIZE];                            // Bufer for encode
  char resolvedPath[PATH_MAX];                     // Buffer to hold path from doc_root and url
  bool reqCheck;                                   // Checks if request is valid
  int  msgCheck;                                   // Checks if GetNextMsg returned correctly
  int  encodeCheck;                                // Checks what value encode returns
  std::string path;                                // String to hold the path to open for request
  HTTPRequest req;                                 // HTTPRequest struct to hold info of request
  HTTPResponse resp;                               // HTTPResponse struct to hold info for response
  struct stat buf;                                 // Stat struct to check if request path is readable
  FILE *socketFile    = fdopen(clntSocket, "r+");  // Wrapper for clntSocket
  const char *docRoot = doc_root.c_str();          // Create a const char to hold doc_root string

  // Read from the clntSocket and fill in buffer with the request 
  msgCheck = GetNextMsg(socketFile, buffer, BUFSIZE);

  // Check if GetNextMsg was valid
  if(msgCheck < 0) {
    sendError(&resp, CLNTERR, CLNTERRTXT, HTMLTYPE);

    // Encode the response then send it and close files
    encodeCheck   = Encode(&resp, outBuf, BUFSIZE);
    PutMsg(outBuf, encodeCheck, socketFile);
    fclose(socketFile); // Close socket file
    close(clntSocket);  // Close client socket
  }

  else {
    // Check if buffer was successfully put into HTTPRequest
    reqCheck = Decode(buffer, &req); 

    // Set the response fields that can be set already accordingly
    resp.version = req.version;
    resp.server = SERVERNAME;

    // If reqCheck is false, send back 400 error
    if(!reqCheck) {
      sendError(&resp, CLNTERR, CLNTERRTXT, HTMLTYPE);
    }

    // If document root is just '/' then just make message the path
    if(strcmp(docRoot, defaultDoc) == 0) {
      path = req.message;
    }
    // Otherwise prepend the document root to the message for path
    else {
      path = doc_root + req.message;
    }

    // Convert the string into a constant char pointer
    const char *fullPath = path.c_str();
    errno = 0;
  
    // Check if the path given exists, otherwise return 404
    if(realpath(fullPath, resolvedPath) == NULL && resp.code == 0) {
      sendError(&resp, NOTFOUND, NOTFOUNDTXT, HTMLTYPE);
    }
  
    // Check if path contains document root, otherwise return 403
    if(strncmp(docRoot,resolvedPath, strlen(docRoot)) != 0 && resp.code == 0) {
      sendError(&resp, FORBIDDEN, FORBIDDENTXT, HTMLTYPE);
    }
  
  
    /* Now that we have the path to the file, we will want to 
     * open it to read only and see if it will open to an actual page
     * If it returns NULL, send NOT FOUND
     * If errno is 13, send FORBIDDEN
     */
    FILE *filePath = fopen(resolvedPath, "r");
  
    // Check if file is NULL
    if(filePath == NULL && resp.code == 0) {
      sendError(&resp, NOTFOUND, NOTFOUNDTXT, HTMLTYPE);
    }
  
    // Chek if errno set to no access
    if(errno == 13 && resp.code == 0) {
      sendError(&resp, FORBIDDEN, FORBIDDENTXT, HTMLTYPE);
    }
  
    stat(resolvedPath, &buf);
  
    // Check if readable, if not throw 403
    if(!(buf.st_mode & S_IROTH) && resp.code == 0) { 
      sendError(&resp, FORBIDDEN, FORBIDDENTXT, HTMLTYPE);
    }

    /* If the code gets to this point and the response code is not
     * 0, then it has been set to an error value and the response is done.
     * Otherwise continue on to opening the path as a FILE
     */
    if(resp.code != 0) {
      // Encode the response then send it and close files
      encodeCheck    = Encode(&resp, outBuf, BUFSIZE);
      PutMsg(outBuf, encodeCheck, socketFile);
      fclose(socketFile); // Close socket file
      close(clntSocket);  // Close client socket
    }

    /* Once we get to thise point, we know that the response code
     * did not error, so now we set code to 200 and the code text
     * to OK then continue filling in the rest of the response struct
     */
    else {
      resp.code     = OK;
      resp.codeText = OKTXT;
  
      // To get the last modified time
      char date[MODIFIEDLEN];
      struct tm *gmt = gmtime(&(buf.st_mtime));
  
      strftime(date, sizeof(date),"%a, %d %b %y %T GMT\r\n", gmt);
      resp.lastMod = date;
  
      // To get content type use strstr to look for '.' 
      typeCheck = strstr(resolvedPath, ".");
      typeCheck++;
  
      // If content type not found return 400
      if(typeCheck == NULL) {
        sendError(&resp, CLNTERR, CLNTERRTXT, HTMLTYPE);
      }
      else {
        // To get content length use the file size from struct stat
        resp.conLen = buf.st_size;
  
        // Compare the type if it matches HTML
        if (strcmp(typeCheck, HTML) == 0) {
          resp.conType = HTMLTYPE;
        }
        // Compare the type if it matches JPG
        else if (strcmp(typeCheck, JPG) == 0) {
          resp.conType = JPGTYPE;
        }
        // Compare the type if it matches PNG
        else if (strcmp(typeCheck, PNG) == 0) {
          resp.conType = PNGTYPE;
        }
        // Send client error if none match
        else {
          sendError(&resp, CLNTERR, CLNTERRTXT, HTMLTYPE);
        }
      }
  
      // Encode the response then send it
      encodeCheck = Encode(&resp, outBuf, BUFSIZE);

      // Put the message from outBuf into the file to be sent
      PutMsg(outBuf, encodeCheck, socketFile);
  
      // Check if code is 200 before sending the file to the client
      if(resp.code == OK) {
        // Send the file from path back through the client socket only if successful
        sendfile(fileno(socketFile), fileno(filePath), NULL, resp.conLen);
      }
  
      fclose(filePath);   // Close the file that opened from path
      fclose(socketFile); // Close socket file
      close(clntSocket);  // Close client socket
    }
  }
}
