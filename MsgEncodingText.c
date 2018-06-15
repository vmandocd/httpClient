/* Routines for Text encoding of request messages.
 * Wire Format:
 *   Method URL Version"
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Practical.h"
#include "MsgProtocol.h"

static const char *MAGIC     = "GET";
static const char *REQSTR    = "/";
static const char *VERSION   = "HTTP/1.1";
static const char *HOST      = "Host";
static const char *DELIMSTR  = " ";
static const char *DELIMCRLF = "\r\n";
static const char *DELIMPAIR = ": ";

/* Encode request message info as a text string for response.
 * WARNING: Message will be silently truncated if buffer is too small!
 * Invariants (e.g. 0 <= candidate <= 1000) not checked.
 */
size_t Encode(HTTPResponse *v, char *outBuf, const size_t bufSize) {
  char *bufPtr = outBuf;
  long size = (size_t) bufSize;
  int rv = snprintf((char *) bufPtr, size, "%s %d %sServer: %sContent-type: %sContent-Length: %d\r\n",
      VERSION, v->code, v->codeText, v->server, v->conType, v->conLen);
  bufPtr += rv;
  size -= rv;
  if(v->lastMod == NULL) {
    rv = snprintf((char *) bufPtr, size, "\r\n");
    bufPtr += rv;
  }
  else {
    rv = snprintf((char *) bufPtr, size, "Last-Modified: %s\r\n", v->lastMod);
    bufPtr += rv;
  }

  return (size_t) (bufPtr - outBuf);
}


/* Extract message information from given buffer.
 * Note: modifies input buffer.
 */
bool Decode(char *inBuf, HTTPRequest *v) {

  // Checks for string length of value in key value pair
  int valueLen1 = 0;
  int valueLen2 = 0;

  char *token;
  char *pairToken;

  // Look for spaces in the first line of the request for GET, url, and version
  token = strsep(&inBuf, DELIMSTR);
  // Check for magic (method)
  if (token == NULL || strcmp(token, MAGIC) != 0 || strlen(token) == 0)
    return false;  // Method is not GET

  // Set method to GET
  v->method = token;

  // Get the url
  token = strsep(&inBuf, DELIMSTR);
  if (token == NULL || strlen(token) == 0)
    return false;

  if (strcmp(token, REQSTR) == 0) {
    char index[] = "/index.html";
    token = (char *) malloc(strlen(index)+1);
    strcpy(token, index);
  }

  // Set message to parsed url
  v->message = token;

  // Next token is the version
  token = strsep(&inBuf, DELIMCRLF);
  if (token == NULL || strcmp(token, VERSION) != 0 || strlen(token) == 0) {
    return false; // Version not correct
  }

  // Set version to HTTP/1.1
  v->version = token;

  // Get rid of the CRLF after initial line, then look for key value pairs
  token = strsep(&inBuf, DELIMCRLF);

  // Check if buffer contains any key value pairs
  token = strsep(&inBuf, DELIMCRLF);
  if (token == NULL || strlen(token) == 0) {
    return false; // Message missing key value pairs
  }

  /* Since strsep returns an empty string of length 0 when
   * reading the delimiter twice in a row, we will know
   * that the buffer no longer contains any more key value
   * pairs when string length is 0
   */
  while(strlen(token) != 0) {
    // Look for the delimiter for the pair
    pairToken = strsep(&token, DELIMPAIR);
    if (pairToken == NULL) {
      return false; // 400 Error no colon space
    }

    // Get the string length of the token
    valueLen1 = strlen(pairToken);

    // Make sure there's no space after key
    pairToken = strsep(&pairToken, DELIMSTR);
    valueLen2 = strlen(pairToken);

    if (pairToken == NULL || strlen(pairToken) == 0 || valueLen1 != valueLen2) {
      return false; // Host key was not found
    }

    /* Check if the key is the Host key
     * If it is then set the host parameters for the struct v
     * It it's not ignore and keep parsing
     */
    if(strcmp(pairToken, HOST) == 0) {
      // Set hostKey to the pairToken and check for value
      v->hostKey = pairToken;

      // Check pairToken for hostVal
      pairToken = strsep(&token, DELIMCRLF);
      pairToken++;
      valueLen1 = strlen(pairToken);

      // Make sure there's no space after key value
      pairToken = strsep(&pairToken, DELIMSTR);
      valueLen2 = strlen(pairToken);
      if (pairToken == NULL || strlen(pairToken) == 0 || valueLen1 != valueLen2) {
        return false; // Host value was not found
      }
      v->hostVal = pairToken;
    }

    /* The key is not the Host key
     * Now check for the value for the key
     */
    else{ 
      pairToken = strsep(&token, DELIMCRLF);
      pairToken++;
      valueLen1 = strlen(pairToken);

      // Make sure there's no space after key value
      pairToken = strsep(&pairToken, DELIMSTR);
      valueLen2 = strlen(pairToken);

      if (pairToken == NULL || strlen(pairToken) == 0 || valueLen1 != valueLen2) {
        return false; // Host value was not found, use 400 error
      }
    }

    // Check if buffer contains any more key value pairs
    token = strsep(&inBuf, DELIMCRLF);
    token = strsep(&inBuf, DELIMCRLF);

  } // end while

  // Check if there's a host, otherwise send false for 400
  if(v->method == NULL)
    return false;

  return true;
}
