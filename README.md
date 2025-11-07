# BTC-USD Order Book Aggregator

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A high-performance, enterprise-grade order book aggregator for cryptocurrency exchanges, designed for real-time price discovery and liquidity aggregation across multiple trading venues.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
  - [macOS](#macos)
  - [Linux (Ubuntu/Debian)](#linux-ubuntudebian)
  - [Windows](#windows)
- [Building from Source](#building-from-source)
- [Usage](#usage)
- [Configuration](#configuration)
- [Architecture](#architecture)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## 🎯 Overview

This project implements a **BTC-USD order book aggregator** that:
- Fetches live order book data from multiple cryptocurrency exchanges (Coinbase, Gemini)
- Aggregates liquidity across exchanges for optimal price discovery
- Calculates the best execution price for buying or selling a specified quantity of Bitcoin
- Implements robust rate limiting to comply with exchange API restrictions
- Uses fixed-point arithmetic for deterministic financial calculations

**Use Case**: Pricing engine for crypto trading platforms that source liquidity from multiple exchanges.

---

## ✨ Features

### Core Functionality
- **Multi-Exchange Support**: Currently supports Coinbase and Gemini, extensible to additional exchanges
- **Real-Time Aggregation**: Combines order books from multiple sources in real-time
- **Optimal Execution**: Calculates best prices by walking through aggregated liquidity
- **Command-Line Interface**: Configurable quantity via `--qty` parameter

### Performance Optimizations
- **Fixed-Point Arithmetic**: Prices stored in cents (USD × 100), quantities in satoshis (BTC × 10⁸) to eliminate floating-point errors
- **Memory Efficiency**: Enum-based exchange IDs (1 byte vs 24+ bytes for strings)
- **Connection Pooling**: Reusable CURL handles with HTTP/2 support
- **Concurrent Access**: Reader-writer locks (`std::shared_mutex`) for thread-safe order book operations
- **Lock-Free Rate Limiting**: Non-blocking rate limiter using atomic compare-and-swap operations
- **Compiler Optimizations**: Built with `-O3 -march=native -flto -ffast-math`

### Scalability Features
- **Plugin Architecture**: Factory pattern enables easy addition of new exchanges
- **Configuration-Driven**: JSON config file for exchange endpoints and settings
- **Parallel Fetching**: Order books fetched concurrently using `std::async`
- **Thread-Safe Design**: All shared data structures protected with appropriate synchronization primitives

### Robustness
- **Rate Limiting**: Configurable rate limiter (default: 1 request per 2 seconds per exchange)
- **Error Handling**: Graceful degradation if one exchange fails
- **Retry Logic**: Automatic retry with exponential backoff for network failures
- **Validation**: Input validation and malformed data handling

---

## 💻 System Requirements

### Minimum Requirements
- **Operating System**: macOS 10.15+, Ubuntu 18.04+, Windows 10+, or any modern Linux distribution
- **Compiler**: 
  - GCC 7.0+ (with C++17 support)
  - Clang 5.0+ (with C++17 support)
  - MSVC 2017+ (with C++17 support)
- **CMake**: 3.15 or higher
- **RAM**: 256 MB (minimal footprint)
- **Disk Space**: 50 MB for build artifacts

### Dependencies
- **libcurl**: 7.58.0+ (for HTTP requests)
- **pthread**: POSIX threads library (usually pre-installed)
- **OpenSSL**: 1.1.0+ (for HTTPS support, typically included with libcurl)

---

## 🚀 Installation

### macOS

#### Install Dependencies

Using **Homebrew** (recommended):

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake curl openssl
```

Using **MacPorts**:

```bash
sudo port install cmake curl openssl
```

#### Build the Project

```bash
# Clone the repository
git clone https://github.com/sachinrai416/orderbook-aggregator.git
cd orderbook-aggregator

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the project (use -j for parallel compilation)
cmake --build . -j$(sysctl -n hw.ncpu)

# The executable will be at: build/orderbook_aggregator
```

---

### Linux (Ubuntu/Debian)

#### Install Dependencies

```bash
# Update package lists
sudo apt-get update

# Install build essentials and dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    libcurl4-openssl-dev \
    libssl-dev \
    pkg-config

# Optional: Install additional tools
sudo apt-get install -y git
```

#### For Other Linux Distributions

**Fedora/RHEL/CentOS**:
```bash
sudo dnf install gcc gcc-c++ cmake libcurl-devel openssl-devel
```

**Arch Linux**:
```bash
sudo pacman -S gcc cmake curl openssl
```

#### Build the Project

```bash
# Clone the repository
git clone https://github.com/sachinrai416/orderbook-aggregator.git
cd orderbook-aggregator

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the project (use all available cores)
cmake --build . -j$(nproc)

# The executable will be at: build/orderbook_aggregator
```

---

### Windows

#### Prerequisites

1. **Install Visual Studio 2017 or later**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"
   - Ensure C++ CMake tools are installed

2. **Install CMake**
   - Download from: https://cmake.org/download/
   - During installation, select "Add CMake to system PATH"

3. **Install vcpkg** (C++ Package Manager)

```powershell
# Open PowerShell as Administrator
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### Install Dependencies via vcpkg

```powershell
# Install libcurl
.\vcpkg install curl:x64-windows
```

#### Build the Project

**Option 1: Using Visual Studio**

1. Open Visual Studio
2. Select "Open a local folder" → Navigate to the cloned repository
3. Visual Studio will automatically detect CMakeLists.txt
4. Press `Ctrl+Shift+B` to build
5. The executable will be in: `out\build\x64-Debug\orderbook_aggregator.exe`

**Option 2: Using Command Line**

```powershell
# Clone the repository
git clone https://github.com/sachinrai416/orderbook-aggregator.git
cd orderbook-aggregator

# Create build directory
mkdir build
cd build

# Configure with CMake (specify vcpkg toolchain)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build the project
cmake --build . --config Release

# The executable will be at: build\Release\orderbook_aggregator.exe
```

---

## 🔨 Building from Source

### Quick Build

```bash
# From repository root
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)  # Linux/macOS

# On Windows
cmake --build . --config Release
```

### Build Options

#### Debug Build (with symbols and logging)

```bash
mkdir -p build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG_ORDERBOOK=ON ..
cmake --build .
```

The `-DDEBUG_ORDERBOOK=ON` flag enables detailed order book logging.

#### Release Build (optimized)

```bash
mkdir -p build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

#### Custom Compiler

```bash
# Use Clang
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Use GCC
cmake -DCMAKE_CXX_COMPILER=g++ ..
```

#### Install to System

```bash
# After building
sudo cmake --install .

# This installs to /usr/local/bin by default
# Run from anywhere:
orderbook_aggregator --qty 5
```

### Clean Build

```bash
# Remove build directory
rm -rf build

# Rebuild from scratch
mkdir build && cd build
cmake .. && cmake --build .
```

---

## 📖 Usage

### Basic Usage

Run with default quantity (10 BTC):

```bash
./orderbook_aggregator
```

**Example Output**:
```
To buy 10 BTC: $1,033,675.50
To sell 10 BTC: $1,032,148.75
```

### Custom Quantity

Specify a custom quantity using the `--qty` flag:

```bash
# Buy/Sell 5 BTC
./orderbook_aggregator --qty 5

# Buy/Sell 0.1 BTC
./orderbook_aggregator --qty 0.1

# Buy/Sell 100 BTC
./orderbook_aggregator --qty 100
```

### Advanced Usage

#### Debug Mode

To see detailed order book information and execution breakdown:

```bash
# Build with debug flag
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG_ORDERBOOK=ON ..
cmake --build .

# Run
./orderbook_aggregator --qty 10
```

**Debug Output Example**:
```
Coinbase Order Book:
  Bids: 50 levels
  Asks: 50 levels
  Best Bid: $103267.50
  Best Ask: $103368.75

Gemini Order Book:
  Bids: 45 levels
  Asks: 48 levels
  Best Bid: $103265.00
  Best Ask: $103370.25

Aggregated Order Book:
  Total Bids: 95 levels
  Total Asks: 98 levels
  Best Aggregated Bid: $103267.50
  Best Aggregated Ask: $103368.75

=== BUY EXECUTION ===
Target: 10 BTC
Total ask levels: 98
Level 1: 2.5 BTC @ $103368.75 = $258421.88 (Coinbase)
Level 2: 5.0 BTC @ $103370.00 = $516850.00 (Gemini)
Level 3: 2.5 BTC @ $103372.50 = $258431.25 (Coinbase)
Total cost: $1,033,703.13

To buy 10 BTC: $1,033,703.13
To sell 10 BTC: $1,032,148.75
```

#### Scripting / Automation

```bash
#!/bin/bash
# price_monitor.sh - Monitor BTC prices every 5 minutes

while true; do
    echo "$(date): Fetching BTC prices..."
    ./orderbook_aggregator --qty 1
    sleep 300  # Wait 5 minutes
done
```

#### Integration with Other Tools

```bash
# Parse output for buy price only
BUY_PRICE=$(./orderbook_aggregator --qty 10 | grep "To buy" | awk '{print $6}' | tr -d '$,')
echo "Buy price: $BUY_PRICE"

# Use in a script
if (( $(echo "$BUY_PRICE < 1000000" | bc -l) )); then
    echo "BTC price below $1M threshold!"
fi
```

---

## ⚙️ Configuration

### Exchange Configuration

The application supports configuration via `config/exchanges.json`. By default, it uses hardcoded Coinbase and Gemini endpoints, but you can customize behavior:

**Default Configuration** (`config/exchanges.json`):

```json
{
  "version": "2.0",
  "global_settings": {
    "default_timeout_ms": 5000,
    "default_rate_limit_ms": 2000,
    "max_retries": 3,
    "connection_pool_size": 10,
    "enable_http2": true
  },
  "exchanges": [
    {
      "id": "coinbase",
      "name": "Coinbase",
      "enabled": true,
      "order_book_config": {
        "full_url": "https://api.exchange.coinbase.com/products/BTC-USD/book?level=2"
      },
      "rate_limits": {
        "interval_ms": 2000
      }
    },
    {
      "id": "gemini",
      "name": "Gemini",
      "enabled": true,
      "order_book_config": {
        "full_url": "https://api.gemini.com/v1/book/BTCUSD"
      },
      "rate_limits": {
        "interval_ms": 2000
      }
    }
  ]
}
```

### Enabling/Disabling Exchanges

To disable an exchange, set `"enabled": false` in the config file.

### Adding New Exchanges

The system is designed to be extensible. See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed instructions on adding new exchanges.

---

## 🏗️ Architecture

This project follows a modular, layered architecture with clear separation of concerns:

### High-Level Components

1. **Exchange Layer**: Abstraction for different cryptocurrency exchanges
2. **Network Layer**: HTTP client with connection pooling and retry logic
3. **Data Layer**: Thread-safe order book aggregation
4. **Rate Limiting**: Lock-free rate limiter for API compliance
5. **Calculation Engine**: Fixed-point arithmetic for price calculations

### Key Design Patterns

- **Factory Pattern**: For creating exchange clients
- **Strategy Pattern**: For different exchange API formats
- **Singleton Pattern**: For HTTP client pooling
- **Template Method**: For rate limiter execution

For detailed architecture documentation, see **[ARCHITECTURE.md](ARCHITECTURE.md)**.

---

## 🧪 Testing

### Manual Testing

Test with different quantities:

```bash
# Test edge cases
./orderbook_aggregator --qty 0.001  # Small quantity
./orderbook_aggregator --qty 1000   # Large quantity (may exceed liquidity)
```

### Verify API Responses

You can manually verify the exchange APIs:

```bash
# Coinbase order book
curl "https://api.exchange.coinbase.com/products/BTC-USD/book?level=2"

# Gemini order book
curl "https://api.gemini.com/v1/book/BTCUSD"
```

### Rate Limiting Test

The rate limiter ensures no more than 1 request per 2 seconds per exchange. You can verify this by running multiple times in quick succession.

### Network Failure Test

Disconnect your network and run the program to see error handling:

```bash
# Disable network
sudo ifconfig en0 down  # macOS
sudo ip link set dev eth0 down  # Linux

# Run program (should show error messages)
./orderbook_aggregator

# Re-enable network
sudo ifconfig en0 up  # macOS
sudo ip link set dev eth0 up  # Linux
```

---

## 🔧 Troubleshooting

### Common Issues

#### Issue: "command not found: cmake"

**Solution**: Install CMake
```bash
# macOS
brew install cmake

# Ubuntu/Debian
sudo apt-get install cmake

# Windows: Download from https://cmake.org/download/
```

#### Issue: "curl/curl.h: No such file or directory"

**Solution**: Install libcurl development headers
```bash
# macOS
brew install curl

# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# Windows
vcpkg install curl:x64-windows
```

#### Issue: "Fatal error: Failed to fetch data from any exchange"

**Possible Causes**:
1. **No internet connection**: Check your network
2. **Exchange API down**: Verify exchanges are operational
3. **Firewall blocking**: Check firewall settings
4. **Rate limiting**: Wait 2+ seconds between runs

**Solution**:
```bash
# Test connectivity
curl -v "https://api.exchange.coinbase.com/products/BTC-USD/book?level=2"

# Check if you're being rate limited
# Wait at least 2 seconds between program executions
```

#### Issue: "Insufficient liquidity"

This means there isn't enough liquidity in the order books to fill your order.

**Solution**: Reduce the quantity
```bash
./orderbook_aggregator --qty 1  # Try smaller quantity
```

#### Issue: Build fails with "unsupported C++17 features"

**Solution**: Update your compiler
```bash
# Check GCC version (should be 7.0+)
g++ --version

# Ubuntu: Update to newer GCC
sudo apt-get install gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
```

#### Issue: Windows build fails with "CURL not found"

**Solution**: Ensure vcpkg is integrated properly
```powershell
# Re-integrate vcpkg
cd C:\vcpkg
.\vcpkg integrate install

# Rebuild
cd path\to\orderbook-aggregator\build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Performance Issues

#### Slow execution (> 5 seconds)

**Possible Causes**:
- Network latency to exchange APIs
- Rate limiter waiting for cooldown period

**Solution**:
- Ensure you have good internet connectivity
- Check if you're running the program too frequently (< 2 seconds apart)

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

### Adding New Exchanges

1. Create a new client class inheriting from `IExchangeClient`
2. Implement `fetchOrderBook()` method
3. Parse the exchange's specific JSON format
4. Add factory method in `ExchangeFactory`
5. Update configuration file

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed instructions.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 📊 Project Status

- ✅ Core functionality complete
- ✅ Coinbase and Gemini integration
- ✅ Rate limiting implemented
- ✅ Fixed-point arithmetic
- ✅ Thread-safe operations
- 🔄 Additional exchanges (planned)
- 🔄 WebSocket support (planned)
- 🔄 Real-time streaming (planned)

---
