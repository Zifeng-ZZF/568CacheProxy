#ifndef _HTTP_PARSER_
#define _HTTP_PARSER_

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpException.h"
#include "Logger.h"

#include <memory>
#include <sstream>

class HttpParser {
    Logger logger;
    void parseResHeader(string msgStr, std::shared_ptr<HttpResponse> httpRes);
    void parseHeader(string msgStr, HttpMessage * mesg, bool isReq);
    void parseRequestLine(string line, HttpRequest * httpReq);
    void parseResponseLine(string line, HttpResponse * mesg);
    void parseStatusLine(string line, std::shared_ptr<HttpRequest> httpReq);
    void parseHeaderLine(string line, HttpMessage * mesg, bool isReq);
    void logRequest(std::shared_ptr<HttpRequest> httpReq);
public:
    HttpParser();
    std::shared_ptr<HttpRequest> getRequest(string requestStr);
    std::shared_ptr<HttpResponse> getResponse(string requestStr);
};

#endif