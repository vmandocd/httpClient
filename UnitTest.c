#include <string.h>
#include "Practical.h"
#include "MsgProtocol.h"
#include "MsgEncoding.h"
#include "Framer.h"
 
int main() {
 
    printf("Begin Unit Tests: \n");

    // Encode Unit Test 
    printf("Encoding HTTPResponse struct as string: \n");
 
    HTTPResponse httpRes;
    memset(&httpRes, 0, sizeof(httpRes));
 
    httpRes.conLen   = 100;
    httpRes.code     = 200;
    httpRes.codeText = "OK\r\n";
    httpRes.version  = "HTTP/1.1";
    httpRes.server   = "Von's Server, ITS SO TIGHT\r\n";
    httpRes.lastMod  = "Wed, 6 Oct 6666 06:55:55 GMT\r\n";
    httpRes.conType  = "text/html\r\n";
 
    const char *request = "HTTP/1.1 200 OK\r\nServer: Von's Server, ITS SO TIGHT\r\nContent-Type: text/html\r\nContent-Length: 100\r\nLast-Modified: Wed, 6 Oct 6666 06:55:55 GMT\r\n\r\n";
 
    char outBuf[BUFSIZE];
    int reqSize = Encode(&httpRes, outBuf, BUFSIZE);
 
    printf("Expecting: \n");
    printf("%s", request);
 
    printf("Actual: \n");
    printf("%s", outBuf);

    // Decode Unit Test
    printf("Decoding HTTPResponse struct as string: \n");

    const char *decoded = "GET /index.html HTTP/1.1 Host suhduh\r\n";

    char inBuf[]  = "GET / HTTP/1.1\r\nHost: suhduh\r\nAnother: One\r\nAnother: One\r\n";
    char inBuf2[] = "GET / HTTP/1.1\r\nHost: suhduh\r\n  : One\r\nAnother: One\r\n";
    char inBuf3[] = "GET / HTTP/1.1\r\nHost: suhduh\r\nAnother:   \r\nAnother: One\r\n";
    char inBuf4[] = "GET / HTTP/1.1\r\nHost: suhduh\r\nAnother: O N E   \r\nAnother: One\r\n";
    char inBuf5[] = "GET / HTTP/1.1\r\nHost Key: suhduh\r\nAnother: One\r\nAnother: One\r\n";

    HTTPRequest httpReq;
    bool decodeVal;

    decodeVal = Decode(inBuf, &httpReq);

    printf("Expected for inBuf: \n");
    printf("%s", decoded);

    printf("Actual: \n");
    printf("%s %s %s %s %s\r\n", httpReq.method, httpReq.message, httpReq.version, httpReq.hostKey, httpReq.hostVal);

    decodeVal = Decode(inBuf2, &httpReq);

    printf("Expected for inBuf2: \n");
    printf("false\r\n");

    printf("Actual: \n");
    printf("%s\r\n", (decodeVal ? "true" : "false"));

    decodeVal = Decode(inBuf3, &httpReq);

    printf("Expected for inBuf3: \n");
    printf("false\r\n");

    printf("Actual: \n");
    printf("%s\r\n", (decodeVal ? "true" : "false"));

    decodeVal = Decode(inBuf4, &httpReq);

    printf("Expected for inBuf4: \n");
    printf("false\r\n");

    printf("Actual: \n");
    printf("%s\r\n", (decodeVal ? "true" : "false"));

    decodeVal = Decode(inBuf5, &httpReq);

    printf("Expected for inBuf5: \n");
    printf("false\r\n");

    printf("Actual: \n");
    printf("%s\r\n", (decodeVal ? "true" : "false"));
}
