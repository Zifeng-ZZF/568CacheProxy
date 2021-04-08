#include "Proxy.h"

/**
 * Constructor
 */
Proxy::Proxy(): workers(), parser(), workerFdMap(), logger() {
    try {
        logger.openFileStream("/var/log/erss/proxy.log");
    }
    catch (SocketException & e) {
        cout << "[LOGGER] open failed" << endl;
    }
}

/**
 * Destructor
 */
Proxy::~Proxy() {
    // cout << "Destroy proxy" << endl;
}

/**
 * run
 * ----
 * start the server on default port
 * entry of the proxy
 */
void Proxy::run() {
    signal(SIGPIPE, SIG_IGN); // closing SIGPIPE
    auto socketObj = std::make_shared<SocketObject>(); 
    try {
        socketObj->setupSocket();
        socketObj->bindAndListenToPort();
    }
    catch(SocketException & e) {
        std::cerr << e.what() << '\n';
        return;
    }
    catch(std::exception & e) {
        std::cerr << e.what() << '\n';
        return;
    }
    listen(socketObj);
}

/**
 * listen
 * --------
 * start the listening of specified socket object, handle any incomming 
 * requests in any connection accepted by the socket
 */
void Proxy::listen(std::shared_ptr<SocketObject> socketObj) {
    cout << "Listening.." << endl;
    cout << " ---- to kill daemon: ps -el | grep proxy & kill pid" << endl;
    try {
        while (true) {
            int fd = socketObj->acceptPort();
            // cout << "Got a connecton at fd=" << fd << endl;
            this->workers.push_back(std::thread([=] { 
                workerFdMap[std::this_thread::get_id()] = fd;
                this->handleRequest(fd); 
            }));
        }
    }
    catch(SocketException & e) {
        std::cerr << "[FATAL] listening exception " << e.what() << endl;
    }
    catch(std::exception & e) {
        std::cerr << "[FATAL] listening exception " << e.what() << endl;
    }
}

/**
 * handleRequest
 * -------------
 * Entry for handle accepted connection's request
 */
void Proxy::handleRequest(int fd) {
    std::vector<char> buff(INI_SIZE);
    std::shared_ptr<HttpRequest> req;
    try {
        readInData(fd, buff);
        requestHandler((req = parser.getRequest(string(buff.data()))), buff);
        workerFdMap.erase(std::this_thread::get_id());
        close(fd);
    }
    catch(HttpException & e) {
        // send back Bad response and quit
        cout << "[HTTP]" << e.what() << endl;
        string bad = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n";
        if (send(fd, bad.c_str(), bad.size()+1, 0) == -1) {
            cerr << "Bad socket send" << endl;
        }
        close(fd);
    }
    catch(HttpRequestSyntaxException & e) {
        string msg = e.getRespnse();
        if (send(fd, msg.c_str(), msg.size()+1, 0) == -1) {
            cerr << "Bad socket send" << endl;
        }
        string log = std::to_string(req->getRequestID()) + ": Responding" + "RESPONSE" + "\n";
        logger.writeFile(log);
        close(fd);
    }
    catch(HttpResponseSyntaxException & e) {
        string msg = e.getRespnse();
        if (send(fd, msg.c_str(), msg.size()+1, 0) == -1) {
            cerr << "Bad socket send" << endl;
        }
        close(fd);
    }
    catch(std::exception & e) {
        cout << "other -- " << e.what() << endl;
    }
}

/**
 * readInData
 * ----------
 * read in / partial parsing http request
 */
void Proxy::readInData(int fd, std::vector<char> & buff) {
    // cout << "Thread " << std::this_thread::get_id() << " starts reading ..." << endl;
    size_t idx = 0, headerEnd = 0;
    bool hasBody = false;
    int restBytes = 0;
    while (true) {
        size_t byteCnt = recv(fd, &buff.data()[idx], buff.size()-idx, 0);
        idx += byteCnt;
        if (byteCnt >= 0) {
            string str(buff.data());            
            if ((headerEnd = str.find("\r\n\r\n")) != string::npos) {
                size_t pos = str.find("Content-Length:");      
                if (pos == string::npos) { //no msg body, done looping
                    break;
                }
                string resline = str.substr(pos);
                size_t numPos = resline.find_first_of(':') + 2;
                size_t lbrPos = resline.find_first_of('\r');
                if (!hasBody) {
                    restBytes = std::stoi(resline.substr(numPos, lbrPos-numPos));
                    int currContBytes = idx - headerEnd - 4;
                    if (currContBytes >= restBytes) {
                        break;
                    }
                    else {
                        restBytes = headerEnd + restBytes - idx + 4;
                        restBytes += byteCnt;
                    }
                    hasBody = true;
                }
                else {
                    restBytes -= byteCnt;
                    if (restBytes <= 0) { //body finish
                        break;
                    }
                }
            }
        }
        else {
            throw SocketException("Failure in receiving data");
        }
        if (idx == buff.size()) {
            buff.resize(2 * buff.size());
        }
    }
    // cout << "Receive data of " << idx << " bytes:\n" << string(buff.data()) << endl;
}

