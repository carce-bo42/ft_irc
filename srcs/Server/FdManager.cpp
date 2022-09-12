#include "Server/FdManager.hpp"
#include "Exceptions.hpp"
#include "Log.hpp"
#include "libft.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <unistd.h>
#include <string.h>

#include <iostream>
#include <cerrno>

using std::string;

namespace irc {

FdManager::FdManager(void)
:
    last_dynalloc_ip_address(0),
    fds_size(0)
{
    ft_memset(last_connection.ip_address, 0, sizeof(last_connection.ip_address));
    last_connection.new_fd = 0;
    if (setUpAddress() == -1
        || setUpListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
}

FdManager::FdManager(string &hostname, string &port)
:
    last_dynalloc_ip_address(0),
    fds_size(0)
{
    ft_memset(last_connection.ip_address, 0, sizeof(last_connection.ip_address));
    last_connection.new_fd = 0;
    if (setUpAddress(hostname, port) == -1
        || setUpListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
}

FdManager::FdManager(const FdManager& other)
:
    last_dynalloc_ip_address(other.last_dynalloc_ip_address),
    fds_size(other.fds_size),
    servinfo(other.servinfo),
    listener(other.listener)
{
    ft_memset(last_connection.ip_address, 0, sizeof(last_connection.ip_address));
    last_connection.new_fd = 0;
    for (int fd_idx=0; fd_idx < fds_size; fd_idx++) {
        fds[fd_idx].events = other.fds[fd_idx].events;
        fds[fd_idx].revents = other.fds[fd_idx].revents;
        fds[fd_idx].fd = other.fds[fd_idx].fd;
    }
}

FdManager::~FdManager(void) {
    if (servinfo != NULL) {
        freeaddrinfo(servinfo);
    }
    for (int fd_idx = 0; fd_idx < fds_size; fd_idx++) {
        if (skipFd(fd_idx)) {
            continue;
        }
        if (close(fds[fd_idx].fd) == -1) {
            throw irc::exc::FatalError("close -1");
        }
    }
    if (last_dynalloc_ip_address != NULL) {
        free(last_dynalloc_ip_address);
    }
}

static int get_addrinfo_from_params(const char* hostname,
                                    const char *port,
                                    struct addrinfo *hints,
                                    struct addrinfo **servinfo)
{
    int ret = -1;

    if ((ret = getaddrinfo(hostname, port, hints, servinfo)) != 0) {
        string error("getaddrinfo error :");
        LOG(ERROR) << error.append(gai_strerror(ret));
        return -1;
    }
    /* Filter out IPv6 cases (makes me dizzy) */
    if ((*servinfo)->ai_family == AF_INET6) {
        LOG(ERROR) << "unsupported IP address length";
        return -1;
    }
    return 0;
}

/*
 * sets all entries in internal struct addrinfo. This includes
 * hostname, the list of addresses to be used, the IRC port,
 * the transport protocol and the IP protocol.
 * Since this function is
 * essential to the server functionality, shuts down the 
 * program in case of failure.
 * 
 * returns 0 on success, 1 otherwise. 
 */
int FdManager::setUpAddress(void) {
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    char hostname[96];
    size_t hostname_len = 96;
    const char* port = "6667";

    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Ipv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (gethostname(&hostname[0], hostname_len) != 0) {
        LOG(ERROR) << "gethostname error";
        return -1;
    }
    if (get_addrinfo_from_params(hostname, port, &hints, &servinfo) == -1) {
        if (servinfo != NULL) {
            freeaddrinfo(servinfo);
            return -1;
        }
    }
    if (servinfo == NULL)
        return -1;
    /* This struct addrinfo may not be the one that binds, but
     * thats one more step from here. */
    this->servinfo = servinfo;
    return 0;
}

/*
 * Here ip works as the hostname. It is already null terminated
 * when calling string c_str method.
 */
int FdManager::setUpAddress(string &hostname, string &port) {
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;

    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Ipv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    if (get_addrinfo_from_params(hostname.c_str(), port.c_str(), &hints,
                                 &servinfo) == -1)
    {
        if (servinfo != NULL) {
            freeaddrinfo(servinfo);
            return -1;
        }
    }
    if (servinfo == NULL)
        return -1;
    /* This struct addrinfo may not be the one that binds, but
     * thats one more step from here. */
    this->servinfo = servinfo;
    return 0;
}

/*
 * tries to bind a socket to one of the provided addresses in
 * the struct addrinfo list provided by servinfo and then, it 
 * starts listen()ing to it.
 * return 0 on success, -1 otherwise.
 */
int FdManager::setUpListener(void) {

    int socketfd = -1;
    /* loop through addresses until bind works */
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        /* open a socket given servinfo */
        if ((socketfd = socket(p->ai_family,
                               p->ai_socktype,
                               p->ai_protocol)) == -1)
        {
            socketfd = -1;
            continue;
        }
        int yes = 1;
	    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) == -1)
        {
            freeaddrinfo(servinfo);
            LOG(ERROR) << "setsockopt raised -1";
            return -1;
        }
        /* assign port to socket */
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            if (close(socketfd) == -1) {
                freeaddrinfo(servinfo);
                return -1;
            }
            socketfd = -1;
            continue;
        }
        /* if it gets to this point everything should be fine */
        servinfo = p;
        break;
    }
    if (socketfd == -1) {
        LOG(ERROR) << "could not bind socket to any address";
        freeaddrinfo(servinfo);
        return -1;
    }
    if (listen(socketfd, LISTENER_BACKLOG) == -1) {
        LOG(ERROR) << "listen raised -1";
        freeaddrinfo(servinfo);
        return -1;
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)
                                      (servinfo->ai_addr);
    hostname = inet_ntoa(sockaddrin->sin_addr);
    /* debug, might not need it in the end */
    LOG(INFO) << "Server mounted succesfully on " << hostname << ":6667";
    listener = socketfd;
    return socketfd;
}

