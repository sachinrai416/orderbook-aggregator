#include "http_client.hpp"
#include <stdexcept>

size_t HTTPClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), total_size);
    return total_size;
}

HTTPClient::HTTPClient() : curl_(curl_easy_init()) {
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // Set common options for low-latency
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_buffer_);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "OrderBookAggregator/2.0");
    curl_easy_setopt(curl_, CURLOPT_TCP_NODELAY, 1L);  // Disable Nagle
    curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);     // Thread-safe
    curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);  // HTTP/2
}

HTTPClient::~HTTPClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

std::string HTTPClient::get(const std::string& url, uint32_t timeout_ms) {
    response_buffer_.clear();
    response_buffer_.reserve(65536);  // Pre-allocate 64KB
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_ms);
    
    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    
    return response_buffer_;
}

HTTPClientPool& HTTPClientPool::instance() {
    static HTTPClientPool pool;
    return pool;
}

std::unique_ptr<HTTPClient> HTTPClientPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pool_.empty()) {
        auto client = std::move(pool_.back());
        pool_.pop_back();
        return client;
    }
    return std::make_unique<HTTPClient>();
}

void HTTPClientPool::release(std::unique_ptr<HTTPClient> client) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.size() < 10) {  // Max pool size
        pool_.push_back(std::move(client));
    }
}
