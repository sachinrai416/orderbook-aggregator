cmake_minimum_required(VERSION 3.15)
project(OrderBookAggregator VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimizations for low-latency (REMOVED -fno-exceptions)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        -O3
        -march=native
        -flto
        -ffast-math
        -funroll-loops
        -Wall -Wextra -Wpedantic
    )
    add_link_options(-flto)
endif()

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

set(SOURCES
    src/main.cpp
    src/order_book.cpp
    # src/exchange_factory.cpp
    src/exchanges/coinbase_client.cpp
    src/exchanges/gemini_client.cpp
    src/rate_limiter.cpp
    src/http_client.cpp
    src/price_calculator.cpp
)

add_executable(orderbook_aggregator ${SOURCES})

target_link_libraries(orderbook_aggregator
    PRIVATE
    CURL::libcurl
    Threads::Threads
)

set_target_properties(orderbook_aggregator PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

install(TARGETS orderbook_aggregator DESTINATION bin)