void FdManager::setUpPoll(void) {
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds_size++;
}

void FdManager::Poll(void) {
    /* -1 = wait until some event happens */
    if (poll(fds, fds_size, POLL_TIMEOUT_MS) == -1) {
        throw irc::exc::FatalError("poll -1");
    }
}

bool FdManager::hasDataToRead(int entry) {
    return (fds[entry].revents & POLLIN) ? true : false;
}

bool FdManager::skipFd(int fd_idx) {
    return (fds[fd_idx].fd == -1);
}

int FdManager::getFdFromIndex(int fd_idx) {
    return fds[fd_idx].fd;
}

/* calls accept, and prepares the fd returned to be polled correctly. 
 * Throws in case of fatal error.
 */
int FdManager::acceptConnection(void) {
    struct sockaddr_storage client;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int fd_new = -1;
    /* get new fd from accepted connection */
    if ((fd_new = accept(fds[0].fd, (struct sockaddr *)&client,
                         &addrlen)) == -1)
    {
        throw irc::exc::FatalError("accept -1");
    }
    /* set to non blocking fd */
    if (fcntl(fd_new, F_SETFL, O_NONBLOCK) == -1) {
        throw irc::exc::FatalError("fctnl -1");
    }
    int fd_new_idx = -1;
    for (int i=0; i < fds_size; i++) {
        /* If there's a -1 somewhere, add new user there. */
        if (fds[i].fd == -1) {
            fd_new_idx = i;
            break;
        }
    }
    /* If all entries are occupied, increase number of fd's */
    if (fd_new_idx == -1) {
        /* case server is at full users */
        if (fds_size == MAX_FDS) {
            if (close(fd_new) == -1) {
                throw irc::exc::FatalError("close -1");
            }
            return -1;
        }
        /* else just increment the size, and add at new last position */
        fds_size++;
        fd_new_idx = fds_size - 1;
    }
    /* set up fd for poll */
    fds[fd_new_idx].fd = fd_new;
    fds[fd_new_idx].events = POLLIN;
    fds[fd_new_idx].revents = 0;

    /* debug information */
    char ip_address[20];
    if (client.ss_family == AF_INET)  {
        struct sockaddr_in *ptr = (struct sockaddr_in *)&client;
        inet_ntop(AF_INET, &(ptr->sin_addr), ip_address, sizeof(ip_address));
    } else {
        struct sockaddr_in6 *ptr = (struct sockaddr_in6 *)&client;
        inet_ntop(AF_INET6, &(ptr->sin6_addr), ip_address, sizeof(ip_address));
    }
    LOG(INFO) << "connected to " << ip_address;

    // para guardar la ip para futuros baneitos 
    last_connection.new_fd = fd_new;
    memcpy(last_connection.ip_address, ip_address, ft_strlen(ip_address));

    return fd_new;
}

void FdManager::closeConnection(int fd) {

    for (int fd_idx = 0; fd_idx < fds_size; fd_idx++) {
        if (fds[fd_idx].fd == fd) {
            if (close(fds[fd_idx].fd) == -1) {
                throw irc::exc::FatalError("close -1");
            }
            fds[fd_idx].fd = -1;
            break ;
        }
    }
}

/* Some socket errors, specially on send() should not terminate
 * the program. */
int FdManager::getSocketError(int fd) {

    int err_code;
    socklen_t len = sizeof(err_code);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err_code, &len) == 0) {
        err_code = errno;
    } else {
        errno = err_code; // problem getting error code
    }
    return err_code;
}

bool FdManager::socketErrorIsNotFatal(int fd) {
    int error = getSocketError(fd);
    return (error == ECONNRESET || error == EPIPE);
}

/* 
 * returns an allocated copy of the last accepted connection
 * ip address, in case the fd passed coincides with the
 * socket provided byt this last accept() call.
 */
const char* FdManager::getSocketAddress(int fd) {
    if (last_connection.new_fd == fd) {
        if (last_dynalloc_ip_address != NULL) {
            free(last_dynalloc_ip_address);
            last_dynalloc_ip_address = NULL;
        }
        char* ret = ft_strdup(last_connection.ip_address);
        if (ret == NULL) {
            throw irc::exc::MallocError();
        }
        last_dynalloc_ip_address = ret; // to free later.
        return ret;
    }
    return NULL;
}


} //namespace
