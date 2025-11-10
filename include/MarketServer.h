#ifndef MARKET_SERVER_H
#define MARKET_SERVER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include "OrderBook.h"
#include "MatchingEngine.h"
#include "SettlementEngine.h"
#include "Trader.h"
#include "Account.h"
#include "Trade.h"
#include "OrderLogger.h"

class MarketServer {
public:
    MarketServer(int port = 8888);
    ~MarketServer();
    
    // Start the server
    void start();
    
    // Stop the server
    void stop();
    
    // Register a trader with an account
    void registerTrader(std::shared_ptr<Trader> trader, std::shared_ptr<Account> account);
    
    // Get or create order book for a symbol
    OrderBook& getOrderBook(const std::string& symbol);
    
    // Submit an order
    bool submitOrder(const Order& order);
    
    // Get trader by ID
    std::shared_ptr<Trader> getTrader(const std::string& traderId);
    
    // Get account by ID
    std::shared_ptr<Account> getAccount(const std::string& accountId);
    
    // Check if server is running
    bool isRunning() const { return running_; }
    
private:
    int port_;
    int serverSocket_;
    std::atomic<bool> running_;
    
    std::map<std::string, std::shared_ptr<OrderBook>> orderBooks_;
    std::map<std::string, std::shared_ptr<Trader>> traders_;
    std::map<std::string, std::shared_ptr<Account>> accounts_;
    std::map<std::string, int> traderSockets_; // traderId -> socket
    
    MatchingEngine matchingEngine_;
    SettlementEngine settlementEngine_;
    OrderLogger orderLogger_;
    
    mutable std::mutex orderBooksMutex_;
    std::mutex tradersMutex_;
    std::mutex accountsMutex_;
    mutable std::mutex socketsMutex_;
    
    std::thread acceptThread_; // Thread for accepting connections
    
    // Socket handling
    void acceptConnections();
    void handleClient(int clientSocket);
    void processMessage(int clientSocket, const std::string& message);
    
    // Message parsing
    Order parseOrderMessage(const std::string& message, const std::string& traderId);
    std::string createResponseMessage(const std::string& status, const std::string& data);
    
    // Trade callback
    void onTradeExecuted(const Trade& trade);
    
    // Settlement callback
    void onSettlementComplete(const std::string& traderId, 
                             const std::string& symbol,
                             double quantity, 
                             double price);
    
    // Get all orderbook symbols (for web interface)
    std::vector<std::string> getOrderBookSymbols() const;
    
    // Get number of connected traders (for web interface)
    int getConnectedTradersCount() const;
    
    // Get number of traders with active orders (for web interface)
    int getTradersWithActiveOrdersCount() const;
    
    // Friend class for web server access
    friend class WebServer;
};

#endif // MARKET_SERVER_H