/**
 * readPlainMessage
 * -----------------
 * Read in plain data without parsing. Used for connecting switching 
 * data / reading response.
 * 
 * fd: socket fd to read from
 * buff: data to be written into
 */
void Proxy::readPlainMessage(int fd, std::vector<char> & buff) {
    buff.resize(MAX_REC);
    int idx = 0, cnt = 0;
    while (true) {
        cnt = recv(fd, &buff.data()[idx], MAX_REC, 0);
        idx += cnt;
        if (cnt > 0) {
            if (cnt < MAX_REC) {
                buff.resize(idx);
                break;
            }
            else {
                buff.resize(buff.size() + MAX_REC);
            }
        }
        else if (cnt == 0) {
            break;
        }
        else {
            throw SocketException("Failure in receving data in reading plain message");
        }
    }
}

/**
 * sendData
 * -----------
 * Send data to certain socket. 
 */
void Proxy::sendData(int fd, std::vector<char> & data) {
    if (fcntl(fd, F_GETFD) == -1) {
        throw SocketFdException("fd used is no longer valid.");
    }
    if (send(fd, data.data(), data.size(), 0) == -1) {
        throw SocketException("Sending fail");
    }
}

/**
 * sendData
 * -----------
 * Send data to certain socket. 
 */
void Proxy::sendData(int fd, const string & data) {
    if (fcntl(fd, F_GETFD) == -1) {
        throw SocketFdException("fd used is no longer valid.");
    }
    if (send(fd, data.c_str(), data.size(), 0) == -1) {
        throw SocketException("Sending fail");
    }
}

/**
 * requestHandler
 * ---------------
 * handle parsed request based on request methods
 */
void Proxy::requestHandler(std::shared_ptr<HttpRequest> req, std::vector<char> & data) {
    try {
        if (req->method == Method::GET) {
            methodGET(req, data);
        }
        else if (req->method == Method::POST) {
            methodPOST(req, data);
        }
        else if (req->method = Method::CONNECT) {
            methodCONNECT(req, data);
        }
    }
    catch(SocketException & e) {
        cout << "[requestHandler] " << e.what() << endl;
    }
    catch(SocketFdException & e) {
        cout << "[requestHandler] invalid fd, closing conn" << e.what() << endl;
    }
    catch(SocketTimeoutException & e) {
        string msg = std::to_string(req->getRequestID()) + ": Tunnel closed\n";
        logger.writeFile(msg);
    }
}

/**
 * responseHandler
 * 
 */
void Proxy::responseHandler(std::shared_ptr<HttpResponse> resp, std::vector<char> & data, std::shared_ptr<HttpRequest> originalReq) {
    string msg = std::to_string(originalReq->getRequestID()) + ": Received " + resp->getStartLine() + " from " + originalReq->getRequestFields("Host") + "\n";
    logger.writeFile(msg);

    string toSend(data.data());
    Status status = resp->getStatus();
    if (status == Status::SUCCESS || status == Status::REDIRECTION) {
        cacheLock.lock();
        if (originalReq->getMethod() == Method::GET) {
            toSend = LRUCache::getInstance()->validateResp(resp, originalReq->uri, data, originalReq);
        }
        cacheLock.unlock();
        sendData(workerFdMap[std::this_thread::get_id()], toSend);
    }
    else if (status == Status::SERVER_ERR || status == Status::INFO || status == Status::CLIENT_ERR) {
        sendData(workerFdMap[std::this_thread::get_id()], toSend);
    }
    else {
        throw HttpResponseSyntaxException("");
    }
    
}

/**
 * methodGET
 * ---------
 * handle http request that has Method GET. First searching for cache
 * history. If no data in cache, redirect the message to the original 
 * server. Then read from the server and send back to the client.
 */
