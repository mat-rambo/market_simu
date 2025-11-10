#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

class MarketServer;

class WebServer {
public:
    WebServer(int port, MarketServer* marketServer);
    ~WebServer();
    
    void start();
    void stop();
    
    bool isRunning() const { return running_; }
    
private:
    int port_;
    int serverSocket_;
    std::atomic<bool> running_;
    MarketServer* marketServer_;
    std::thread serverThread_;
    std::thread acceptThread_;
    
    std::map<int, std::thread> clientThreads_;
    std::mutex clientsMutex_;
    
    void acceptConnections();
    void handleClient(int clientSocket);
    void handleHttpRequest(int clientSocket, const std::string& request);
    void handleWebSocket(int clientSocket, const std::string& request);
    void sendHttpResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& body);
    void sendWebSocketMessage(int clientSocket, const std::string& message);
    
    std::string getOrderBookJson(const std::string& symbol);
    std::string getAllOrderBooksJson();
    std::string getAccountJson(const std::string& accountId);
    std::string getAllAccountsJson();
    std::string getStatsJson();
    
    void broadcastOrderBookUpdate(const std::string& symbol);
    
    std::vector<int> websocketClients_;
    std::mutex websocketClientsMutex_;
};

#endif // WEB_SERVER_H

