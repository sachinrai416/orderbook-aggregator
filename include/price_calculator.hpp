#pragma once

#include "types.hpp"
#include <vector>
#include <string>

struct ExecutionResult {
    int64_t total_cost;      // In cents (fixed-point)
    Quantity quantity_filled; // In satoshis
    bool fully_filled;
    std::string error;
    
    [[nodiscard]] double getTotalCostUSD() const noexcept {
        return static_cast<double>(total_cost) / PRICE_SCALE;
    }
    
    [[nodiscard]] double getQuantityBTC() const noexcept {
        return static_cast<double>(quantity_filled) / QUANTITY_SCALE;
    }
};

class PriceCalculator {
public:
    static ExecutionResult calculateBuyPrice(
        const std::vector<PriceLevel>& asks, 
        Quantity quantity);
    
    static ExecutionResult calculateSellPrice(
        const std::vector<PriceLevel>& bids, 
        Quantity quantity);
};
