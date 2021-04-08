#include "HttpParser.h"
#include <iostream>
using std::cout;
using std::endl;

HttpParser::HttpParser():logger() {
    try {
        logger.openFileStream("/var/log/erss/proxy.log");
    }
    catch (SocketException & e) {
        cout << "[LOGGER] open failed" << endl;
    }
}

void HttpParser::parseHeaderLine(string line, HttpMessage * mesg, bool isReq) {
    size_t commaPos = line.find_first_of(':');      
    string header = line.substr(0, commaPos);
    string value = line.substr(commaPos+2);
    if (commaPos == string::npos || header.empty()) {
        throw HttpSyntaxException("");
    } 
    if (isReq) {
        ((HttpRequest *)mesg)->setRequestField(header, value);
    }
    else {
        ((HttpResponse *)mesg)->setResponseField(header, value);
    }
}

void HttpParser::parseHeader(string msgStr, HttpMessage * mesg, bool isReq) {
    mesg->setWholeMessage(msgStr);
    size_t lineCnt = 0, crlfPos = 0, total = 0;
    size_t headerEnd = msgStr.find("\r\n\r\n");
    while (total < headerEnd+2) {
        crlfPos = msgStr.find_first_of('\r');
        if (crlfPos == string::npos) {
            throw HttpSyntaxException("");
        }
        string line = msgStr.substr(0, crlfPos);
        if (lineCnt == 0) {
            if (isReq) {
                parseRequestLine(line, (HttpRequest *)mesg);
            }
            else {
                parseResponseLine(line, (HttpResponse *)mesg);
            }
        }
        else {
            parseHeaderLine(line, mesg, isReq);
        }
        msgStr = msgStr.substr(crlfPos + 2);
        ++lineCnt;
        total += crlfPos + 2;
    }
    if (msgStr.length() > 0) { // msgStr is at the empty line now
        mesg->setContent(msgStr);
    }
}

void HttpParser::parseRequestLine(string line, HttpRequest * httpReq) {
    size_t spos = line.find_first_of(' ');
    if (spos == string::npos) {
        throw HttpRequestSyntaxException("");
    }
    string method = line.substr(0, spos);
    httpReq->setStartLine(line);
    if (method == "GET") {
        httpReq->method = Method::GET;
    }
    else if (method == "POST") {
        httpReq->method = Method::POST;
    }
    else if (method == "CONNECT") {
        httpReq->method = Method::CONNECT;
    }
    else {
        throw HttpRequestSyntaxException("");
    }
    line = line.substr(spos+1);
    spos = line.find_first_of(' ');
    if (spos == string::npos) {
        throw HttpRequestSyntaxException("");
    }

    httpReq->uri = line.substr(0, spos);
    if (httpReq->uri.empty()) {
        throw HttpRequestSyntaxException("");
    }
    
    httpReq->version = line.substr(spos+1);
    if (httpReq->version != "HTTP/1.1" && httpReq->version != "HTTP/1.0") {
        throw HttpRequestSyntaxException("");
    }
}

void HttpParser::parseResponseLine(string line, HttpResponse * resp) {
    size_t spos = line.find_first_of(' ');
    if (spos == string::npos) {
        throw HttpResponseSyntaxException("");
    }
    resp->setStartLine(line);

    resp->version = line.substr(0, spos);
    if (resp->version != "HTTP/1.1" && resp->version != "HTTP/1.0") {
        throw HttpRequestSyntaxException("");
    }

    line = line.substr(spos+1);
    spos = line.find_first_of(' ');
    if (spos == string::npos) {
        throw HttpResponseSyntaxException("");
    }

    string statusField = line.substr(0, spos);
    if (statusField.length() != 3 || statusField[0] < '1' || statusField[0] > '5' || 
        statusField[1] != '0' || statusField[2] < '0' || statusField[2] > '9') {
        throw HttpResponseSyntaxException("");
    }
    resp->setStatusField(statusField);
    
    line = line.substr(spos+1);
    spos = line.find_first_of('\r');
    if (spos == string::npos) {
        throw HttpResponseSyntaxException("");
    }
    resp->reasonPhrase = line.substr(0, spos);
}

std::shared_ptr<HttpRequest> HttpParser::getRequest(string requestStr) {
    auto req = std::make_shared<HttpRequest>();
    try {
        parseHeader(requestStr, req.get(), true);
    }
    catch (HttpSyntaxException & e) {
        throw HttpRequestSyntaxException("");
    }
    logRequest(req);
    return req;
}

std::shared_ptr<HttpResponse> HttpParser::getResponse(string requestStr){
    auto resp = std::make_shared<HttpResponse>();
    try {
        parseHeader(requestStr, resp.get(), false);
    }
    catch (HttpSyntaxException & e) {
        throw HttpResponseSyntaxException("");
    }
    return resp;
}

void HttpParser::logRequest(std::shared_ptr<HttpRequest> req) {
    string logMessage = std::to_string(req->getRequestID()) + ": ";
    logMessage.append("\"").append(req->getStartLine()).append(" from ");
    logMessage.append(req->getRequestFields("Host")).append("@");

    time_t curr = time(NULL);
    logMessage.append(ctime(&curr)).append("\n");

    logger.writeFile(logMessage);
}