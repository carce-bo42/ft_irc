#ifndef IRC42_FDMANAGER_H
# define IRC42_FDMANAGER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include <string>
#include "Types.hpp"

namespace irc {

/* 
 * Esta clase sirve de base de datos para los sockets del servidor.
 * Se encarga de inicializar el que está en esucha, de trabajar con
 * la estructura de poll, y de aceptar y derivar conexiones de forma
 * agnóstica: ni lee ni escribe de los sockets.
 */

class FdManager {

    public:
    FdManager(void);
    FdManager(std::string &ip, std::string &port);
    FdManager(const FdManager &other);
    ~FdManager();

    /* Setup */
    int setUpAddress(void);
    int setUpAddress(std::string &hostname, std::string &port);
    int setUpListener(void);
    void setUpPoll(void);

    /* main utils */
    void Poll(void);

    int acceptConnection(void);
    const char* acceptConnection(int *fd) ;
    void closeConnection(int fd_idx);

    /* accessors */
    bool hasDataToRead(int entry);
    bool skipFd(int fd_idx);
    int getFdFromIndex(int fd_idx);

    /* socket error helpers */
    bool socketErrorIsNotFatal(int fd);
    int getSocketError(int);
    /* fd from clients manager. This includes
    * the listener, at entry 0. */

   
    /* Temporarily saves information about the
     * most recently accepted connection. */
    typedef struct ConnInfo {
        int new_fd;
        char ip_address[20];
    } ConnInfo;

    // whoever allocates, deallocates.
    char* last_dynalloc_ip_address;

    // gets a duplicate from the ip_address stored in last_connection.
    const char* getSocketAddress(int fd);

    ConnInfo last_connection;

    struct pollfd fds[255]; // MAX_FDS
    int fds_size;
    struct addrinfo *servinfo;
    int listener;
    std::string hostname;
};

}

#endif /* IRC42_FDMANAGER_H */
