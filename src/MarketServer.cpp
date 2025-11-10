#include "MarketServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <errno.h>

MarketServer::MarketServer(int port) 
    : port_(port), serverSocket_(-1), running_(false), 
      orderLogger_("") {  // Empty string will use environment variables
    // Initialize order logger
    if (!orderLogger_.initialize()) {
        std::cerr << "Warning: Failed to initialize order logger" << std::endl;
    }
    
    matchingEngine_.setTradeCallback(
        [this](const Trade& trade) { 
            this->onTradeExecuted(trade);
            // Log trade to database
            orderLogger_.logTrade(trade);
        }
    );
    
    settlementEngine_.setSettlementCallback(
        [this](const std::string& traderId, const std::string& symbol,
               double quantity, double price) {
            this->onSettlementComplete(traderId, symbol, quantity, price);
        }
    );
}

MarketServer::~MarketServer() {
    try {
        stop();
    } catch (...) {
        // Ignore exceptions during stop
    }
    try {
        orderLogger_.close();
    } catch (...) {
        // Ignore exceptions during close
    }
}

void MarketServer::start() {
    // Create socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to set socket options");
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(serverSocket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(serverSocket_);
        if (errno == EADDRINUSE) {
            std::ostringstream errorMsg;
            errorMsg << "❌ ERROR: Port " << port_ << " is already in use!\n"
                     << "   Another server instance is already running on this port.\n"
                     << "   Please stop the existing server or use a different port.\n"
                     << "   To find and kill the process using this port, run:\n"
                     << "   sudo lsof -i :" << port_ << " | grep LISTEN\n"
                     << "   sudo kill -9 <PID>";
            throw std::runtime_error(errorMsg.str());
        } else {
            std::ostringstream errorMsg;
            errorMsg << "Failed to bind socket on port " << port_ 
                     << ": " << strerror(errno) << " (errno: " << errno << ")";
            throw std::runtime_error(errorMsg.str());
        }
    }
    
    // Listen
    if (listen(serverSocket_, 10) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    running_ = true;
    std::cout << "Market server started on port " << port_ << std::endl;
    
    // Start accepting connections in a separate thread
    acceptThread_ = std::thread(&MarketServer::acceptConnections, this);
}

void MarketServer::stop() {
    if (running_) {
        running_ = false;
        if (serverSocket_ >= 0) {
            // Shutdown socket to wake up accept() call
            shutdown(serverSocket_, SHUT_RDWR);
            close(serverSocket_);
            serverSocket_ = -1;
        }
        if (acceptThread_.joinable()) {
            acceptThread_.join();
        }
        std::cout << "Market server stopped" << std::endl;
    }
}

void MarketServer::acceptConnections() {
    while (running_) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);
        
        int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
            if (running_) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }
        
        std::cout << "New client connected from " 
                  << inet_ntoa(clientAddress.sin_addr) << std::endl;
        
        // Handle client in a separate thread
        std::thread clientThread(&MarketServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void MarketServer::handleClient(int clientSocket) {
    char buffer[4096];
    std::string traderId;
    
    // First message should be trader registration
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string message(buffer);
    
    // Parse registration: "REGISTER:traderId"
    if (message.substr(0, 9) == "REGISTER:") {
        traderId = message.substr(9);
        // Remove newline if present
        traderId.erase(std::remove(traderId.begin(), traderId.end(), '\n'), traderId.end());
        traderId.erase(std::remove(traderId.begin(), traderId.end(), '\r'), traderId.end());
        
        // Create trader and account if they don't exist
        {
            std::lock_guard<std::mutex> lock(tradersMutex_);
            if (traders_.find(traderId) == traders_.end()) {
                auto account = std::make_shared<Account>(traderId, 10000.0); // Initial balance
                auto trader = std::make_shared<Trader>(traderId, traderId);
                trader->setAccount(account);
                
                accounts_[traderId] = account;
                traders_[traderId] = trader;
            }
        }
        
        std::string response = "REGISTERED:" + traderId + "\n";
        send(clientSocket, response.c_str(), response.length(), 0);
        
        // Register the socket for this trader
        {
            std::lock_guard<std::mutex> lock(socketsMutex_);
            traderSockets_[traderId] = clientSocket;
        }
    } else {
        close(clientSocket);
        return;
    }
    
    // Process messages
    while (running_) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        
        buffer[bytesRead] = '\0';
        message = std::string(buffer);
        
        processMessage(clientSocket, message);
    }
    
    // Unregister the socket when trader disconnects
    if (!traderId.empty()) {
        std::lock_guard<std::mutex> lock(socketsMutex_);
        traderSockets_.erase(traderId);
    }
    
    close(clientSocket);
    std::cout << "Client " << traderId << " disconnected" << std::endl;
}

