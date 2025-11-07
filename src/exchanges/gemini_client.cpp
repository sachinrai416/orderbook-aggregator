#include "exchange_interface.hpp"
#include "http_client.hpp"
#include "json.hpp"
#include <chrono>

using json = nlohmann::json;

class GeminiClient : public ExchangeClientBase<GeminiClient> {
public:
    GeminiClient() 
        : url_("https://api.gemini.com/v1/book/BTCUSD?limit_bids=0&limit_asks=0") {}
    
    OrderBookSnapshot fetchOrderBookImpl() {
        OrderBookSnapshot snapshot;
        snapshot.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        try {
            auto client = HTTPClientPool::instance().acquire();
            std::string response = client->get(url_, 15000);  // Gemini timeout
            HTTPClientPool::instance().release(std::move(client));
            parseResponse(response, snapshot);
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Gemini fetch error: ") + e.what();
        }
        
        return snapshot;
    }
    
    Exchange getExchangeIdImpl() const { 
        return Exchange::GEMINI; 
    }
    
    std::string getNameImpl() const { 
        return "Gemini"; 
    }
    
private:
    std::string url_;
    
    void parseResponse(const std::string& json_data, OrderBookSnapshot& snapshot) {
        // Same as before (Gemini JSON parsing)
        try {
            auto j = json::parse(json_data);
            
            if (j.contains("bids")) {
                snapshot.bids.reserve(j["bids"].size());
                for (const auto& bid : j["bids"]) {
                    double price_dbl = std::stod(bid["price"].get<std::string>());
                    double amount_dbl = std::stod(bid["amount"].get<std::string>());
                    
                    Price price = static_cast<Price>(price_dbl * PRICE_SCALE + 0.5);
                    Quantity size = static_cast<Quantity>(amount_dbl * QUANTITY_SCALE + 0.5);
                    
                    if (price > 0 && size > 0) {
                        snapshot.bids.emplace_back(price, size, Exchange::GEMINI);
                    }
                }
            }
            
            if (j.contains("asks")) {
                snapshot.asks.reserve(j["asks"].size());
                for (const auto& ask : j["asks"]) {
                    double price_dbl = std::stod(ask["price"].get<std::string>());
                    double amount_dbl = std::stod(ask["amount"].get<std::string>());
                    
                    Price price = static_cast<Price>(price_dbl * PRICE_SCALE + 0.5);
                    Quantity size = static_cast<Quantity>(amount_dbl * QUANTITY_SCALE + 0.5);
                    
                    if (price > 0 && size > 0) {
                        snapshot.asks.emplace_back(price, size, Exchange::GEMINI);
                    }
                }
            }
            
            snapshot.success = true;
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Gemini parse error: ") + e.what();
        }
    }
};
