#pragma once

#include "types.hpp"
#include <vector>
#include <string>
#include <optional>

struct OrderBookSnapshot {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
    int64_t timestamp_us;  // Microsecond precision
    bool success;
    std::string error;
    
    OrderBookSnapshot() 
        : timestamp_us(0), success(false) {}
};

class IExchangeClient {
public:
    virtual ~IExchangeClient() = default;
    virtual OrderBookSnapshot fetchOrderBook() = 0;
    virtual Exchange getExchangeId() const = 0;
    virtual std::string getName() const = 0;
};
