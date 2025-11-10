#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Trade.h"

class OrderBook {
public:
    OrderBook(const std::string& symbol);
    
    const std::string& getSymbol() const { return symbol_; }
    
    // Add order to the book
    void addOrder(const Order& order);
    
    // Remove order from the book
    bool removeOrder(const std::string& orderId);
    
    // Get best bid (highest buy price)
    double getBestBid() const;
    
    // Get best ask (lowest sell price)
    double getBestAsk() const;
    
    // Get all buy orders sorted by price (descending)
    std::vector<Order> getBuyOrders() const;
    
    // Get all sell orders sorted by price (ascending)
    std::vector<Order> getSellOrders() const;
    
    // Get order by ID
    Order* getOrder(const std::string& orderId);
    
private:
    std::string symbol_;
    
    // Buy orders: price -> vector of orders (sorted by time)
    std::map<double, std::vector<Order>, std::greater<double>> buyOrders_;
    
    // Sell orders: price -> vector of orders (sorted by time)
    std::map<double, std::vector<Order>> sellOrders_;
    
    // Quick lookup: orderId -> iterator
    std::map<std::string, std::pair<double, size_t>> orderIndex_;
    
    void indexOrder(const std::string& orderId, double price, size_t position, OrderSide side);
    void removeFromIndex(const std::string& orderId);
};

#endif // ORDERBOOK_H

