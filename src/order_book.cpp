#include "order_book.hpp"
#include <algorithm>

void OrderBook::clear() {
    std::unique_lock lock(mutex_);
    bids_.clear();
    asks_.clear();
}

void OrderBook::addBid(Price price, Quantity size, Exchange exchange) {
    std::unique_lock lock(mutex_);
    bids_.emplace(price, PriceLevel(price, size, exchange));
}

void OrderBook::addAsk(Price price, Quantity size, Exchange exchange) {
    std::unique_lock lock(mutex_);
    asks_.emplace(price, PriceLevel(price, size, exchange));
}

std::vector<PriceLevel> OrderBook::getBids() const {
    std::shared_lock lock(mutex_);  // Multiple readers allowed
    std::vector<PriceLevel> result;
    result.reserve(bids_.size());
    for (const auto& [price, level] : bids_) {
        result.push_back(level);
    }
    return result;
}

std::vector<PriceLevel> OrderBook::getAsks() const {
    std::shared_lock lock(mutex_);
    std::vector<PriceLevel> result;
    result.reserve(asks_.size());
    for (const auto& [price, level] : asks_) {
        result.push_back(level);
    }
    return result;
}

void OrderBook::mergeBids(const std::vector<PriceLevel>& bids) {
    std::unique_lock lock(mutex_);
    for (const auto& bid : bids) {
        bids_.emplace(bid.price, bid);
    }
}

void OrderBook::mergeAsks(const std::vector<PriceLevel>& asks) {
    std::unique_lock lock(mutex_);
    for (const auto& ask : asks) {
        asks_.emplace(ask.price, ask);
    }
}

size_t OrderBook::bidDepth() const {
    std::shared_lock lock(mutex_);
    return bids_.size();
}

size_t OrderBook::askDepth() const {
    std::shared_lock lock(mutex_);
    return asks_.size();
}
