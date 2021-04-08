#include "LRUCache.h"

LRUCache::LRUCache(): capacity(CAP), lru(), hashing(), preStoredResp(), logger(){
    initialize();
    try {
        logger.openFileStream("/var/log/erss/proxy.log");
    }
    catch (SocketException & e) {
        cout << "[LOGGER] open failed" << endl;
    }
}


LRUCache::~LRUCache() {
    for (auto item : lru) {
        delete item.second;
    }
}


LRUCache * LRUCache::getInstance() {
    static LRUCache mInstance;
    return &mInstance;
}


void LRUCache::initialize() {
    preStoredResp[400] = "HTTP/1.1 400 NotFound\r\n\r\n";
    preStoredResp[504] = "HTTP/1.1 504 GatewayTimeout\r\n\r\n";
}


HttpResponse * LRUCache::get(const string & key) {
    if (hashing.find(key) == hashing.end()) {
        return NULL;
    }
    lru.splice(lru.begin(), lru, hashing[key]);
    return hashing[key]->second;
}


void LRUCache::add(const string & key, HttpResponse * val) {
    if (get(key) != NULL) {
        HttpResponse * temp = hashing[key]->second;
        hashing[key]->second = val;
        delete temp;
    }
    // if full, delete the least recently used
    if (lru.size() == capacity) {
        string toDel = lru.back().first;
        lru.pop_back();
        string msg = "(no-id): NOTE evicted " + toDel + " from cache\n";
        hashing.erase(toDel);
    }
    lru.emplace_front(key, val);
    hashing[key] = lru.begin();
}


bool LRUCache::validateReq(std::shared_ptr<HttpRequest> req) {
    std::shared_ptr<HttpResponse> resp(get(req->uri));
    if (resp.get() == NULL) {
        string msg = std::to_string(req->getRequestID()) + ": not in cache\n";
        logger.writeFile(msg);
        return false;
    }
    string msg = std::to_string(req->getRequestID()) + ": in cache, ";
    string reqCacheCtrl = req->getRequestFields("Cache-Control");
    string respCacheCtrl = resp->getResponseFields("Cache-Control");
    if (reqCacheCtrl == "") {
        return checkResponseCacheControl(req, resp);
    }
    else if (reqCacheCtrl == "no-store" || reqCacheCtrl == "no-cahce") {
        msg.append("requires validation\n");
        logger.writeFile(msg);
        return false;
    }
    else if (reqCacheCtrl.find("max-age") != string::npos) {
        int age = stoi(resp->getResponseFields("Age"));
        int reqMaxAge = stoi(reqCacheCtrl.substr(reqCacheCtrl.find("=")+1));
        if (reqMaxAge <= age) {
            msg.append("valid\n");
            logger.writeFile(msg);
            return true;
        }
        else {
            msg.append("requires validation\n");
            logger.writeFile(msg);
            return false;
        }
    }
    else { //response cache-control?
        return checkResponseCacheControl(req, resp);
    }
}

bool LRUCache::checkResponseCacheControl(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
    string respCacheCtrl = resp->getResponseFields("Cache-Control");
    if (respCacheCtrl != "" && (respCacheCtrl.find("max-age") != string::npos || respCacheCtrl.find("s-maxage") != string::npos)) {
        return cacheControlValidateReq(req, resp);
    }
    else {
        string expire = resp->getResponseFields("Expires");
        if (expire != "") {
            return expireValidateReq(expire, req);
        }
        else {
            return heuristicValidateReq(req, resp);
        } 
    }
}