void MarketServer::processMessage(int clientSocket, const std::string& message) {
    // Format: "ORDER:traderId:symbol:side:type:price:quantity"
    
    if (message.substr(0, 6) == "ORDER:") {
        std::istringstream iss(message.substr(6));
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, ':')) {
            // Remove newline if present
            token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
            token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        if (tokens.size() >= 6) {
            std::string traderId = tokens[0];
            Order order = parseOrderMessage(message, traderId);
            
            if (submitOrder(order)) {
                std::string response = "ORDER_ACCEPTED:" + order.orderId + "\n";
                send(clientSocket, response.c_str(), response.length(), 0);
            } else {
                std::string response = "ORDER_REJECTED:" + order.orderId + ":Invalid order\n";
                send(clientSocket, response.c_str(), response.length(), 0);
            }
        } else {
            std::string response = "ERROR:Invalid order format. Expected: ORDER:traderId:symbol:side:type:price:quantity\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
    }
}

Order MarketServer::parseOrderMessage(const std::string& message, const std::string& traderId) {
    Order order;
    order.traderId = traderId;
    order.orderId = "ORD_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    order.timestamp = std::chrono::system_clock::now();
    order.filledQuantity = 0.0;
    order.status = OrderStatus::PENDING;
    
    // Format: "ORDER:traderId:symbol:side:type:price:quantity"
    std::istringstream iss(message.substr(6));
    std::string token;
    std::vector<std::string> tokens;
    
    while (std::getline(iss, token, ':')) {
        // Remove newline if present
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    if (tokens.size() >= 6) {
        order.symbol = tokens[1];
        order.side = (tokens[2] == "BUY") ? OrderSide::BUY : OrderSide::SELL;
        order.type = (tokens[3] == "MARKET") ? OrderType::MARKET : OrderType::LIMIT;
        try {
            order.price = std::stod(tokens[4]);
            order.quantity = std::stod(tokens[5]);
        } catch (const std::exception& e) {
            order.status = OrderStatus::REJECTED;
        }
    } else {
        order.status = OrderStatus::REJECTED;
    }
    
    return order;
}

bool MarketServer::submitOrder(const Order& order) {
    // Validate trader exists
    {
        std::lock_guard<std::mutex> lock(tradersMutex_);
        if (traders_.find(order.traderId) == traders_.end()) {
            return false;
        }
    }
    
    // Log order to database
    orderLogger_.logOrder(order);
    
    // Get or create order book
    OrderBook* orderBook;
    {
        std::lock_guard<std::mutex> lock(orderBooksMutex_);
        if (orderBooks_.find(order.symbol) == orderBooks_.end()) {
            orderBooks_[order.symbol] = std::make_shared<OrderBook>(order.symbol);
        }
        orderBook = orderBooks_[order.symbol].get();
    }
    
    // Create a mutable copy for matching
    Order mutableOrder = order;
    
    // Match the order
    std::vector<Trade> trades = matchingEngine_.submitOrder(mutableOrder, *orderBook);
    
    // Settle trades
    if (!trades.empty()) {
        std::map<std::string, std::shared_ptr<Account>> accounts;
        {
            std::lock_guard<std::mutex> lock(accountsMutex_);
            for (const auto& trade : trades) {
                if (accounts.find(trade.buyTraderId) == accounts.end()) {
                    auto trader = getTrader(trade.buyTraderId);
                    if (trader) {
                        accounts[trade.buyTraderId] = trader->getAccount();
                    }
                }
                if (accounts.find(trade.sellTraderId) == accounts.end()) {
                    auto trader = getTrader(trade.sellTraderId);
                    if (trader) {
                        accounts[trade.sellTraderId] = trader->getAccount();
                    }
                }
            }
        }
        
        settlementEngine_.settleTrades(trades, accounts);
    }
    
    return true;
}

void MarketServer::registerTrader(std::shared_ptr<Trader> trader, std::shared_ptr<Account> account) {
    std::lock_guard<std::mutex> lock(tradersMutex_);
    traders_[trader->getTraderId()] = trader;
    
    std::lock_guard<std::mutex> accountLock(accountsMutex_);
    accounts_[account->getAccountId()] = account;
}

OrderBook& MarketServer::getOrderBook(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(orderBooksMutex_);
    if (orderBooks_.find(symbol) == orderBooks_.end()) {
        orderBooks_[symbol] = std::make_shared<OrderBook>(symbol);
    }
    return *orderBooks_[symbol];
}

std::shared_ptr<Trader> MarketServer::getTrader(const std::string& traderId) {
    std::lock_guard<std::mutex> lock(tradersMutex_);
    auto it = traders_.find(traderId);
    if (it != traders_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Account> MarketServer::getAccount(const std::string& accountId) {
    std::lock_guard<std::mutex> lock(accountsMutex_);
    auto it = accounts_.find(accountId);
    if (it != accounts_.end()) {
        return it->second;
    }
    return nullptr;
}

void MarketServer::onTradeExecuted(const Trade& trade) {
    std::cout << "Trade executed: " << trade.tradeId 
              << " | " << trade.symbol 
              << " | " << trade.quantity 
              << " @ " << trade.price 
              << " | Buyer: " << trade.buyTraderId 
              << " | Seller: " << trade.sellTraderId << std::endl;
    
    // Notify buyer
    {
        std::lock_guard<std::mutex> lock(socketsMutex_);
        auto buyIt = traderSockets_.find(trade.buyTraderId);
        if (buyIt != traderSockets_.end()) {
            std::ostringstream oss;
            oss << "TRADE_EXECUTED:" << trade.tradeId 
                << ":" << trade.symbol 
                << ":BUY:" << trade.quantity 
                << "@" << trade.price << "\n";
            std::string msg = oss.str();
            send(buyIt->second, msg.c_str(), msg.length(), 0);
        }
    }
    
    // Notify seller
    {
        std::lock_guard<std::mutex> lock(socketsMutex_);
        auto sellIt = traderSockets_.find(trade.sellTraderId);
        if (sellIt != traderSockets_.end()) {
            std::ostringstream oss;
            oss << "TRADE_EXECUTED:" << trade.tradeId 
                << ":" << trade.symbol 
                << ":SELL:" << trade.quantity 
                << "@" << trade.price << "\n";
            std::string msg = oss.str();
            send(sellIt->second, msg.c_str(), msg.length(), 0);
        }
    }
}

void MarketServer::onSettlementComplete(const std::string& traderId, 
                                        const std::string& symbol,
                                        double quantity, 
                                        double price) {
    std::cout << "Settlement: Trader " << traderId 
              << " | " << symbol 
              << " | " << quantity 
              << " @ " << price << std::endl;
    
    // Notify trader
    {
        std::lock_guard<std::mutex> lock(socketsMutex_);
        auto it = traderSockets_.find(traderId);
        if (it != traderSockets_.end()) {
            std::ostringstream oss;
            oss << "SETTLEMENT:" << symbol 
                << ":" << quantity 
                << "@" << price << "\n";
            std::string msg = oss.str();
            send(it->second, msg.c_str(), msg.length(), 0);
        }
    }
}

std::vector<std::string> MarketServer::getOrderBookSymbols() const {
    std::lock_guard<std::mutex> lock(orderBooksMutex_);
    std::vector<std::string> symbols;
    for (const auto& pair : orderBooks_) {
        symbols.push_back(pair.first);
    }
    return symbols;
}

int MarketServer::getConnectedTradersCount() const {
    std::lock_guard<std::mutex> lock(socketsMutex_);
    return static_cast<int>(traderSockets_.size());
}

int MarketServer::getTradersWithActiveOrdersCount() const {
    std::set<std::string> tradersWithOrders;
    
    // Check all orderbooks for unique trader IDs
    {
        std::lock_guard<std::mutex> lock(orderBooksMutex_);
        for (const auto& pair : orderBooks_) {
            auto buyOrders = pair.second->getBuyOrders();
            for (const auto& order : buyOrders) {
                if (order.status == OrderStatus::PENDING || order.status == OrderStatus::PARTIALLY_FILLED) {
                    tradersWithOrders.insert(order.traderId);
                }
            }
            
            auto sellOrders = pair.second->getSellOrders();
            for (const auto& order : sellOrders) {
                if (order.status == OrderStatus::PENDING || order.status == OrderStatus::PARTIALLY_FILLED) {
                    tradersWithOrders.insert(order.traderId);
                }
            }
        }
    }
    
    return static_cast<int>(tradersWithOrders.size());
}

