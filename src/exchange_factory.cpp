#include "exchange_factory.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::vector<std::unique_ptr<IExchangeClient>> ExchangeFactory::createFromConfig(
    const std::string& config_path) {
    
    std::vector<std::unique_ptr<IExchangeClient>> clients;
    
    // If no config file, use defaults
    if (config_path.empty()) {
        clients.push_back(createCoinbase());
        clients.push_back(createGemini());
        return clients;
    }
    
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open config file: " << config_path << "\n";
            std::cerr << "Using default exchanges (Coinbase, Gemini)\n";
            clients.push_back(createCoinbase());
            clients.push_back(createGemini());
            return clients;
        }
        
        json config;
        file >> config;
        
        if (!config.contains("exchanges")) {
            throw std::runtime_error("Config missing 'exchanges' array");
        }
        
        for (const auto& exchange : config["exchanges"]) {
            if (!exchange["enabled"].get<bool>()) {
                continue;
            }
            
            std::string id = exchange["id"].get<std::string>();
            
            if (id == "coinbase") {
                clients.push_back(createCoinbase());
            } else if (id == "gemini") {
                clients.push_back(createGemini());
            }
            // Add more exchanges here as they're implemented
            // else if (id == "binance") {
            //     clients.push_back(createBinance());
            // }
        }
        
        if (clients.empty()) {
            std::cerr << "Warning: No enabled exchanges found in config\n";
            std::cerr << "Using default exchanges (Coinbase, Gemini)\n";
            clients.push_back(createCoinbase());
            clients.push_back(createGemini());
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config: " << e.what() << "\n";
        std::cerr << "Using default exchanges (Coinbase, Gemini)\n";
        clients.push_back(createCoinbase());
        clients.push_back(createGemini());
    }
    
    return clients;
}