string LRUCache::validateResp(std::shared_ptr<HttpResponse> resp, string uri, std::vector<char> & data, std::shared_ptr<HttpRequest> originalReq) {
    string cacheControl = resp->getResponseFields("Cache-Control");
    string expire = resp->getResponseFields("Expires");
    if (this->get(uri) == NULL || cacheControl == "" || expire != "") {
        if (expire != "") {
            struct tm expireTm;
            strptime(expire.c_str(), "%a, %d %b %Y %H:%M:%S", &expireTm);
            time_t expTt = mktime(&expireTm);
            string msg = std::to_string(originalReq->getRequestID()) + ": cached, expires at " + ctime(&expTt) + "\n";
            logger.writeFile(msg);
        }
        else {
            string msg = std::to_string(originalReq->getRequestID()) + ": cached, but requires revalidation\n";
            logger.writeFile(msg);
        }
        HttpResponse * caching = new HttpResponse();
        *caching = *resp;
        this->add(uri, caching);
        return get(uri)->getMessage();
    } 

    if (cacheControl.find("no-store") != string::npos || cacheControl.find("no-cache") != string::npos) {
        string msg = std::to_string(originalReq->getRequestID()) + ": not cacheable because No-Cache/No-Store\n";
        logger.writeFile(msg);
        return string(data.data());
    }

    Status status = resp->getStatus();
    if (status == Status::SUCCESS) {
        HttpResponse * caching = new HttpResponse();
        *caching = *resp;
        this->add(uri, caching);
        return get(uri)->getMessage();
    }
    else if (status == Status::REDIRECTION) {
        return get(uri)->getMessage();
    }
    else if (status == Status::SERVER_ERR) {
        return string(data.data());
    }
    else {
        throw HttpResponseSyntaxException("");
    }
}

bool LRUCache::cacheControlValidateReq(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> cachedResp) {
    string respCacheCtrl = cachedResp->getResponseFields("Cache-Control");
    string reqCacheCtrl = req->getRequestFields("Cache-Control");
    int age = stoi(cachedResp->getResponseFields("Age"));
    int maxAge = stoi(respCacheCtrl.substr(respCacheCtrl.find("=")+1));

    string msg = std::to_string(req->getRequestID());
    if (age <= maxAge) {
        if (reqCacheCtrl.find("min-fresh") != string::npos) {
            int minFresh = stoi(reqCacheCtrl.substr(reqCacheCtrl.find("=")+1));
            if(maxAge - age >= minFresh) {
                msg.append(":in cache, valid\n");
                logger.writeFile(msg);
                return true;
            }
            else {
                return heuristicValidateReq(req, cachedResp);
            }
        }
        msg.append(":in cache, valid\n");
        logger.writeFile(msg);
        return true;
    }
    else {
        if (reqCacheCtrl.find("max-stale") != string::npos) {
            if (respCacheCtrl.find("proxy-revalidate") != string::npos || respCacheCtrl.find("must-revalidate") != string::npos) {
                return heuristicValidateReq(req, cachedResp);
            }
            int maxStale = stoi(reqCacheCtrl.substr(reqCacheCtrl.find("=")+1));
            if(age - maxAge <= maxStale) {
                msg.append(":in cache, valid\n");
                logger.writeFile(msg);
                return true;
            }
            return heuristicValidateReq(req, cachedResp);
        }
        return heuristicValidateReq(req, cachedResp);
    }
}

bool LRUCache::heuristicValidateReq(std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> cachedResp) {
    // first If-None-Match
    string msg = std::to_string(req->getRequestID());
    if (cachedResp->getResponseFields("ETag") != "") {
        string header = "If-None-Match";
        string etagVal = cachedResp->getResponseFields("ETag");
        req->updateMessage(header, etagVal);
        string log = msg + ": NOTE ETags: " + etagVal;
        logger.writeFile(log);
        msg.append(":in cache, requires validation\n");
        logger.writeFile(msg);
        return false; // need go to server
    }
    // If-Unmodified-Since, If-Modified-Since
    if (cachedResp->getResponseFields("Last-Modified") != "") {
        string header = "If-Modified-Since";
        string value = cachedResp->getResponseFields("Last-Modified");
        req->updateMessage(header, value);
        msg.append(":in cache, requires validation\n");
        logger.writeFile(msg);
        return false;
    }
    msg.append(":in cache, valid\n");
    logger.writeFile(msg);
    return true;
}

bool LRUCache::expireValidateReq(string expires, std::shared_ptr<HttpRequest> req) {
    struct tm expireTm;
    strptime(expires.c_str(), "%a, %d %b %Y %H:%M:%S", &expireTm);
    time_t currTt = time(0);
    time_t expTt = mktime(&expireTm);
    long diff = currTt - expTt;
    string msg = std::to_string(req->getRequestID()) + ": in cache, ";
    if (diff < 3600) {
        msg.append("valid\n");
        logger.writeFile(msg);
        return true;
    }
    else {
        msg.append("but expired at " + string(ctime(&expTt)) + "\n");
        logger.writeFile(msg);
        return false;
    }
}


