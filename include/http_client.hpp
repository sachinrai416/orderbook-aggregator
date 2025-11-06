#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <curl/curl.h>
#include <iostream>
class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();
    
    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;
    
    std::string get(const std::string& url, uint32_t timeout_ms = 5000);
    
private:
    CURL* curl_;
    std::string response_buffer_;
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

class HTTPClientPool {
public:
    static HTTPClientPool& instance();
    std::unique_ptr<HTTPClient> acquire();
    void release(std::unique_ptr<HTTPClient> client);
    
private:
    HTTPClientPool() = default;
    std::vector<std::unique_ptr<HTTPClient>> pool_;
    std::mutex mutex_;
};
