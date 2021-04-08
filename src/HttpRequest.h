#ifndef MY_HTTP_REQUEST
#define MY_HTTP_REQUEST
#include "HttpMessage.h"

enum Method {
    GET,
    POST,
    CONNECT
};

class HttpRequest: public HttpMessage {
    std::unordered_map<string, string> requestFields;
    long requestID;
    static long idCnt;
public:
    Method method;
    string uri;
    string version;     

    typedef HttpMessage super;
    HttpRequest(): super(), requestID(idCnt) {
        ++idCnt;
    }

    Method getMethod() const {
        return this->method;
    }

    long getRequestID() const {
        return requestID;
    }

    string getRequestFields(const string & key) const;
    void setRequestField(const string & key, const string & val);
    void updateMessage(const string & key, const string & val);
};

#endif