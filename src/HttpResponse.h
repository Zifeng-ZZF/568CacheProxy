#ifndef MY_HTTP_RESPONSE
#define MY_HTTP_RESPONSE
#include "HttpMessage.h"

enum Status {
    INFO,
    SUCCESS,
    REDIRECTION,
    CLIENT_ERR,
    SERVER_ERR,
};

class HttpResponse: public HttpMessage {
    std::unordered_map<string, string> responseFields;
    std::unordered_map<char, Status> flagStatusMap;
    string statusCode;
    Status statusGeneral;
public:
    string version;
    string reasonPhrase;

    typedef HttpMessage super;
    HttpResponse(): super() {
        initialize();
    }

    string getResponseFields(const string & key) const {
        if (responseFields.find(key) == responseFields.end()) {
            return "";
        }
        return responseFields.at(key);
    }

    void setResponseField(const string & key, const string & val){
        responseFields[key] = val;
    }

    void setStatusField(string status) {
        this->statusCode = status;
        this->statusGeneral = flagStatusMap[status[0]];
    }

    Status getStatus() const {
        return this->statusGeneral;
    }

    string getStatusCode() const {
        return this->statusCode;
    }

    void updateMessage(const string & key, const string & val) {
        setResponseField(key, val);
        string crlf = "\r\n";
        string message = start_line + crlf;
        for (auto & item : responseFields) {
            message += item.first + ": " + item.second + crlf;
        }
        message += content;
        cout << "[DEBUG] reconstruct response:\n" << message << endl;
        this->wholeMsg = message;
    }

    void initialize() {
        // responseFields["Accept-Ranges"] = "";
        // responseFields["Age"] = "";
        // responseFields["ETag"] = "";
        // responseFields["Location"] = "";
        // responseFields["Proxy-Authenticate"] = "";;
        // responseFields["Retry-After"] = "";
        // responseFields["Server"] = "";
        // responseFields["Vary"] = "";
        // responseFields["WWW-Authenticate"] = "";
        flagStatusMap['1'] = Status::INFO;
        flagStatusMap['2'] = Status::SUCCESS;
        flagStatusMap['3'] = Status::REDIRECTION;
        flagStatusMap['4'] = Status::CLIENT_ERR;
        flagStatusMap['5'] = Status::SERVER_ERR;
    }
};

#endif