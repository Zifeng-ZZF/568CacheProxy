#ifndef MY_HTTP_MSG
#define MY_HTTP_MSG
#include <string>
#include <iostream>
#include <unordered_map>
using std::string;
using std::cout;
using std::endl;

class HttpMessage {
protected:
    string start_line;
    string content;
    string wholeMsg;
    std::unordered_map<string, string> genFields;
    std::unordered_map<string, string> entityFields;
public:
    HttpMessage() {
        initialize();
    }

    virtual void updateMessage(const string & key, const string & val) = 0;

    void setWholeMessage(string msg) {
        this->wholeMsg = msg;
    }

    string getMessage() const {
        return this->wholeMsg;
    }

    void setStartLine(string & line) {
        this->start_line = line;
    }

    string getStartLine() const {
        return this->start_line;
    }

    void setContent(string & content) {
        this->content = content;
    }

    void initialize() {
        genFields["Cache-Control"] = "";
        genFields["Connection"] = "";
        genFields["Date"] = "";
        genFields["Pragma"] = "";
        genFields["Trailer"] = "";
        genFields["Transfer-Encoding"] = "";
        genFields["Upgrade"] = "";
        genFields["Via"] = "";
        genFields["Warning"] = "";

        entityFields["Allow"] = "";
        entityFields["Content-Encoding"] = "";
        entityFields["Content-Language"] = "";
        entityFields["Content-Length"] = "";
        entityFields["Content-Location"] = "";
        entityFields["Content-MD5"] = "";
        entityFields["Content-Range"] = "";
        entityFields["Content-Type"] = "";
        entityFields["Expires"] = "";
        entityFields["Last-Modified"] = "";
    }
};

#endif