void Proxy::methodGET(std::shared_ptr<HttpRequest> req, std::vector<char> & data) {
    // cout << "*********** Handling GET ************" << endl;
    cacheLock.lock();
    if (LRUCache::getInstance()->validateReq(req)) {
        std::shared_ptr<HttpResponse> resp(LRUCache::getInstance()->get(req->uri));
        sendData(workerFdMap[std::this_thread::get_id()], resp->getMessage());
        return;
    }
    cacheLock.unlock();

    string host = req->getRequestFields("Host");
    if (host == "") {
        throw SocketException("No host available");
    }
    auto servSkt = std::make_shared<SocketObject>(host, "80"); 
    int servfd;
    try {
        servfd = servSkt->setupAndConnect();
        string msg = std::to_string(req->getRequestID()) + ": Requesting " + req->getStartLine() + " from " + req->getRequestFields("Host") + "\n";
        logger.writeFile(msg);
        sendData(servfd, req->getMessage());
        std::vector<char> responseBuff;
        readPlainMessage(servfd, responseBuff);
        responseHandler(parser.getResponse(string(responseBuff.data())), responseBuff, req);
    }
    catch (HttpResponseSyntaxException & e) {
        close(servfd);
        throw;
    }
    catch (SocketException & e) {
        cout << e.what() << endl;
    }
    catch (std::exception & e) {
        cout << "Other: " << e.what() << endl;
    }
}

/**
 * methodCONNECT
 * -------------
 * Handle CONNECT request by selecting from both sockets
 * Open tunnel for the two socket
 */
void Proxy::methodCONNECT(std::shared_ptr<HttpRequest> req, std::vector<char> & data) {
    // cout << "*********** Handling CONNECT ***********" << endl;
    string uri = req->getRequestFields("Host");
    if (uri == "") {
        throw SocketException("No host available");
    }
    string host, port;
    int sep = uri.find(':');
    if (sep == string::npos) {
        host = uri;
        port = "443";
    }
    else {
        host = uri.substr(0, sep);
        port = uri.substr(sep+1);
        if (port != "443") {
            throw SocketException("Wrong port for CONNECT method.");
        }
    }
    auto servSkt = std::make_shared<SocketObject>(host, port); 
    int servFd = servSkt->setupAndConnect();
    int clientFd = workerFdMap[std::this_thread::get_id()];
    sendData(clientFd, "HTTP/1.1 200 OK\r\n\r\n"); // need acknowledging
    
    while (true) {
        // setting up fds set
        fd_set sockset;
        FD_ZERO(&sockset);
        FD_SET(clientFd, &sockset); 
        FD_SET(servFd, &sockset);
        struct timeval tv;
        tv.tv_sec = 25; // timeout
        tv.tv_usec = 0;
        int maxfd = clientFd > servFd ? clientFd : servFd;

        // select will remove unreadable fds
        int res = select(maxfd+1, &sockset, nullptr, nullptr, &tv); 
        if (res == -1) {
            throw SocketException("Select error in tunel setting");
        }
        if (res == 0) {
            close(servFd);
            throw SocketTimeoutException("Timeout in tunnel communication");
        }

        try {
            // if fd is readable, redirect messages in tunnel
            if (FD_ISSET(clientFd, &sockset)) {
                connectTunel(clientFd, servFd);
            } 
            else {
                connectTunel(servFd, clientFd);
            }
        }
        catch (SocketFdException & e) {
            close(servFd);
            throw;
        }
    } 
}

/**
 * connectTunel
 * ------------
 * Contruct a tunnel for client and server to communicate
 *
 * fdIn: receving socket
 * fdOut: sending socket
 */
void Proxy::connectTunel(int fdIn, int fdOut) {
    std::vector<char> buff;
    readPlainMessage(fdIn, buff);
    sendData(fdOut, buff);
}

/**
 * methodPOST
 * ----------
 * Redirect to server and get a repsonse from server then redirect back
 * to the client. No modification needed.
 * 
 * req: request with POST methpd
 * data: data read from the client
 */
void Proxy::methodPOST(std::shared_ptr<HttpRequest> req, std::vector<char> & data) {
    string host = req->getRequestFields("Host");
    if (host == "") {
        throw SocketException("No host available");
    }
    auto servSkt = std::make_shared<SocketObject>(host, "80"); 
    int servfd;
    try {
        servfd = servSkt->setupAndConnect();
        string msg = std::to_string(req->getRequestID()) + ": Requesting " + req->getStartLine() + " from " + req->getRequestFields("Host") + "\n";
        logger.writeFile(msg);
        sendData(servfd, data);
        std::vector<char> responseBuff;
        readPlainMessage(servfd, responseBuff);
        responseHandler(parser.getResponse(string(responseBuff.data())), responseBuff, req);
    }
    catch (HttpResponseSyntaxException & e) {
        close(servfd);
        throw;
    }
    catch (SocketException & e) {
        cout << e.what() << endl;
    }
    catch (std::exception & e) {
        cout << "Other: " << e.what() << endl;
    }
}