#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>

#include "Server/Server.hpp"
#include "Log.hpp"


/**
 * https://modern.ircdocs.horse/ for numeric replies
 * 
 * To anyone wondering how exceptions and memory deallocation on destructors
 * work :
 * Yes, destructors are guaranteed to be called on stack unwinding,
 * including unwinding due to exception being thrown.
 * There are only few tricks with exceptions that you have to remember:
 * Destructor of the class is not called if exception is thrown in its
 * constructor.
 *
 * Para conectarse a irc hispano :
 * nc irc.irc-hispano.org 6667
 */

bool running = true;

using namespace irc;
using std::string;

void error(string msg) {
    std::cout << msg << std::endl;
    exit(1);
}

/* n = 1 : port
 * n = 2 : host addr
 * n = 3 : server password (?) // no server password 
 */
int main(int argc, char **argv) {
    try {
        if (argc == 1) {
            Server server;
            return 0;
        } else if (argc == 2) { // only password set ! 
            string password = argv[1];
            Server server(password);
            return 0;
        } else if (argc == 3) {
            string hostname(argv[1]);
            string port(argv[2]);
            Server(hostname, port);
            // connect to server using params
            return 0;
        } else if (argc == 4) {
            string hostname(argv[1]);
            string port(argv[2]);
            string password = argv[3];
            Server(hostname, port, password);
            return 0;
        } else {
            return 42;
        }
    } catch (...) {
        LOG(ERROR) << "Error setting up server";
        return 42;
    }
}
