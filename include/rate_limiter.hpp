// #pragma once

// #include <chrono>
// #include <atomic>
// #include <thread>
// #include <future>
// #include <functional>

// class RateLimiter {
// public:
//     explicit RateLimiter(std::chrono::milliseconds interval)
//         : interval_(interval)
//         , last_call_(std::chrono::steady_clock::now() - interval) {}
    
//     // Add move constructor and assignment
//     RateLimiter(RateLimiter&& other) noexcept
//         : interval_(other.interval_)
//         , last_call_(other.last_call_.load(std::memory_order_relaxed)) {}
    
//     RateLimiter& operator=(RateLimiter&& other) noexcept {
//         if (this != &other) {
//             interval_ = other.interval_;
//             last_call_.store(other.last_call_.load(std::memory_order_relaxed), 
//                            std::memory_order_relaxed);
//         }
//         return *this;
//     }
    
//     // Delete copy operations
//     RateLimiter(const RateLimiter&) = delete;
//     RateLimiter& operator=(const RateLimiter&) = delete;
    
//     bool canProceed() const noexcept {
//         auto now = std::chrono::steady_clock::now();
//         auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
//             now - last_call_.load(std::memory_order_relaxed));
//         return elapsed >= interval_;
//     }
    
//     template<typename Func>
//     auto execute(Func&& func) -> std::future<decltype(func())> {
//         using ReturnType = decltype(func());
        
//         return std::async(std::launch::async, 
//             [this, func = std::forward<Func>(func)]() mutable -> ReturnType {
//             while (!canProceed()) {
//                 std::this_thread::yield();
//             }
            
//             last_call_.store(std::chrono::steady_clock::now(), 
//                            std::memory_order_relaxed);
//             return func();
//         });
//     }
    
//     void reset() noexcept {
//         last_call_.store(
//             std::chrono::steady_clock::now() - interval_,
//             std::memory_order_relaxed);
//     }
    
// private:
//     std::chrono::milliseconds interval_;
//     std::atomic<std::chrono::steady_clock::time_point> last_call_;
// };


#pragma once

#include <chrono>
#include <atomic>
#include <thread>
#include <future>
#include <functional>

class RateLimiter {
public:
    explicit RateLimiter(std::chrono::milliseconds interval)
        : interval_(interval)
        , last_call_(std::chrono::steady_clock::now() - interval) {}
    
    RateLimiter(RateLimiter&& other) noexcept
        : interval_(other.interval_)
        , last_call_(other.last_call_.load(std::memory_order_relaxed)) {}
    
    RateLimiter& operator=(RateLimiter&& other) noexcept {
        if (this != &other) {
            interval_ = other.interval_;
            last_call_.store(other.last_call_.load(std::memory_order_relaxed), 
                           std::memory_order_relaxed);
        }
        return *this;
    }
    
    RateLimiter(const RateLimiter&) = delete;
    RateLimiter& operator=(const RateLimiter&) = delete;
    
    bool canProceed() const noexcept {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_call_.load(std::memory_order_acquire));
        return elapsed >= interval_;
    }
    
    template<typename Func>
    auto execute(Func&& func) -> std::future<decltype(func())> {
        using ReturnType = decltype(func());
        
        return std::async(std::launch::async, 
            [this, func = std::forward<Func>(func)]() mutable -> ReturnType {
            
            // FIXED: Use compare-and-swap for thread safety
            auto now = std::chrono::steady_clock::now();
            auto expected = last_call_.load(std::memory_order_acquire);
            
            while (true) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - expected);
                
                if (elapsed >= interval_) {
                    // Try to atomically update last_call_
                    if (last_call_.compare_exchange_weak(expected, now, 
                        std::memory_order_release, std::memory_order_acquire)) {
                        // Successfully claimed the time slot
                        break;
                    }
                    // CAS failed, another thread updated it, retry
                } else {
                    // Not enough time elapsed, wait
                    std::this_thread::yield();
                }
                
                now = std::chrono::steady_clock::now();
                expected = last_call_.load(std::memory_order_acquire);
            }
            
            return func();
        });
    }
    
    void reset() noexcept {
        last_call_.store(
            std::chrono::steady_clock::now() - interval_,
            std::memory_order_release);
    }
    
private:
    std::chrono::milliseconds interval_;
    std::atomic<std::chrono::steady_clock::time_point> last_call_;
};
