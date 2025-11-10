#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "OrderBook.h"
#include "Trade.h"

class MatchingEngine {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    
    MatchingEngine();
    
    // Submit an order for matching
    std::vector<Trade> submitOrder(Order& order, OrderBook& orderBook);
    
    // Set callback for trade notifications
    void setTradeCallback(TradeCallback callback) { tradeCallback_ = callback; }
    
private:
    TradeCallback tradeCallback_;
    
    // Match a buy order against sell orders
    std::vector<Trade> matchBuyOrder(Order& buyOrder, OrderBook& orderBook);
    
    // Match a sell order against buy orders
    std::vector<Trade> matchSellOrder(Order& sellOrder, OrderBook& orderBook);
    
    // Create a trade from two orders
    Trade createTrade(const Order& buyOrder, const Order& sellOrder, 
                      double price, double quantity);
    
    // Generate unique trade ID
    std::string generateTradeId();
    
    int tradeIdCounter_;
};

#endif // MATCHING_ENGINE_H

