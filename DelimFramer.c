#include <stdio.h>
#include <stdint.h>
#include "Practical.h"
#include "Framer.h"

static const char DELIMITER  = '\r';
static const char DELIMITER2 = '\n';

/* Read up to bufSize bytes or until delimiter, copying into the given
 * buffer as we go.
 * Encountering EOF after some data but before delimiter results in failure.
 * (That is: EOF is not a valid delimiter.)
 * Returns the number of bytes placed in buf (delimiter NOT transferred).
 * If buffer fills without encountering delimiter, negative count is returned.
 * If stream ends before first byte, -1 is returned.
 * Precondition: buf has room for at least bufSize bytes.
 */
int GetNextMsg(FILE *in, char *buf, size_t bufSize) {
  size_t count = 0;
  int delimCount = 0;
  int nextChar, nextChar1;
  while (count < bufSize) {
    nextChar = getc(in);

    // Check if EOF was reached when reading the request
    if (nextChar == EOF) {
      if (count > 0)
        DieWithUserMessage("GetNextMsg()", "Stream ended prematurely");
      else {
        return -1;
      }
    }

    // Look for the DELIMITER in the message
    if (nextChar == DELIMITER) {

      buf[count++] = nextChar;

      nextChar1 = getc(in);
      // Check if EOF was reached or if \n is not read
      // Return -1 if either occur
      if (nextChar1 == EOF) {
        DieWithUserMessage("GetNextMsg()", "Stream ended prematurely");
      }
      else if(nextChar1 != DELIMITER2) {
        return -1;
      }
      
      // If you get to this point, CRLF was found so put them in buffer
      buf[count++] = nextChar1;

      // Check if CRLF found twice, then break out
      if(delimCount == 1) {
        break;
      }

      // Set delimCount to 1 so we know that CRLF found once
      delimCount = 1;

    }

    else {
      // If delimCount is 1, then set to 0  and don't put into buffer
      // since we know two CRLF's in a row not found and there's more request
      delimCount = 0;
    }

    // Check if CRLF was never found
    if (delimCount == 0)
      buf[count++] = nextChar;

  }

  if (nextChar != DELIMITER) { // Out of space: count==bufSize
    return -count;
  } else { // Found delimiter
    return count;
  }
}

/* Write the given message to the output stream, followed by
 * the delimiter.  Return number of bytes written, or -1 on failure.
 */
int PutMsg(char buf[], size_t msgSize, FILE *out) {
  // Check for delimiter in message
  size_t i;
  int delimCheck = 0;

  // Loop through buffer 
  for (i = 0; i < msgSize; i++) {
    // If CRLF found twice, break out of the loop
    if (buf[i] == DELIMITER) {
      if(buf[i+1] == DELIMITER2 && delimCheck == 1) {
        break;
      }
      else{
        delimCheck = 1;
      }
    }
    else {
      delimCheck = 0;
    }
  }

  // Check if fwrite was able to write all of it
  if (fwrite(buf, 1, msgSize, out) != msgSize) {
    printf("fwrite failed\n");
    return -1;
  }

  // End with CRLF
  fputc(DELIMITER, out);
  fputc(DELIMITER2, out);
  fflush(out);

  return msgSize;
}
