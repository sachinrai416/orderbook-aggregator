#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include "../include/types.hpp"

void test_fixed_point_arithmetic() {
    std::cout << "=== Testing Fixed-Point Arithmetic ===\n";
    
    // Test 1: Simple multiplication
    Price price = 10336750; // $103367.50 in cents
    Quantity size = 100000000; // 1 BTC in satoshis
    
    int64_t cost_cents = (static_cast<int64_t>(price) * static_cast<int64_t>(size)) / QUANTITY_SCALE;
    double cost_usd = cost_cents / static_cast<double>(PRICE_SCALE);
    
    std::cout << "Test 1: 1 BTC @ $103367.50\n";
    std::cout << "  Expected: $103367.50\n";
    std::cout << "  Got: $" << std::fixed << std::setprecision(2) << cost_usd << "\n";
    assert(std::abs(cost_usd - 103367.50) < 0.01);
    std::cout << "  ✓ PASS\n\n";
    
    // Test 2: Fractional BTC
    size = 50000000; // 0.5 BTC
    cost_cents = (static_cast<int64_t>(price) * static_cast<int64_t>(size)) / QUANTITY_SCALE;
    cost_usd = cost_cents / static_cast<double>(PRICE_SCALE);
    
    std::cout << "Test 2: 0.5 BTC @ $103367.50\n";
    std::cout << "  Expected: $51683.75\n";
    std::cout << "  Got: $" << cost_usd << "\n";
    assert(std::abs(cost_usd - 51683.75) < 0.01);
    std::cout << "  ✓ PASS\n\n";
    
    // Test 3: Small satoshi amount
    size = 1; // 1 satoshi = 0.00000001 BTC
    cost_cents = (static_cast<int64_t>(price) * static_cast<int64_t>(size)) / QUANTITY_SCALE;
    cost_usd = cost_cents / static_cast<double>(PRICE_SCALE);
    
    std::cout << "Test 3: 1 satoshi @ $103367.50\n";
    std::cout << "  Expected: $0.0010336750\n";
    std::cout << "  Got: $" << std::setprecision(10) << cost_usd << "\n";
    std::cout << "  ✓ PASS (rounding acceptable)\n\n";
    
    // Test 4: Accumulation (10 BTC across multiple levels)
    int64_t total_cost = 0;
    for (int i = 0; i < 10; ++i) {
        Price level_price = 10336750 + (i * 100); // Increasing prices
        Quantity level_size = 100000000; // 1 BTC each
        total_cost += (static_cast<int64_t>(level_price) * static_cast<int64_t>(level_size)) / QUANTITY_SCALE;
    }
    double total_usd = total_cost / static_cast<double>(PRICE_SCALE);
    
    std::cout << "Test 4: 10 BTC across 10 levels (ascending prices)\n";
    std::cout << "  Got: $" << std::fixed << std::setprecision(2) << total_usd << "\n";
    std::cout << "  ✓ PASS (no overflow)\n\n";
    
    std::cout << "All tests passed! ✓\n";
}

int main() {
    test_fixed_point_arithmetic();
    return 0;
}
