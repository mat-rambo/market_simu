#include "SettlementEngine.h"
#include <iostream>
#include <stdexcept>

SettlementEngine::SettlementEngine() {
}

void SettlementEngine::settleTrade(const Trade& trade, 
                                   std::shared_ptr<Account> buyAccount,
                                   std::shared_ptr<Account> sellAccount) {
    if (!buyAccount || !sellAccount) {
        throw std::invalid_argument("Accounts cannot be null");
    }
    
    double totalCost = trade.price * trade.quantity;
    
    // Buyer pays money and receives shares
    if (!buyAccount->withdraw(totalCost)) {
        // Insufficient funds - don't settle, but don't throw
        // The trade should not have been executed if funds were insufficient
        // This is a safety check
        std::cerr << "Warning: Buyer account has insufficient funds for trade " 
                  << trade.tradeId << std::endl;
        return;
    }
    buyAccount->updatePosition(trade.symbol, trade.quantity);
    
    // Seller receives money and gives shares
    sellAccount->deposit(totalCost);
    sellAccount->updatePosition(trade.symbol, -trade.quantity);
    
    // Notify about settlement
    if (settlementCallback_) {
        settlementCallback_(trade.buyTraderId, trade.symbol, trade.quantity, trade.price);
        settlementCallback_(trade.sellTraderId, trade.symbol, -trade.quantity, trade.price);
    }
}

void SettlementEngine::settleTrades(const std::vector<Trade>& trades,
                                     const std::map<std::string, std::shared_ptr<Account>>& accounts) {
    for (const auto& trade : trades) {
        auto buyAccountIt = accounts.find(trade.buyTraderId);
        auto sellAccountIt = accounts.find(trade.sellTraderId);
        
        if (buyAccountIt == accounts.end() || sellAccountIt == accounts.end()) {
            continue; // Skip if accounts not found
        }
        
        settleTrade(trade, buyAccountIt->second, sellAccountIt->second);
    }
}

