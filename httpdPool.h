#ifndef HTTPDPOOL_H
#define HTTPDPOOL_H

#include <string.h>
#include <stdlib.h>

using namespace std;

const char *POOL   = "pool";
const char *NOPOOL = "nopool";

void start_httpdPool(unsigned short port, string doc_root, long int poolSize);

#endif // HTTPD_H
