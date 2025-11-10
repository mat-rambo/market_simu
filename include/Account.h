#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <map>

class Account {
public:
    Account(const std::string& accountId, double initialBalance = 0.0);
    
    const std::string& getAccountId() const { return accountId_; }
    double getBalance() const { return balance_; }
    double getPosition(const std::string& symbol) const;
    
    void deposit(double amount);
    bool withdraw(double amount);
    void updatePosition(const std::string& symbol, double quantity);
    
private:
    std::string accountId_;
    double balance_;
    std::map<std::string, double> positions_; // symbol -> quantity
};

#endif // ACCOUNT_H

