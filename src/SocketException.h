#ifndef SOCKET_EXCEPTION_H
#define SOCKET_EXCEPTION_H

#include <exception>
#include <string>

class SocketException : public std::exception {
private:
    std::string msg;
public:
    explicit SocketException(std::string msg) : msg(msg) {}
    virtual const char * what() {
        return this->msg.c_str();
    }
};

class SocketTimeoutException : public std::exception {
private:
    std::string msg;
public:
    explicit SocketTimeoutException(std::string msg) : msg(msg) {}
    virtual const char * what() {
        return this->msg.c_str();
    }
};

class SocketFdException : public std::exception {
private:
    std::string msg;
public:
    explicit SocketFdException(std::string msg) : msg(msg) {}
    virtual const char * what() {
        return this->msg.c_str();
    }
};

#endif