#include "SocketObject.h"

/**
 * Set-up the socket, incl. getaddrInfo, create socket object by acquiring
 * socket file descriptor value.
*/
void SocketObject::setupSocket() {
    memset(&hints, 0, sizeof(hints));

    // use IPv4 or IPv6, whichever
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    // fill in my IP for me
    hints.ai_flags = AI_PASSIVE;   

    int result;
    if((result = getaddrinfo(NULL, port.c_str(), &hints, &res)) != 0){
        throw SocketException("Failure in getaddrinfo"); 
    }

    if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
        throw SocketException("Failure in socket creation"); 
    }
}

/**
 * Bind and listen to specific port.
 */
void SocketObject::bindAndListenToPort() {
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        throw SocketException("Failure in socket setsockopt"); 
    }

    //binding results
    int bindResult;
    if((bindResult = bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
        throw SocketException("Failure in socket bind"); 
    }

    //listening to the port
    int listeningResult;
    if((listeningResult = listen(sockfd, BACKLOG)) == -1){
        throw SocketException("Failure in socket listen"); 
    }
}

/**
 * Accept on the port: block the caller until incoming requests
*/
int SocketObject::acceptPort() {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int newSocketFd;
    if((newSocketFd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1){
        throw SocketException("Failure in socket listen"); 
    }
    return newSocketFd;
}

/**
 * Establish socket connection with target server
*/
int SocketObject::setupAndConnect(){
    //setup phase
    memset(&hints, 0, sizeof(hints));
    // use IPv4 or IPv6, whichever
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    
    int result_getaddr;
    //get address information
    if((result_getaddr = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res)) != 0){
        throw SocketException("Failure in getaddrinfo"); 
    }

    struct addrinfo * p = NULL;
    for(p = res; p != NULL; p = p->ai_next) {
        // try setting socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        // try connecting socket
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    return sockfd;
}
