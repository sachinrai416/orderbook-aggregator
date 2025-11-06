#pragma once

#include <string>
#include <cstdint>

// Use enum for exchange to save memory (1 byte vs 24+ bytes for std::string)
enum class Exchange : uint8_t {
    COINBASE = 0,
    GEMINI = 1,
    BINANCE = 2,  // Future
    KRAKEN = 3,   // Future
    UNKNOWN = 255
};

// Fixed-point price representation for deterministic calculations
// Store prices in cents (USD * 100) to avoid floating-point errors
using Price = int64_t;
using Quantity = int64_t;  // BTC in satoshis (BTC * 1e8)

constexpr int64_t PRICE_SCALE = 100;       // 2 decimal places
constexpr int64_t QUANTITY_SCALE = 100000000;  // 8 decimal places (satoshis)

// Price level with minimal memory footprint
struct PriceLevel {
    Price price;        // 8 bytes
    Quantity size;      // 8 bytes
    Exchange exchange;  // 1 byte
    // Total: 17 bytes (vs 48+ with std::string)
    
    PriceLevel(Price p, Quantity s, Exchange ex)
        : price(p), size(s), exchange(ex) {}
        
    // For compatibility with existing code
    [[nodiscard]] double getPriceAsDouble() const noexcept {
        return static_cast<double>(price) / PRICE_SCALE;
    }
    
    [[nodiscard]] double getSizeAsDouble() const noexcept {
        return static_cast<double>(size) / QUANTITY_SCALE;
    }
};

// Exchange metadata
struct ExchangeConfig {
    Exchange id;
    std::string name;
    std::string url;
    uint32_t rate_limit_ms;
    uint32_t timeout_ms;
};

inline const char* exchangeName(Exchange ex) {
    switch(ex) {
        case Exchange::COINBASE: return "Coinbase";
        case Exchange::GEMINI: return "Gemini";
        case Exchange::BINANCE: return "Binance";
        case Exchange::KRAKEN: return "Kraken";
        default: return "Unknown";
    }
}
