#include <iostream>
#include <iomanip>
#include <variant>
#include <vector>
#include <future>
#include <cstring>
#include <curl/curl.h>

#include "order_book.hpp"
#include "price_calculator.hpp"

// CRITICAL: Include exchange implementations BEFORE registry
#include "exchanges/coinbase_client.cpp"
#include "exchanges/gemini_client.cpp"

// NOW include registry
#include "exchange_registry.hpp"

double parseQuantity(int argc, char* argv[]) {
    double quantity = 10.0;
    
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--qty") == 0 && i + 1 < argc) {
            try {
                quantity = std::stod(argv[i + 1]);
                if (quantity <= 0) {
                    std::cerr << "Error: Quantity must be positive\n";
                    return -1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid quantity - " << e.what() << "\n";
                return -1;
            }
            break;
        }
    }
    
    return quantity;
}

std::string formatCurrency(double value) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    std::string result = ss.str();
    
    size_t decimal_pos = result.find('.');
    if (decimal_pos == std::string::npos) {
        decimal_pos = result.length();
    }
    
    for (int i = decimal_pos - 3; i > 0; i -= 3) {
        result.insert(i, ",");
    }
    
    return result;
}

// Helper to create all exchanges (defined here after client implementations)
inline std::vector<ExchangeVariant> createAllExchanges() {
    return std::vector<ExchangeVariant>{
        CoinbaseClient{},
        GeminiClient{}
    };
}

int main(int argc, char* argv[]) {
    double quantity_btc = parseQuantity(argc, argv);
    if (quantity_btc < 0) return 1;
    
    Quantity quantity = static_cast<Quantity>(quantity_btc * QUANTITY_SCALE);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    try {
        // Create all exchanges from registry
        auto exchanges = createAllExchanges();
        
        // Create rate limiters
        auto limiters = createRateLimiters(exchanges.size());
        
        // Fetch all exchanges in parallel using std::visit
        std::vector<std::future<OrderBookSnapshot>> futures;
        
        for (size_t i = 0; i < exchanges.size(); ++i) {
            futures.push_back(std::async(std::launch::async, [&, i]() {
                return std::visit([&](auto& client) {
                    return FetchOrderBook{}(client, *limiters[i]);
                }, exchanges[i]);
            }));
        }
        
        // Aggregate order books
        OrderBook aggregated;
        bool has_data = false;
        int successful_exchanges = 0;
        
        for (size_t i = 0; i < futures.size(); ++i) {
            auto snapshot = futures[i].get();
            
            if (!snapshot.success) {
                std::string name = std::visit(GetExchangeName{}, exchanges[i]);
                std::cerr << "Warning: " << snapshot.error << " (Exchange: " << name << ")\n";
                continue;
            }
            
            aggregated.mergeBids(snapshot.bids);
            aggregated.mergeAsks(snapshot.asks);
            has_data = true;
            successful_exchanges++;
        }
        
        if (!has_data) {
            std::cerr << "Error: Failed to fetch data from any exchange\n";
            curl_global_cleanup();
            return 1;
        }
        
        if (successful_exchanges < static_cast<int>(exchanges.size())) {
            std::cerr << "\nNote: Using data from " << successful_exchanges 
                      << "/" << exchanges.size() << " exchanges\n\n";
        }
        
        // Calculate execution prices
        auto bids = aggregated.getBids();
        auto asks = aggregated.getAsks();
        
        auto buy_result = PriceCalculator::calculateBuyPrice(asks, quantity);
        auto sell_result = PriceCalculator::calculateSellPrice(bids, quantity);
        
        // Output results
        std::cout << std::fixed << std::setprecision(2);
        
        if (buy_result.fully_filled) {
            std::cout << "To buy " << quantity_btc << " BTC: $"
                      << formatCurrency(buy_result.getTotalCostUSD()) << "\n";
        } else {
            std::cout << "To buy " << quantity_btc << " BTC: Insufficient liquidity\n";
        }
        
        if (sell_result.fully_filled) {
            std::cout << "To sell " << quantity_btc << " BTC: $"
                      << formatCurrency(sell_result.getTotalCostUSD()) << "\n";
        } else {
            std::cout << "To sell " << quantity_btc << " BTC: Insufficient liquidity\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        curl_global_cleanup();
        return 1;
    }
    
    curl_global_cleanup();
    return 0;
}
