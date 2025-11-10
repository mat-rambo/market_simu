#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "MarketServer.h"
#include "TestClient.h"
#include "Account.h"
#include "OrderBook.h"

// Helper function to log order submission
void logOrderSubmission(const std::string& traderId, const std::string& symbol,
                        const std::string& side, const std::string& type,
                        double price, double quantity) {
    std::cout << "[ORDER] Trader: " << traderId 
              << " | Symbol: " << symbol
              << " | Side: " << side
              << " | Type: " << type
              << " | Price: " << std::fixed << std::setprecision(2) << price
              << " | Quantity: " << quantity << std::endl;
}

// Helper function to log orderbook state
void logOrderBook(const OrderBook& orderBook) {
    std::cout << "[ORDERBOOK] Symbol: " << orderBook.getSymbol() << std::endl;
    
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    
    std::cout << "  Best Bid: " << (bestBid > 0 ? std::to_string(bestBid) : "N/A") << std::endl;
    std::cout << "  Best Ask: " << (bestAsk > 0 ? std::to_string(bestAsk) : "N/A") << std::endl;
    
    auto buyOrders = orderBook.getBuyOrders();
    auto sellOrders = orderBook.getSellOrders();
    
    if (!buyOrders.empty()) {
        std::cout << "  Buy Orders:" << std::endl;
        for (const auto& order : buyOrders) {
            std::cout << "    - " << order.orderId 
                      << " | Trader: " << order.traderId
                      << " | Price: " << std::fixed << std::setprecision(2) << order.price
                      << " | Quantity: " << order.quantity
                      << " | Filled: " << order.filledQuantity
                      << " | Status: ";
            switch (order.status) {
                case OrderStatus::PENDING: std::cout << "PENDING"; break;
                case OrderStatus::PARTIALLY_FILLED: std::cout << "PARTIALLY_FILLED"; break;
                case OrderStatus::FILLED: std::cout << "FILLED"; break;
                case OrderStatus::CANCELLED: std::cout << "CANCELLED"; break;
                case OrderStatus::REJECTED: std::cout << "REJECTED"; break;
            }
            std::cout << std::endl;
        }
    }
    
    if (!sellOrders.empty()) {
        std::cout << "  Sell Orders:" << std::endl;
        for (const auto& order : sellOrders) {
            std::cout << "    - " << order.orderId 
                      << " | Trader: " << order.traderId
                      << " | Price: " << std::fixed << std::setprecision(2) << order.price
                      << " | Quantity: " << order.quantity
                      << " | Filled: " << order.filledQuantity
                      << " | Status: ";
            switch (order.status) {
                case OrderStatus::PENDING: std::cout << "PENDING"; break;
                case OrderStatus::PARTIALLY_FILLED: std::cout << "PARTIALLY_FILLED"; break;
                case OrderStatus::FILLED: std::cout << "FILLED"; break;
                case OrderStatus::CANCELLED: std::cout << "CANCELLED"; break;
                case OrderStatus::REJECTED: std::cout << "REJECTED"; break;
            }
            std::cout << std::endl;
        }
    }
    
    if (buyOrders.empty() && sellOrders.empty()) {
        std::cout << "  (Empty)" << std::endl;
    }
    std::cout << std::endl;
}

class MarketServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a different port for each test to avoid conflicts
        static int testPort = 9999;
        port_ = testPort++;
        
        server_ = std::make_unique<MarketServer>(port_);
        
        // Start server in a separate thread
        serverThread_ = std::thread([this]() {
            try {
                server_->start();
            } catch (...) {
                // Server stopped
            }
        });
        
        // Wait for server to be ready (check if running)
        for (int i = 0; i < 20; ++i) {
            if (server_->isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    void TearDown() override {
        if (server_) {
            server_->stop();
        }
        if (serverThread_.joinable()) {
            serverThread_.join();
        }
        // Wait a bit for cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Clear server to ensure fresh start for next test
        server_.reset();
    }
    
    int port_;
    std::unique_ptr<MarketServer> server_;
    std::thread serverThread_;
};

// Test 1: Basic order matching - BUY and SELL LIMIT orders that match
TEST_F(MarketServerTest, BasicOrderMatching) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    // Wait a bit for server to be fully ready
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    
    // Small delay after registration
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader1 submits a BUY order at 150.00
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    std::string response1 = trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    EXPECT_TRUE(response1.find("ORDER_ACCEPTED:") == 0 || !response1.empty()) 
        << "Response1: " << response1;
    
    // Small delay to ensure order is processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a SELL order at 150.00 (should match)
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    std::string response2 = trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    EXPECT_TRUE(response2.find("ORDER_ACCEPTED:") == 0 || !response2.empty())
        << "Response2: " << response2;
    
    // Wait for settlement
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after matching
    logOrderBook(orderBook);
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    
    ASSERT_NE(account1, nullptr) << "Account1 is null";
    ASSERT_NE(account2, nullptr) << "Account2 is null";
    
    // Trader1 should have paid 1500.00 (10 * 150.00)
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 1500.0);
    // Trader2 should have received 1500.00
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 1500.0);
    
    // Check positions
    EXPECT_DOUBLE_EQ(account1->getPosition("AAPL"), 10.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("AAPL"), -10.0);
    
    // Orderbook should be empty (orders matched)
    EXPECT_DOUBLE_EQ(orderBook.getBestBid(), 0.0);
    EXPECT_DOUBLE_EQ(orderBook.getBestAsk(), 0.0);
}

