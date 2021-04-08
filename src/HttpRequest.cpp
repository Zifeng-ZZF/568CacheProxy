#include "HttpRequest.h"

long HttpRequest::idCnt = 0;

string HttpRequest::getRequestFields(const string & key) const {
    if (requestFields.find(key) == requestFields.end()) {
        return "";
    }
    return requestFields.at(key);
}

void HttpRequest::setRequestField(const string & key, const string & val) {
    requestFields[key] = val;
}

void HttpRequest::updateMessage(const string & key, const string & val) {
    setRequestField(key, val);
    string crlf = "\r\n";
    string message = start_line + crlf;
    for (auto & item : requestFields) {
        message += item.first + ": " + item.second + crlf;
    }
    message += content;
    cout << "[DEBUG] reconstruct request:\n" << message << endl;
    this->wholeMsg = message;
}