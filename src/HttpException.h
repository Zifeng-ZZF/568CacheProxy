#ifndef HTTP_EXCEPT_H
#define HTTP_EXCEPT_H

#include <exception>
#include <string>

class HttpException : public std::exception {
private:
    std::string msg;
    int statusCode;
public:
    explicit HttpException(std::string msg, int status) : msg(msg), statusCode(status) {}
    virtual const char * what() {
        return this->msg.c_str();
    }
};

class HttpSyntaxException : public std::exception {
private:
    std::string msg;
    std::string response;
public:
    explicit HttpSyntaxException(std::string msg) : msg(msg){
        response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    }
    virtual const char * what() {
        return this->msg.c_str();
    }
    std::string getRespnse() {
        return this->response;
    }
};

class HttpRequestSyntaxException : public std::exception {
private:
    std::string msg;
    std::string response;
public:
    explicit HttpRequestSyntaxException(std::string msg) : msg(msg){
        response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    }
    virtual const char * what() {
        return this->msg.c_str();
    }
    std::string getRespnse() {
        return this->response;
    }
};

class HttpResponseSyntaxException : public std::exception {
private:
    std::string msg;
    std::string response;
public:
    explicit HttpResponseSyntaxException(std::string msg) : msg(msg){
        response = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    }
    virtual const char * what() {
        return this->msg.c_str();
    }
    std::string getRespnse() {
        return this->response;
    }
};

#endif