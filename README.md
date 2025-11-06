# BTC-USD Order Book Aggregator v2.0

Enterprise-grade C++ order book aggregator optimized for low-latency trading systems.

## Architecture Highlights

### Performance Optimizations
- **Fixed-Point Arithmetic**: Prices in cents, quantities in satoshis (no float errors)
- **Memory Efficiency**: Enum-based exchange IDs (1 byte vs 24+ bytes)
- **Connection Pooling**: Reusable CURL handles with HTTP/2 support
- **Reader-Writer Locks**: `std::shared_mutex` for concurrent access
- **Zero-Copy Parsing**: Direct JSON-to-fixed-point conversion
- **Compiler Flags**: `-O3 -march=native -flto -ffast-math`

### Scalability Features
- **Plugin Architecture**: Factory pattern for adding exchanges
- **Configuration-Driven**: JSON config for exchange endpoints
- **Thread-Safe**: Lock-free rate limiter, shared mutex order book
- **Horizontal Scaling**: Parallel fetching with futures
- **Extensible**: Interface-based design for WebSocket support

## Build Instructions

