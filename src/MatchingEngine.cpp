#include "MatchingEngine.h"
#include <sstream>
#include <iomanip>
#include <chrono>

MatchingEngine::MatchingEngine() : tradeIdCounter_(0) {
}

std::vector<Trade> MatchingEngine::submitOrder(Order& order, OrderBook& orderBook) {
    std::vector<Trade> trades;
    
    if (order.side == OrderSide::BUY) {
        trades = matchBuyOrder(order, orderBook);
    } else {
        trades = matchSellOrder(order, orderBook);
    }
    
    // If order is not fully filled and it's a limit order, add to order book
    if (order.status != OrderStatus::FILLED && 
        order.status != OrderStatus::CANCELLED &&
        order.type == OrderType::LIMIT &&
        order.filledQuantity < order.quantity) {
        order.status = OrderStatus::PENDING;
        orderBook.addOrder(order);
    }
    
    // Notify about trades
    for (const auto& trade : trades) {
        if (tradeCallback_) {
            tradeCallback_(trade);
        }
    }
    
    return trades;
}

std::vector<Trade> MatchingEngine::matchBuyOrder(Order& buyOrder, OrderBook& orderBook) {
    std::vector<Trade> trades;
    
    if (buyOrder.quantity <= 0 || buyOrder.filledQuantity >= buyOrder.quantity) {
        return trades;
    }
    
    double remainingQuantity = buyOrder.quantity - buyOrder.filledQuantity;
    auto sellOrders = orderBook.getSellOrders();
    
    for (auto& sellOrder : sellOrders) {
        if (remainingQuantity <= 0) break;
        
        // For limit orders, check price
        if (buyOrder.type == OrderType::LIMIT) {
            if (buyOrder.price < sellOrder.price) {
                break; // No more matches possible
            }
        }
        
        Order* sellOrderPtr = orderBook.getOrder(sellOrder.orderId);
        if (!sellOrderPtr) continue;
        
        double availableQuantity = sellOrder.quantity - sellOrder.filledQuantity;
        if (availableQuantity <= 0) continue;
        
        // Determine match price (price-time priority)
        double matchPrice = (buyOrder.type == OrderType::MARKET) ? 
                           sellOrder.price : 
                           std::min(buyOrder.price, sellOrder.price);
        
        double matchQuantity = std::min(remainingQuantity, availableQuantity);
        
        // Create trade
        Trade trade = createTrade(buyOrder, *sellOrderPtr, matchPrice, matchQuantity);
        trades.push_back(trade);
        
        // Update order quantities
        buyOrder.filledQuantity += matchQuantity;
        sellOrderPtr->filledQuantity += matchQuantity;
        
        // Update order status
        if (buyOrder.filledQuantity >= buyOrder.quantity) {
            buyOrder.status = OrderStatus::FILLED;
        } else {
            buyOrder.status = OrderStatus::PARTIALLY_FILLED;
        }
        
        if (sellOrderPtr->filledQuantity >= sellOrderPtr->quantity) {
            sellOrderPtr->status = OrderStatus::FILLED;
            orderBook.removeOrder(sellOrder.orderId);
        } else {
            sellOrderPtr->status = OrderStatus::PARTIALLY_FILLED;
        }
        
        remainingQuantity -= matchQuantity;
    }
    
    return trades;
}

std::vector<Trade> MatchingEngine::matchSellOrder(Order& sellOrder, OrderBook& orderBook) {
    std::vector<Trade> trades;
    
    if (sellOrder.quantity <= 0 || sellOrder.filledQuantity >= sellOrder.quantity) {
        return trades;
    }
    
    double remainingQuantity = sellOrder.quantity - sellOrder.filledQuantity;
    auto buyOrders = orderBook.getBuyOrders();
    
    for (auto& buyOrder : buyOrders) {
        if (remainingQuantity <= 0) break;
        
        // For limit orders, check price
        if (sellOrder.type == OrderType::LIMIT) {
            if (sellOrder.price > buyOrder.price) {
                break; // No more matches possible
            }
        }
        
        Order* buyOrderPtr = orderBook.getOrder(buyOrder.orderId);
        if (!buyOrderPtr) continue;
        
        double availableQuantity = buyOrder.quantity - buyOrder.filledQuantity;
        if (availableQuantity <= 0) continue;
        
        // Determine match price (price-time priority)
        // When a SELL order matches a BUY order, use the better (lower) price
        double matchPrice = (sellOrder.type == OrderType::MARKET) ? 
                           buyOrder.price : 
                           std::min(sellOrder.price, buyOrder.price);
        
        double matchQuantity = std::min(remainingQuantity, availableQuantity);
        
        // Create trade
        Trade trade = createTrade(*buyOrderPtr, sellOrder, matchPrice, matchQuantity);
        trades.push_back(trade);
        
        // Update order quantities
        sellOrder.filledQuantity += matchQuantity;
        buyOrderPtr->filledQuantity += matchQuantity;
        
        // Update order status
        if (sellOrder.filledQuantity >= sellOrder.quantity) {
            sellOrder.status = OrderStatus::FILLED;
        } else {
            sellOrder.status = OrderStatus::PARTIALLY_FILLED;
        }
        
        if (buyOrderPtr->filledQuantity >= buyOrderPtr->quantity) {
            buyOrderPtr->status = OrderStatus::FILLED;
            orderBook.removeOrder(buyOrder.orderId);
        } else {
            buyOrderPtr->status = OrderStatus::PARTIALLY_FILLED;
        }
        
        remainingQuantity -= matchQuantity;
    }
    
    return trades;
}

Trade MatchingEngine::createTrade(const Order& buyOrder, const Order& sellOrder, 
                                   double price, double quantity) {
    Trade trade;
    trade.tradeId = generateTradeId();
    trade.buyOrderId = buyOrder.orderId;
    trade.sellOrderId = sellOrder.orderId;
    trade.buyTraderId = buyOrder.traderId;
    trade.sellTraderId = sellOrder.traderId;
    trade.symbol = buyOrder.symbol;
    trade.price = price;
    trade.quantity = quantity;
    trade.timestamp = std::chrono::system_clock::now();
    return trade;
}

std::string MatchingEngine::generateTradeId() {
    std::ostringstream oss;
    oss << "TRADE_" << std::setfill('0') << std::setw(8) << tradeIdCounter_++;
    return oss.str();
}

