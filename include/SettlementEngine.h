#ifndef SETTLEMENT_ENGINE_H
#define SETTLEMENT_ENGINE_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include "Trade.h"
#include "Account.h"

class SettlementEngine {
public:
    using SettlementCallback = std::function<void(const std::string& traderId, 
                                                   const std::string& symbol,
                                                   double quantity, 
                                                   double price)>;
    
    SettlementEngine();
    
    // Settle a trade
    void settleTrade(const Trade& trade, 
                     std::shared_ptr<Account> buyAccount,
                     std::shared_ptr<Account> sellAccount);
    
    // Settle multiple trades
    void settleTrades(const std::vector<Trade>& trades,
                     const std::map<std::string, std::shared_ptr<Account>>& accounts);
    
    // Set callback for settlement notifications
    void setSettlementCallback(SettlementCallback callback) { 
        settlementCallback_ = callback; 
    }
    
private:
    SettlementCallback settlementCallback_;
};

#endif // SETTLEMENT_ENGINE_H

