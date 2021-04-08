#ifndef _MY_PROXY
#define _MY_PROXY
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <exception>
#include <unordered_map>
#include "SocketObject.h"
#include "SocketException.h"
#include "HttpParser.h"
#include "LRUCache.h"

using std::cout;
using std::endl;
using std::cerr;

const size_t INI_SIZE = 1024;
const size_t MAX_REC = 65536;

/**
 * Proxy entity class
 * 
 * @author Zifeng Zhang
 */
class Proxy {
    Logger logger;
    std::mutex cacheLock;
    std::vector<std::thread> workers;
    std::unordered_map<std::thread::id, int> workerFdMap;
    HttpParser parser;
    void listen(std::shared_ptr<SocketObject> socketObj);
    void readInData(int fd, std::vector<char> & buff);
    void readPlainMessage(int fd, std::vector<char> & buff);
    void sendData(int fd, std::vector<char> & data);
    void sendData(int fd, const string & data);
    void handleRequest(int fd);
    void requestHandler(std::shared_ptr<HttpRequest> req, std::vector<char> & data);
    void responseHandler(std::shared_ptr<HttpResponse> resp, std::vector<char> & data, std::shared_ptr<HttpRequest> originalReq);
    void methodCONNECT(std::shared_ptr<HttpRequest> req, std::vector<char> & data);
    void methodGET(std::shared_ptr<HttpRequest> req, std::vector<char> & data);
    void methodPOST(std::shared_ptr<HttpRequest> req, std::vector<char> & data);
    void redirectResponse(std::shared_ptr<HttpResponse> req);
    void connectTunel(int fdIn, int fdOut);
public:
    Proxy();
    ~Proxy();
    void run();
};

void readInDataOut(int fd);
#endif