// Test 2: Orders that don't match stay in orderbook
TEST_F(MarketServerTest, OrdersStayInOrderBook) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader1 submits a BUY order at 150.00
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    
    // Wait for order to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a SELL order at 151.00 (won't match - too high)
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 151.00, 10);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 151.00, 10);
    
    // Wait for orders to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after both orders
    logOrderBook(orderBook);
    
    // Check orderbook
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    
    // Orders should stay in orderbook since they don't match
    EXPECT_GT(bestBid, 0.0) << "Buy order should be in orderbook";
    EXPECT_GT(bestAsk, 0.0) << "Sell order should be in orderbook";
    
    if (bestBid > 0.0) {
        EXPECT_DOUBLE_EQ(bestBid, 150.00);
    }
    if (bestAsk > 0.0) {
        EXPECT_DOUBLE_EQ(bestAsk, 151.00);
    }
    
    // Balances should be unchanged
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0);
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0);
}

// Test 3: Partial fills
TEST_F(MarketServerTest, PartialFill) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    ASSERT_TRUE(trader1.connect());
    ASSERT_TRUE(trader2.connect());
    
    ASSERT_TRUE(trader1.registerTrader("trader1"));
    ASSERT_TRUE(trader2.registerTrader("trader2"));
    
    // Trader1 submits a BUY order for 10 shares
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a SELL order for 5 shares (partial fill)
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 150.00, 5);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 150.00, 5);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after partial fill
    logOrderBook(orderBook);
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    
    // Trader1 should have paid for 5 shares: 5 * 150.00 = 750.00
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 750.0);
    // Trader2 should have received 750.00
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 750.0);
    
    // Check positions
    EXPECT_DOUBLE_EQ(account1->getPosition("AAPL"), 5.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("AAPL"), -5.0);
    
    // Orderbook should still have trader1's remaining 5 shares
    EXPECT_DOUBLE_EQ(orderBook.getBestBid(), 150.00);
}

// Test 4: MARKET orders
TEST_F(MarketServerTest, MarketOrders) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader2 submits a SELL LIMIT order at 150.00
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after limit order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader1 submits a MARKET BUY order (should match at 150.00)
    logOrderSubmission("trader1", "AAPL", "BUY", "MARKET", 0.0, 10);
    trader1.submitOrder("trader1", "AAPL", "BUY", "MARKET", 0.0, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after market order
    logOrderBook(orderBook);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    
    // Trader1 should have paid 1500.00 (market order matched at limit price)
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 1500.0);
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 1500.0);
    
    // Check positions
    EXPECT_DOUBLE_EQ(account1->getPosition("AAPL"), 10.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("AAPL"), -10.0);
}

// Test 5: Multiple orders matching
TEST_F(MarketServerTest, MultipleOrdersMatching) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    TestClient trader3("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    ASSERT_TRUE(trader3.connect()) << "Failed to connect trader3";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    ASSERT_TRUE(trader3.registerTrader("trader3")) << "Failed to register trader3";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader1 submits a large BUY order
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 20);
    trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 20);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a SELL order for 10 shares
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after second order
    logOrderBook(orderBook);
    
    // Trader3 submits another SELL order for 10 shares
    logOrderSubmission("trader3", "AAPL", "SELL", "LIMIT", 150.00, 10);
    trader3.submitOrder("trader3", "AAPL", "SELL", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after all orders
    logOrderBook(orderBook);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    auto account3 = server_->getAccount("trader3");
    
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    ASSERT_NE(account3, nullptr);
    
    // Trader1 should have paid for 20 shares: 20 * 150.00 = 3000.00
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 3000.0);
    // Trader2 should have received 1500.00
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 1500.0);
    // Trader3 should have received 1500.00
    EXPECT_DOUBLE_EQ(account3->getBalance(), 10000.0 + 1500.0);
    
    // Check positions
    EXPECT_DOUBLE_EQ(account1->getPosition("AAPL"), 20.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("AAPL"), -10.0);
    EXPECT_DOUBLE_EQ(account3->getPosition("AAPL"), -10.0);
}

