#ifndef TRADE_H
#define TRADE_H

#include <string>
#include <chrono>

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT
};

enum class OrderStatus {
    PENDING,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct Order {
    std::string orderId;
    std::string traderId;
    std::string symbol;
    OrderSide side;
    OrderType type;
    double price;      // For limit orders
    double quantity;
    double filledQuantity;
    OrderStatus status;
    std::chrono::system_clock::time_point timestamp;
    
    Order() : price(0.0), quantity(0.0), filledQuantity(0.0), 
              status(OrderStatus::PENDING) {}
};

struct Trade {
    std::string tradeId;
    std::string buyOrderId;
    std::string sellOrderId;
    std::string buyTraderId;
    std::string sellTraderId;
    std::string symbol;
    double price;
    double quantity;
    std::chrono::system_clock::time_point timestamp;
    
    Trade() : price(0.0), quantity(0.0) {}
};

#endif // TRADE_H

