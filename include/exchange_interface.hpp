#pragma once

#include "types.hpp"
#include <string>
#include <vector>

// OrderBookSnapshot remains the same
struct OrderBookSnapshot {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
    int64_t timestamp_us;
    bool success;
    std::string error;
    
    OrderBookSnapshot() 
        : timestamp_us(0), success(false) {}
};

// CRTP Base Class - No virtual functions!
template <typename Derived>
class ExchangeClientBase {
public:
    // Static polymorphism - resolved at compile-time
    OrderBookSnapshot fetchOrderBook() {
        return static_cast<Derived*>(this)->fetchOrderBookImpl();
    }
    
    Exchange getExchangeId() const {
        return static_cast<const Derived*>(this)->getExchangeIdImpl();
    }
    
    std::string getName() const {
        return static_cast<const Derived*>(this)->getNameImpl();
    }
    
    // No virtual destructor needed - no vtable!
    ~ExchangeClientBase() = default;
    
protected:
    ExchangeClientBase() = default;
};
