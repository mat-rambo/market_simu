#include "Account.h"
#include <stdexcept>

Account::Account(const std::string& accountId, double initialBalance)
    : accountId_(accountId), balance_(initialBalance) {
}

double Account::getPosition(const std::string& symbol) const {
    auto it = positions_.find(symbol);
    if (it != positions_.end()) {
        return it->second;
    }
    return 0.0;
}

void Account::deposit(double amount) {
    if (amount < 0) {
        throw std::invalid_argument("Deposit amount must be positive");
    }
    balance_ += amount;
}

bool Account::withdraw(double amount) {
    if (amount < 0) {
        throw std::invalid_argument("Withdraw amount must be positive");
    }
    if (balance_ >= amount) {
        balance_ -= amount;
        return true;
    }
    return false;
}

void Account::updatePosition(const std::string& symbol, double quantity) {
    positions_[symbol] += quantity;
    if (positions_[symbol] == 0.0) {
        positions_.erase(symbol);
    }
}

