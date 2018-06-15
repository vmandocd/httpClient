#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

bool Decode(char *inBuf, HTTPRequest *v);
size_t Encode(HTTPResponse *v, char *outBuf, size_t bufSize);
