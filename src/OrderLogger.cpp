#include "OrderLogger.h"
#include <postgresql/libpq-fe.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>

OrderLogger::OrderLogger(const std::string& connectionString)
    : connectionString_(connectionString), conn_(nullptr) {
}

OrderLogger::~OrderLogger() {
    close();
}

bool OrderLogger::initialize() {
    PGconn* conn = PQconnectdb(connectionString_.c_str());
    
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return false;
    }
    
    conn_ = conn;
    
    // Create orders table
    std::string createOrdersTable = R"(
        CREATE TABLE IF NOT EXISTS orders (
            order_id VARCHAR(255) PRIMARY KEY,
            trader_id VARCHAR(255) NOT NULL,
            symbol VARCHAR(50) NOT NULL,
            side VARCHAR(10) NOT NULL,
            type VARCHAR(10) NOT NULL,
            price DECIMAL(20, 2) NOT NULL,
            quantity DECIMAL(20, 2) NOT NULL,
            filled_quantity DECIMAL(20, 2) DEFAULT 0,
            status VARCHAR(20) NOT NULL,
            timestamp BIGINT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!executeQuery(createOrdersTable)) {
        return false;
    }
    
    // Create trades table
    std::string createTradesTable = R"(
        CREATE TABLE IF NOT EXISTS trades (
            trade_id VARCHAR(255) PRIMARY KEY,
            order_id_buy VARCHAR(255) NOT NULL,
            order_id_sell VARCHAR(255) NOT NULL,
            symbol VARCHAR(50) NOT NULL,
            buyer_id VARCHAR(255) NOT NULL,
            seller_id VARCHAR(255) NOT NULL,
            price DECIMAL(20, 2) NOT NULL,
            quantity DECIMAL(20, 2) NOT NULL,
            timestamp BIGINT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!executeQuery(createTradesTable)) {
        return false;
    }
    
    // Create indexes for better query performance
    executeQuery("CREATE INDEX IF NOT EXISTS idx_orders_trader ON orders(trader_id)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_orders_symbol ON orders(symbol)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_orders_timestamp ON orders(timestamp)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_trades_symbol ON trades(symbol)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_trades_timestamp ON trades(timestamp)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_trades_buyer ON trades(buyer_id)");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_trades_seller ON trades(seller_id)");
    
    std::cout << "Order logger initialized: Connected to PostgreSQL database" << std::endl;
    return true;
}

std::string OrderLogger::escapeString(const std::string& str) {
    if (!conn_) {
        return str;
    }
    
    PGconn* conn = static_cast<PGconn*>(conn_);
    char* escaped = new char[str.length() * 2 + 1];
    int error;
    PQescapeStringConn(conn, escaped, str.c_str(), str.length(), &error);
    
    if (error) {
        delete[] escaped;
        return str;
    }
    
    std::string result(escaped);
    delete[] escaped;
    return result;
}

bool OrderLogger::logOrder(const Order& order) {
    if (!conn_) {
        return false;
    }
    
    PGconn* conn = static_cast<PGconn*>(conn_);
    
    // Convert timestamp to Unix timestamp
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        order.timestamp.time_since_epoch()
    ).count();
    
    // Convert status to string
    std::string statusStr;
    switch (order.status) {
        case OrderStatus::PENDING:
            statusStr = "PENDING";
            break;
        case OrderStatus::PARTIALLY_FILLED:
            statusStr = "PARTIALLY_FILLED";
            break;
        case OrderStatus::FILLED:
            statusStr = "FILLED";
            break;
        case OrderStatus::REJECTED:
            statusStr = "REJECTED";
            break;
        default:
            statusStr = "UNKNOWN";
    }
    
    std::string sideStr = (order.side == OrderSide::BUY) ? "BUY" : "SELL";
    std::string typeStr = (order.type == OrderType::MARKET) ? "MARKET" : "LIMIT";
    
    // Use parameterized query for safety
    std::ostringstream query;
    query << "INSERT INTO orders (order_id, trader_id, symbol, side, type, "
          << "price, quantity, filled_quantity, status, timestamp) VALUES ("
          << "$1, $2, $3, $4, $5, $6, $7, $8, $9, $10) "
          << "ON CONFLICT (order_id) DO UPDATE SET "
          << "filled_quantity = EXCLUDED.filled_quantity, "
          << "status = EXCLUDED.status";
    
    const char* paramValues[10] = {
        order.orderId.c_str(),
        order.traderId.c_str(),
        order.symbol.c_str(),
        sideStr.c_str(),
        typeStr.c_str(),
        nullptr, // price
        nullptr, // quantity
        nullptr, // filled_quantity
        statusStr.c_str(),
        nullptr  // timestamp
    };
    
    // Convert numbers to strings for parameters
    std::string priceStr = std::to_string(order.price);
    std::string quantityStr = std::to_string(order.quantity);
    std::string filledStr = std::to_string(order.filledQuantity);
    std::string timestampStr = std::to_string(timestamp);
    
    paramValues[5] = priceStr.c_str();
    paramValues[6] = quantityStr.c_str();
    paramValues[7] = filledStr.c_str();
    paramValues[9] = timestampStr.c_str();
    
    int paramLengths[10] = {
        static_cast<int>(order.orderId.length()),
        static_cast<int>(order.traderId.length()),
        static_cast<int>(order.symbol.length()),
        static_cast<int>(sideStr.length()),
        static_cast<int>(typeStr.length()),
        static_cast<int>(priceStr.length()),
        static_cast<int>(quantityStr.length()),
        static_cast<int>(filledStr.length()),
        static_cast<int>(statusStr.length()),
        static_cast<int>(timestampStr.length())
    };
    
    int paramFormats[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // All text format
    
    PGresult* res = PQexecParams(conn, query.str().c_str(), 10, nullptr, paramValues, 
                                  paramLengths, paramFormats, 0);
    
    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!success) {
        std::cerr << "Failed to log order: " << PQerrorMessage(conn) << std::endl;
    }
    
    PQclear(res);
    return success;
}

