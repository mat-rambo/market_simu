#ifndef TRADER_H
#define TRADER_H

#include <string>
#include <memory>
#include "Account.h"

class Trader {
public:
    Trader(const std::string& traderId, const std::string& accountId);
    
    const std::string& getTraderId() const { return traderId_; }
    const std::string& getAccountId() const { return accountId_; }
    std::shared_ptr<Account> getAccount() const { return account_; }
    
    void setAccount(std::shared_ptr<Account> account) { account_ = account; }
    
private:
    std::string traderId_;
    std::string accountId_;
    std::shared_ptr<Account> account_;
};

#endif // TRADER_H

