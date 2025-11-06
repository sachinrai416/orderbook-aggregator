// #include "price_calculator.hpp"
// #include <algorithm>

// ExecutionResult PriceCalculator::calculateBuyPrice(
//     const std::vector<PriceLevel>& asks, 
//     Quantity quantity) {
    
//     ExecutionResult result{0, 0, false, ""};
    
//     if (asks.empty()) {
//         result.error = "No asks available";
//         return result;
//     }
    
//     // Sort ascending by price (cheapest first)
//     std::vector<PriceLevel> sorted_asks = asks;
//     std::sort(sorted_asks.begin(), sorted_asks.end(), 
//         [](const PriceLevel& a, const PriceLevel& b) {
//             return a.price < b.price;
//         });
    
//     Quantity remaining = quantity;
    
//     for (const auto& ask : sorted_asks) {
//         if (remaining <= 0) break;
        
//         Quantity fill_amount = std::min(remaining, ask.size);
        
//         // Fixed-point multiplication: (price_cents * satoshis) / QUANTITY_SCALE
//         result.total_cost += (ask.price * fill_amount) / QUANTITY_SCALE;
//         result.quantity_filled += fill_amount;
//         remaining -= fill_amount;
//     }
    
//     result.fully_filled = (remaining == 0);
    
//     if (!result.fully_filled) {
//         result.error = "Insufficient liquidity";
//     }
    
//     return result;
// }

// ExecutionResult PriceCalculator::calculateSellPrice(
//     const std::vector<PriceLevel>& bids, 
//     Quantity quantity) {
    
//     ExecutionResult result{0, 0, false, ""};
    
//     if (bids.empty()) {
//         result.error = "No bids available";
//         return result;
//     }
    
//     // Sort descending by price (highest first)
//     std::vector<PriceLevel> sorted_bids = bids;
//     std::sort(sorted_bids.begin(), sorted_bids.end(), 
//         [](const PriceLevel& a, const PriceLevel& b) {
//             return a.price > b.price;
//         });
    
//     Quantity remaining = quantity;
    
//     for (const auto& bid : sorted_bids) {
//         if (remaining <= 0) break;
        
//         Quantity fill_amount = std::min(remaining, bid.size);
        
//         result.total_cost += (bid.price * fill_amount) / QUANTITY_SCALE;
//         result.quantity_filled += fill_amount;
//         remaining -= fill_amount;
//     }
    
//     result.fully_filled = (remaining == 0);
    
//     if (!result.fully_filled) {
//         result.error = "Insufficient liquidity";
//     }
    
//     return result;
// }


#include "price_calculator.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>

// Compile with -DDEBUG_ORDERBOOK to enable
#ifdef DEBUG_ORDERBOOK
#define DEBUG_LOG(x) std::cerr << x << std::endl
#else
#define DEBUG_LOG(x)
#endif

ExecutionResult PriceCalculator::calculateBuyPrice(
    const std::vector<PriceLevel>& asks, 
    Quantity quantity) {
    
    ExecutionResult result{0, 0, false, ""};
    
    if (asks.empty()) {
        result.error = "No asks available";
        return result;
    }
    
    // Sort ascending (though multimap already sorted, this handles edge cases)
    std::vector<PriceLevel> sorted_asks = asks;
    std::sort(sorted_asks.begin(), sorted_asks.end(), 
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price < b.price;
        });
    
    Quantity remaining = quantity;
    
    DEBUG_LOG("\n=== BUY EXECUTION ===");
    DEBUG_LOG("Target: " << (quantity / static_cast<double>(QUANTITY_SCALE)) << " BTC");
    DEBUG_LOG("Total ask levels: " << sorted_asks.size());
    
    int level = 0;
    for (const auto& ask : sorted_asks) {
        if (remaining <= 0) break;
        
        Quantity fill_amount = std::min(remaining, ask.size);
        
        // Fixed-point multiplication: (cents * satoshis) / satoshis = cents
        // Example: (10336750 * 100000000) / 100000000 = 10336750 cents = $103367.50
        int64_t fill_cost_cents = (static_cast<int64_t>(ask.price) * 
                                   static_cast<int64_t>(fill_amount)) / QUANTITY_SCALE;
        
        result.total_cost += fill_cost_cents;
        result.quantity_filled += fill_amount;
        remaining -= fill_amount;
        
        DEBUG_LOG("Level " << ++level << ": " 
                 << (fill_amount / static_cast<double>(QUANTITY_SCALE)) << " BTC @ $"
                 << (ask.price / static_cast<double>(PRICE_SCALE)) 
                 << " = $" << (fill_cost_cents / static_cast<double>(PRICE_SCALE))
                 << " (" << exchangeName(ask.exchange) << ")");
    }
    
    result.fully_filled = (remaining == 0);
    
    DEBUG_LOG("Total cost: $" << result.getTotalCostUSD());
    DEBUG_LOG("Remaining: " << (remaining / static_cast<double>(QUANTITY_SCALE)) << " BTC\n");
    
    if (!result.fully_filled) {
        result.error = "Insufficient liquidity";
    }
    
    return result;
}

ExecutionResult PriceCalculator::calculateSellPrice(
    const std::vector<PriceLevel>& bids, 
    Quantity quantity) {
    
    ExecutionResult result{0, 0, false, ""};
    
    if (bids.empty()) {
        result.error = "No bids available";
        return result;
    }
    
    // Sort descending (though multimap already sorted)
    std::vector<PriceLevel> sorted_bids = bids;
    std::sort(sorted_bids.begin(), sorted_bids.end(), 
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price > b.price;
        });
    
    Quantity remaining = quantity;
    
    DEBUG_LOG("\n=== SELL EXECUTION ===");
    DEBUG_LOG("Target: " << (quantity / static_cast<double>(QUANTITY_SCALE)) << " BTC");
    DEBUG_LOG("Total bid levels: " << sorted_bids.size());
    
    int level = 0;
    for (const auto& bid : sorted_bids) {
        if (remaining <= 0) break;
        
        Quantity fill_amount = std::min(remaining, bid.size);
        
        // Fixed-point multiplication
        int64_t fill_revenue_cents = (static_cast<int64_t>(bid.price) * 
                                      static_cast<int64_t>(fill_amount)) / QUANTITY_SCALE;
        
        result.total_cost += fill_revenue_cents;
        result.quantity_filled += fill_amount;
        remaining -= fill_amount;
        
        DEBUG_LOG("Level " << ++level << ": " 
                 << (fill_amount / static_cast<double>(QUANTITY_SCALE)) << " BTC @ $"
                 << (bid.price / static_cast<double>(PRICE_SCALE)) 
                 << " = $" << (fill_revenue_cents / static_cast<double>(PRICE_SCALE))
                 << " (" << exchangeName(bid.exchange) << ")");
    }
    
    result.fully_filled = (remaining == 0);
    
    DEBUG_LOG("Total revenue: $" << result.getTotalCostUSD());
    DEBUG_LOG("Remaining: " << (remaining / static_cast<double>(QUANTITY_SCALE)) << " BTC\n");
    
    if (!result.fully_filled) {
        result.error = "Insufficient liquidity";
    }
    
    return result;
}
