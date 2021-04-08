#ifndef _SOCKET_OBJECT_H
#define _SOCKET_OBJECT_H
#include <cstdlib>
#include <unistd.h>
#include "SocketException.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>

#define BACKLOG 10
#define LISTEN_PORT "12345"
#define HOST "127.0.0.1"

/**
 * Socket object representing socket.
 * 
 * @author Yichen 
 * @date 2021.02.27
*/
class SocketObject {
private:
    int sockfd;
    struct addrinfo hints;
    struct addrinfo* res;
    std::string hostname;
    std::string port;

public:
    SocketObject() 
        : sockfd(0), hints(), res(nullptr), hostname(HOST), port(LISTEN_PORT){}

    SocketObject(const std::string & hostname, const std::string & port) 
        : sockfd(0), hints(), res(nullptr), hostname(hostname), port(port){}

    ~SocketObject(){
        freeaddrinfo(res);
    }

    /**
     * Set-up the socket, incl. getaddrInfo, create socket object by acquiring
     * socket file descriptor value.
    */
    void setupSocket();

    /**
     * Bind and listen to specific port.
    */
    void bindAndListenToPort();

    /**
     * Accept on the port: block the caller until incoming requests
     * 
     * @returns socket fd of newly created socket object.
    */
    int acceptPort();

    /**
     * Establish socket connection with target server
    */
    int setupAndConnect();

};

#endif
