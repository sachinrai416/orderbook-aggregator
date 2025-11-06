#pragma once

#include "types.hpp"
#include <vector>
#include <map>
#include <shared_mutex>
#include <memory>

class OrderBook {
public:
    OrderBook() = default;
    
    void clear();
    void addBid(Price price, Quantity size, Exchange exchange);
    void addAsk(Price price, Quantity size, Exchange exchange);
    
    // Use shared_mutex for reader-writer lock
    std::vector<PriceLevel> getBids() const;
    std::vector<PriceLevel> getAsks() const;
    
    void mergeBids(const std::vector<PriceLevel>& bids);
    void mergeAsks(const std::vector<PriceLevel>& asks);
    
    size_t bidDepth() const;
    size_t askDepth() const;
    
private:
    mutable std::shared_mutex mutex_;  // Multiple readers, single writer
    
    // Multimap maintains sorted order: O(log n) insert
    std::multimap<Price, PriceLevel, std::greater<Price>> bids_;  // Descending
    std::multimap<Price, PriceLevel> asks_;  // Ascending
};
