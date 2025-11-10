# Testing Documentation

## Test Suite Overview

A comprehensive test suite has been created using Google Test framework that covers:

1. **Basic Order Matching** - BUY and SELL LIMIT orders that match
2. **Orders Stay in OrderBook** - Orders that don't match remain in orderbook
3. **Partial Fills** - Orders that partially fill
4. **Market Orders** - MARKET order matching
5. **Multiple Orders Matching** - Multiple orders matching against one large order
6. **Price Priority** - Better prices match first
7. **Insufficient Funds** - Handling of insufficient funds
8. **Multiple Symbols** - Trading across different symbols

## Building Tests

```bash
cd build
cmake ..
make market_tests
```

## Running Tests

```bash
./market_tests
```

## Test Structure

- **TestClient** (`tests/TestClient.h/cpp`): Helper class for socket communication with the market server
- **test_market.cpp**: Main test file with all test cases

## Known Issues

The tests are currently experiencing some issues:
- Connection timing issues - server may not be ready when clients connect
- Thread synchronization - accessing server state from test thread while server runs in separate thread
- Orderbook state access - need to ensure thread-safe access to orderbook

## Future Improvements

1. Add proper synchronization for server state access
2. Implement retry logic for connections
3. Add more comprehensive error handling
4. Test edge cases (negative prices, zero quantities, etc.)
5. Add performance tests

