#ifndef ORDER_LOGGER_H
#define ORDER_LOGGER_H

#include <string>
#include <memory>
#include <mutex>
#include "Trade.h"

class OrderLogger {
public:
    // PostgreSQL connection string format: "host=localhost port=5432 dbname=market user=postgres password=postgres"
    OrderLogger(const std::string& connectionString = "host=localhost port=5432 dbname=market user=postgres password=postgres");
    ~OrderLogger();
    
    // Initialize database (create tables)
    bool initialize();
    
    // Log an order submission
    bool logOrder(const Order& order);
    
    // Log a trade execution
    bool logTrade(const Trade& trade);
    
    // Close database connection
    void close();

private:
    std::string connectionString_;
    void* conn_; // PGconn* (using void* to avoid including libpq-fe.h in header)
    mutable std::mutex connMutex_; // Protect connection from concurrent access
    
    bool executeQuery(const std::string& query);
    std::string escapeString(const std::string& str);
};

#endif // ORDER_LOGGER_H