bool OrderLogger::logTrade(const Trade& trade) {
    if (!conn_) {
        return false;
    }
    
    PGconn* conn = static_cast<PGconn*>(conn_);
    
    // Convert timestamp to Unix timestamp
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        trade.timestamp.time_since_epoch()
    ).count();
    
    // Use parameterized query for safety
    std::ostringstream query;
    query << "INSERT INTO trades (trade_id, order_id_buy, order_id_sell, symbol, "
          << "buyer_id, seller_id, price, quantity, timestamp) VALUES ("
          << "$1, $2, $3, $4, $5, $6, $7, $8, $9)";
    
    const char* paramValues[9] = {
        trade.tradeId.c_str(),
        trade.buyOrderId.c_str(),
        trade.sellOrderId.c_str(),
        trade.symbol.c_str(),
        trade.buyTraderId.c_str(),
        trade.sellTraderId.c_str(),
        nullptr, // price
        nullptr, // quantity
        nullptr  // timestamp
    };
    
    // Convert numbers to strings for parameters
    std::string priceStr = std::to_string(trade.price);
    std::string quantityStr = std::to_string(trade.quantity);
    std::string timestampStr = std::to_string(timestamp);
    
    paramValues[6] = priceStr.c_str();
    paramValues[7] = quantityStr.c_str();
    paramValues[8] = timestampStr.c_str();
    
    int paramLengths[9] = {
        static_cast<int>(trade.tradeId.length()),
        static_cast<int>(trade.buyOrderId.length()),
        static_cast<int>(trade.sellOrderId.length()),
        static_cast<int>(trade.symbol.length()),
        static_cast<int>(trade.buyTraderId.length()),
        static_cast<int>(trade.sellTraderId.length()),
        static_cast<int>(priceStr.length()),
        static_cast<int>(quantityStr.length()),
        static_cast<int>(timestampStr.length())
    };
    
    int paramFormats[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // All text format
    
    PGresult* res = PQexecParams(conn, query.str().c_str(), 9, nullptr, paramValues, 
                                  paramLengths, paramFormats, 0);
    
    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!success) {
        std::cerr << "Failed to log trade: " << PQerrorMessage(conn) << std::endl;
    }
    
    PQclear(res);
    return success;
}

bool OrderLogger::executeQuery(const std::string& query) {
    if (!conn_) {
        return false;
    }
    
    PGconn* conn = static_cast<PGconn*>(conn_);
    PGresult* res = PQexec(conn, query.c_str());
    
    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK || 
                    PQresultStatus(res) == PGRES_TUPLES_OK);
    
    if (!success) {
        // Ignore "already exists" errors for CREATE TABLE IF NOT EXISTS
        std::string error = PQerrorMessage(conn);
        if (error.find("already exists") == std::string::npos) {
            std::cerr << "SQL error: " << error << std::endl;
        }
    }
    
    PQclear(res);
    return success;
}

void OrderLogger::close() {
    if (conn_) {
        PGconn* conn = static_cast<PGconn*>(conn_);
        PQfinish(conn);
        conn_ = nullptr;
    }
}
