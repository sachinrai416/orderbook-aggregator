#pragma once

#include "exchange_interface.hpp"
#include "types.hpp"
#include <memory>
#include <vector>

class ExchangeFactory {
public:
    static std::unique_ptr<IExchangeClient> createCoinbase();
    static std::unique_ptr<IExchangeClient> createGemini();
    
    // Factory method for configuration-driven creation
    static std::vector<std::unique_ptr<IExchangeClient>> createFromConfig(
        const std::string& config_path);
};
