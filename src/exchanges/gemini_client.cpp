#include "exchange_interface.hpp"
#include "http_client.hpp"
#include "json.hpp"
#include <chrono>
#include <cmath>

using json = nlohmann::json;

class GeminiClient : public IExchangeClient {
public:
    GeminiClient() 
        : url_("https://api.gemini.com/v1/book/BTCUSD") {}
    
    OrderBookSnapshot fetchOrderBook() override {
        OrderBookSnapshot snapshot;
        snapshot.timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        try {
            auto client = HTTPClientPool::instance().acquire();
            std::string response = client->get(url_, 10000);  // CHANGED: 10000ms (10 seconds)
            HTTPClientPool::instance().release(std::move(client));
            
            parseResponse(response, snapshot);
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Gemini fetch error: ") + e.what();
        }
        
        return snapshot;
    }
    
    Exchange getExchangeId() const override { return Exchange::GEMINI; }
    std::string getName() const override { return "Gemini"; }
    
private:
    std::string url_;
    
    void parseResponse(const std::string& json_data, OrderBookSnapshot& snapshot) {
        try {
            auto j = json::parse(json_data);
            
            // Gemini format: [{"price": "50000.00", "amount": "0.5"}, ...]
            if (j.contains("bids")) {
                snapshot.bids.reserve(j["bids"].size());
                for (const auto& bid : j["bids"]) {
                    double price_dbl = std::stod(bid["price"].get<std::string>());
                    double amount_dbl = std::stod(bid["amount"].get<std::string>());
                    
                    Price price = static_cast<Price>(std::round(price_dbl * PRICE_SCALE));
                    Quantity size = static_cast<Quantity>(std::round(amount_dbl * QUANTITY_SCALE));
                    
                    snapshot.bids.emplace_back(price, size, Exchange::GEMINI);
                }
            }
            
            if (j.contains("asks")) {
                snapshot.asks.reserve(j["asks"].size());
                for (const auto& ask : j["asks"]) {
                    double price_dbl = std::stod(ask["price"].get<std::string>());
                    double amount_dbl = std::stod(ask["amount"].get<std::string>());
                    
                    Price price = static_cast<Price>(std::round(price_dbl * PRICE_SCALE));
                    Quantity size = static_cast<Quantity>(std::round(amount_dbl * QUANTITY_SCALE));
                    
                    snapshot.asks.emplace_back(price, size, Exchange::GEMINI);
                }
            }
            
            snapshot.success = true;
        } catch (const std::exception& e) {
            snapshot.success = false;
            snapshot.error = std::string("Gemini parse error: ") + e.what();
        }
    }
};

#include "exchange_factory.hpp"
std::unique_ptr<IExchangeClient> ExchangeFactory::createGemini() {
    return std::make_unique<GeminiClient>();
}
