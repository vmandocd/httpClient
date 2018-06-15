struct HTTPRequest {
  char *method;   // GET
  char *message;  // Request url
  char *version;  // Version of 
  char *hostKey;  // Host
  char *hostVal;  // Value of Host
};

struct HTTPResponse {
  int conLen;            // Content length
  int  code;             // 200, 400, 403, or 404
  const char *codeText;  // Message for the code 
  const char *version;   // Version
  const char *server;    // Server name
  const char *lastMod;   // Last modified
  const char *conType;   // Content type
};

typedef struct HTTPRequest HTTPRequest;
typedef struct HTTPResponse HTTPResponse;

enum{
  MAX_METHOD = 3,  // Only accepting GET
  MAX_VERSION = 8  // Only accepting HTTP/1.1
};
