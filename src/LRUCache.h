#ifndef MY_LRU
#define MY_LRU
#include <unordered_map>
#include <string>
#include <mutex>
#include <list>
#include <memory>
#include <vector>
#include <time.h>
#include "HttpResponse.h"
#include "HttpException.h"
#include "HttpRequest.h"
#include "Logger.h"

const int CAP = 100;

using std::list;
using std::pair;
using std::unordered_map;
using std::string;
typedef list<pair<string, HttpResponse *>>::iterator listIt;

/**
 * LRU Cache singleton class
 * maps URI to its response
 * 
 * @author: Zifeng Zhang
 */
class LRUCache {
private:
    Logger logger;
    int capacity;
    list<pair<string, HttpResponse *>> lru;
    unordered_map<string, listIt> hashing;
    unordered_map<int, string> preStoredResp;
    LRUCache();
    ~LRUCache();
public:
    static LRUCache * getInstance();
    HttpResponse * get(const string & key);
    void add(const string & key, HttpResponse * val);
    LRUCache(const LRUCache &) = delete;
    void operator=(const LRUCache &) = delete;

    void initialize();

    bool validateReq(std::shared_ptr<HttpRequest> req);
    string validateResp(std::shared_ptr<HttpResponse> resp, string uri, std::vector<char> & data, std::shared_ptr<HttpRequest> originalReq);

    bool cacheControlValidateReq(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> cachedResp);
    bool cacheControlValidateResp(std::shared_ptr<HttpResponse> resp);

    bool heuristicValidateReq(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> cachedResp);
    bool heuristicValidateResp(std::shared_ptr<HttpResponse> resp);

    bool checkResponseCacheControl(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp);

    bool expireValidateReq(string, std::shared_ptr<HttpRequest> req);
};

#endif