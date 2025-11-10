#include "OrderBook.h"
#include <algorithm>

OrderBook::OrderBook(const std::string& symbol) : symbol_(symbol) {
}

void OrderBook::addOrder(const Order& order) {
    if (order.side == OrderSide::BUY) {
        buyOrders_[order.price].push_back(order);
        indexOrder(order.orderId, order.price, buyOrders_[order.price].size() - 1, OrderSide::BUY);
    } else {
        sellOrders_[order.price].push_back(order);
        indexOrder(order.orderId, order.price, sellOrders_[order.price].size() - 1, OrderSide::SELL);
    }
}

bool OrderBook::removeOrder(const std::string& orderId) {
    auto it = orderIndex_.find(orderId);
    if (it == orderIndex_.end()) {
        return false;
    }
    
    double price = it->second.first;
    size_t position = it->second.second;
    
    // Try to find in buy orders
    auto buyIt = buyOrders_.find(price);
    if (buyIt != buyOrders_.end() && position < buyIt->second.size()) {
        if (buyIt->second[position].orderId == orderId) {
            buyIt->second.erase(buyIt->second.begin() + position);
            if (buyIt->second.empty()) {
                buyOrders_.erase(buyIt);
            } else {
                // Reindex remaining orders at this price level
                for (size_t i = position; i < buyIt->second.size(); ++i) {
                    orderIndex_[buyIt->second[i].orderId] = {price, i};
                }
            }
            orderIndex_.erase(it);
            return true;
        }
    }
    
    // Try to find in sell orders
    auto sellIt = sellOrders_.find(price);
    if (sellIt != sellOrders_.end() && position < sellIt->second.size()) {
        if (sellIt->second[position].orderId == orderId) {
            sellIt->second.erase(sellIt->second.begin() + position);
            if (sellIt->second.empty()) {
                sellOrders_.erase(sellIt);
            } else {
                // Reindex remaining orders at this price level
                for (size_t i = position; i < sellIt->second.size(); ++i) {
                    orderIndex_[sellIt->second[i].orderId] = {price, i};
                }
            }
            orderIndex_.erase(it);
            return true;
        }
    }
    
    return false;
}

double OrderBook::getBestBid() const {
    if (buyOrders_.empty()) {
        return 0.0;
    }
    return buyOrders_.begin()->first;
}

double OrderBook::getBestAsk() const {
    if (sellOrders_.empty()) {
        return 0.0;
    }
    return sellOrders_.begin()->first;
}

std::vector<Order> OrderBook::getBuyOrders() const {
    std::vector<Order> orders;
    for (const auto& priceLevel : buyOrders_) {
        for (const auto& order : priceLevel.second) {
            orders.push_back(order);
        }
    }
    return orders;
}

std::vector<Order> OrderBook::getSellOrders() const {
    std::vector<Order> orders;
    for (const auto& priceLevel : sellOrders_) {
        for (const auto& order : priceLevel.second) {
            orders.push_back(order);
        }
    }
    return orders;
}

Order* OrderBook::getOrder(const std::string& orderId) {
    auto it = orderIndex_.find(orderId);
    if (it == orderIndex_.end()) {
        return nullptr;
    }
    
    double price = it->second.first;
    size_t position = it->second.second;
    
    // Try buy orders
    auto buyIt = buyOrders_.find(price);
    if (buyIt != buyOrders_.end() && position < buyIt->second.size()) {
        if (buyIt->second[position].orderId == orderId) {
            return &buyIt->second[position];
        }
    }
    
    // Try sell orders
    auto sellIt = sellOrders_.find(price);
    if (sellIt != sellOrders_.end() && position < sellIt->second.size()) {
        if (sellIt->second[position].orderId == orderId) {
            return &sellIt->second[position];
        }
    }
    
    return nullptr;
}

void OrderBook::indexOrder(const std::string& orderId, double price, size_t position, OrderSide side) {
    orderIndex_[orderId] = {price, position};
}

void OrderBook::removeFromIndex(const std::string& orderId) {
    orderIndex_.erase(orderId);
}

