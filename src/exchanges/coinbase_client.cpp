#include "exchange_interface.hpp"
#include "http_client.hpp"
#include "json.hpp"
#include <chrono>

using json = nlohmann::json;

// Inherit from CRTP base with self-reference
class CoinbaseClient : public ExchangeClientBase<CoinbaseClient> {
public:
    CoinbaseClient() 
        : url_("https://api.exchange.coinbase.com/products/BTC-USD/book?level=2") {}
    
    // Implementation methods (not virtual!)
    OrderBookSnapshot fetchOrderBookImpl() {
        OrderBookSnapshot snapshot;
        snapshot.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        try {
            auto client = HTTPClientPool::instance().acquire();
            std::string response = client->get(url_, 5000);
            HTTPClientPool::instance().release(std::move(client));
            parseResponse(response, snapshot);
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Coinbase fetch error: ") + e.what();
        }
        
        return snapshot;
    }
    
    Exchange getExchangeIdImpl() const { 
        return Exchange::COINBASE; 
    }
    
    std::string getNameImpl() const { 
        return "Coinbase"; 
    }
    
private:
    std::string url_;
    
    void parseResponse(const std::string& json_data, OrderBookSnapshot& snapshot) {
        try {
            auto j = json::parse(json_data);
            
            if (j.contains("bids")) {
                snapshot.bids.reserve(j["bids"].size());
                for (const auto& bid : j["bids"]) {
                    double price_dbl = std::stod(bid[0].get<std::string>());
                    double size_dbl = std::stod(bid[1].get<std::string>());
                    
                    Price price = static_cast<Price>(price_dbl * PRICE_SCALE + 0.5);
                    Quantity size = static_cast<Quantity>(size_dbl * QUANTITY_SCALE + 0.5);
                    
                    if (price > 0 && size > 0) {
                        snapshot.bids.emplace_back(price, size, Exchange::COINBASE);
                    }
                }
            }
            
            if (j.contains("asks")) {
                snapshot.asks.reserve(j["asks"].size());
                for (const auto& ask : j["asks"]) {
                    double price_dbl = std::stod(ask[0].get<std::string>());
                    double size_dbl = std::stod(ask[1].get<std::string>());
                    
                    Price price = static_cast<Price>(price_dbl * PRICE_SCALE + 0.5);
                    Quantity size = static_cast<Quantity>(size_dbl * QUANTITY_SCALE + 0.5);
                    
                    if (price > 0 && size > 0) {
                        snapshot.asks.emplace_back(price, size, Exchange::COINBASE);
                    }
                }
            }
            
            snapshot.success = true;
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Coinbase parse error: ") + e.what();
        }
    }
};
