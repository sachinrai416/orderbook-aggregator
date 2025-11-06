#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <future>
#include <locale>
#include <cstring>
#include <curl/curl.h>

#include "order_book.hpp"
#include "exchange_factory.hpp"
#include "rate_limiter.hpp"
#include "price_calculator.hpp"

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
    
    // Add commas for thousands
    size_t decimal_pos = result.find('.');
    if (decimal_pos == std::string::npos) {
        decimal_pos = result.length();
    }
    
    for (int i = decimal_pos - 3; i > 0; i -= 3) {
        result.insert(i, ",");
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    double quantity = parseQuantity(argc, argv);
    if (quantity < 0) return 1;
    
    Quantity quantity_fixed = static_cast<Quantity>(quantity * QUANTITY_SCALE);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    try {
        auto exchanges = ExchangeFactory::createFromConfig("");
        
        std::vector<std::unique_ptr<RateLimiter>> limiters;
        for (size_t i = 0; i < exchanges.size(); ++i) {
            limiters.push_back(
                std::make_unique<RateLimiter>(std::chrono::milliseconds(2000)));
        }
        
        // Fetch order books in parallel
        std::vector<std::future<OrderBookSnapshot>> futures;
        for (size_t i = 0; i < exchanges.size(); ++i) {
            futures.push_back(limiters[i]->execute([&, i]() {
                return exchanges[i]->fetchOrderBook();
            }));
        }
        
        // Aggregate order books
        OrderBook aggregated;
        bool has_data = false;
        
        for (size_t i = 0; i < futures.size(); ++i) {
            auto snapshot = futures[i].get();
            
            if (!snapshot.success) {
                std::cerr << "Warning: " << snapshot.error << "\n";
                continue;
            }
            
            #ifdef DEBUG_ORDERBOOK
            std::cerr << "\n" << exchanges[i]->getName() << " Order Book:\n";
            std::cerr << "  Bids: " << snapshot.bids.size() << " levels\n";
            std::cerr << "  Asks: " << snapshot.asks.size() << " levels\n";
            if (!snapshot.bids.empty()) {
                std::cerr << "  Best Bid: $" << std::fixed << std::setprecision(2)
                         << (snapshot.bids[0].price / static_cast<double>(PRICE_SCALE)) << "\n";
            }
            if (!snapshot.asks.empty()) {
                std::cerr << "  Best Ask: $" << std::fixed << std::setprecision(2)
                         << (snapshot.asks[0].price / static_cast<double>(PRICE_SCALE)) << "\n";
            }
            #endif
            
            aggregated.mergeBids(snapshot.bids);
            aggregated.mergeAsks(snapshot.asks);
            has_data = true;
        }
        
        if (!has_data) {
            std::cerr << "Error: Failed to fetch data from any exchange\n";
            curl_global_cleanup();
            return 1;
        }
        
        auto bids = aggregated.getBids();
        auto asks = aggregated.getAsks();
        
        #ifdef DEBUG_ORDERBOOK
        std::cerr << "\nAggregated Order Book:\n";
        std::cerr << "  Total Bids: " << bids.size() << " levels\n";
        std::cerr << "  Total Asks: " << asks.size() << " levels\n";
        if (!bids.empty()) {
            std::cerr << "  Best Aggregated Bid: $" << std::fixed << std::setprecision(2)
                     << (bids[0].price / static_cast<double>(PRICE_SCALE)) << "\n";
        }
        if (!asks.empty()) {
            std::cerr << "  Best Aggregated Ask: $" << std::fixed << std::setprecision(2)
                     << (asks[0].price / static_cast<double>(PRICE_SCALE)) << "\n";
        }
        #endif
        
        auto buy_result = PriceCalculator::calculateBuyPrice(asks, quantity_fixed);
        auto sell_result = PriceCalculator::calculateSellPrice(bids, quantity_fixed);
        
        // Output results
        std::cout << std::fixed << std::setprecision(2);
        
        if (buy_result.fully_filled) {
            std::cout << "To buy " << quantity << " BTC: $"
                      << formatCurrency(buy_result.getTotalCostUSD()) << "\n";
        } else {
            std::cout << "To buy " << quantity << " BTC: Insufficient liquidity\n";
        }
        
        if (sell_result.fully_filled) {
            std::cout << "To sell " << quantity << " BTC: $"
                      << formatCurrency(sell_result.getTotalCostUSD()) << "\n";
        } else {
            std::cout << "To sell " << quantity << " BTC: Insufficient liquidity\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        curl_global_cleanup();
        return 1;
    }
    
    curl_global_cleanup();
    return 0;
}