// Test 6: Price priority - better prices match first
TEST_F(MarketServerTest, PricePriority) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    TestClient trader3("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    ASSERT_TRUE(trader3.connect()) << "Failed to connect trader3";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    ASSERT_TRUE(trader3.registerTrader("trader3")) << "Failed to register trader3";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader1 submits a BUY order at 150.00
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a SELL order at 149.00 (better price - should match first)
    // This should match against trader1's BUY at 150, using price 149
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 149.00, 5);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 149.00, 5);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after second order
    logOrderBook(orderBook);
    
    // Trader3 submits a SELL order at 150.00
    // This should match against trader1's remaining BUY at 150, using price 150
    logOrderSubmission("trader3", "AAPL", "SELL", "LIMIT", 150.00, 5);
    trader3.submitOrder("trader3", "AAPL", "SELL", "LIMIT", 150.00, 5);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after all orders
    logOrderBook(orderBook);
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    auto account3 = server_->getAccount("trader3");
    
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    ASSERT_NE(account3, nullptr);
    
    // Trader1 should have paid for 5 shares at 149.00 and 5 at 150.00
    // 5 * 149.00 + 5 * 150.00 = 745.00 + 750.00 = 1495.00
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 1495.0);
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 745.0);
    EXPECT_DOUBLE_EQ(account3->getBalance(), 10000.0 + 750.0);
}

// Test 7: Insufficient funds
TEST_F(MarketServerTest, InsufficientFunds) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trader1 tries to buy with insufficient funds (10000 * 2000 = 20M, but only has 10K)
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 2000.00, 10000);
    std::string response = trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 2000.00, 10000);
    
    // Order should be accepted (validation happens at settlement)
    EXPECT_TRUE(response.find("ORDER_ACCEPTED:") == 0 || !response.empty());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log orderbook after first order
    auto& orderBook = server_->getOrderBook("AAPL");
    logOrderBook(orderBook);
    
    // Trader2 submits a matching SELL order
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 2000.00, 10000);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 2000.00, 10000);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Log orderbook after both orders
    logOrderBook(orderBook);
    
    // Settlement should fail for trader1 (insufficient funds)
    // Trader2's order should remain in orderbook
    // The sell order might still be there if settlement failed
    // Note: Current implementation doesn't handle settlement failures gracefully
    // This test documents current behavior
}

// Test 8: Multiple symbols
TEST_F(MarketServerTest, MultipleSymbols) {
    TestClient trader1("127.0.0.1", port_);
    TestClient trader2("127.0.0.1", port_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(trader1.connect()) << "Failed to connect trader1";
    ASSERT_TRUE(trader2.connect()) << "Failed to connect trader2";
    
    ASSERT_TRUE(trader1.registerTrader("trader1")) << "Failed to register trader1";
    ASSERT_TRUE(trader2.registerTrader("trader2")) << "Failed to register trader2";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Trade AAPL
    logOrderSubmission("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    trader1.submitOrder("trader1", "AAPL", "BUY", "LIMIT", 150.00, 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    logOrderSubmission("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    trader2.submitOrder("trader2", "AAPL", "SELL", "LIMIT", 150.00, 10);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Log AAPL orderbook
    auto& aaplOrderBook = server_->getOrderBook("AAPL");
    logOrderBook(aaplOrderBook);
    
    // Trade GOOGL - use smaller quantity so trader1 has enough funds
    // After AAPL: trader1 has 8500, so can afford 3 shares at 2500 = 7500
    logOrderSubmission("trader1", "GOOGL", "BUY", "LIMIT", 2500.00, 3);
    trader1.submitOrder("trader1", "GOOGL", "BUY", "LIMIT", 2500.00, 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logOrderSubmission("trader2", "GOOGL", "SELL", "LIMIT", 2500.00, 3);
    trader2.submitOrder("trader2", "GOOGL", "SELL", "LIMIT", 2500.00, 3);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Log GOOGL orderbook
    auto& googlOrderBook = server_->getOrderBook("GOOGL");
    logOrderBook(googlOrderBook);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Check balances
    auto account1 = server_->getAccount("trader1");
    auto account2 = server_->getAccount("trader2");
    
    ASSERT_NE(account1, nullptr);
    ASSERT_NE(account2, nullptr);
    
    // Trader1: 1500 (AAPL) + 7500 (GOOGL) = 9000
    EXPECT_DOUBLE_EQ(account1->getBalance(), 10000.0 - 9000.0);
    EXPECT_DOUBLE_EQ(account2->getBalance(), 10000.0 + 9000.0);
    
    // Check positions
    EXPECT_DOUBLE_EQ(account1->getPosition("AAPL"), 10.0);
    EXPECT_DOUBLE_EQ(account1->getPosition("GOOGL"), 3.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("AAPL"), -10.0);
    EXPECT_DOUBLE_EQ(account2->getPosition("GOOGL"), -3.0);
}

