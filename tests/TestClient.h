#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class TestClient {
public:
    TestClient(const std::string& host, int port);
    ~TestClient();
    
    bool connect();
    void disconnect();
    
    // Send a message and wait for response
    std::string sendMessage(const std::string& message);
    
    // Register as a trader
    bool registerTrader(const std::string& traderId);
    
    // Submit an order
    std::string submitOrder(const std::string& traderId, const std::string& symbol,
                           const std::string& side, const std::string& type,
                           double price, double quantity);
    
    // Receive a message (non-blocking, returns empty if no message)
    std::string receiveMessage();
    
    bool isConnected() const { return socket_ >= 0; }
    
private:
    std::string host_;
    int port_;
    int socket_;
};

#endif // TEST_CLIENT_H

