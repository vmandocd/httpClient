#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "httpd.h"
#include "httpdPool.h"

using namespace std;

void usage(char * argv0)
{
	cerr << "Usage: " << argv0 << " listen_port docroot_dir" << endl;
}

int main(int argc, char *argv[])
{
        // Arguments can be 3 to 5 with thread pool extension
	if (argc < 3 || argc > 5) {
		usage(argv[0]);
		return 1;
	}

	long int port = strtol(argv[1], NULL, 10);

	if (errno == EINVAL || errno == ERANGE) {
		usage(argv[0]);
		return 2;
	}

	if (port <= 0 || port > USHRT_MAX) {
		cerr << "Invalid port: " << port << endl;
		return 3;
	}

	string doc_root  = argv[2];

        char *poolCheck = argv[3];

        // If no arg[3]
        if(argc == 3) {
          start_httpd(port, doc_root);
        }

        // If arg[3] is pool
        if(strcmp(poolCheck, POOL) == 0){
          long int poolSize = strtol(argv[4], NULL, 10);
          start_httpdPool(port, doc_root, poolSize);
        }

        // If nopool or 3 arguments
        else if(strcmp(poolCheck, NOPOOL) == 0) {
	  start_httpd(port, doc_root);
        }

        // Invalid argument for argv[3]
        else {
          cerr << "Invalid string: " << poolCheck << ", should be pool or nopool"<< endl;
          return 4;
        }

	return 0;
}
