#include "WebServer.h"
#include "MarketServer.h"
#include "OrderBook.h"
#include "Account.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <errno.h>

WebServer::WebServer(int port, MarketServer* marketServer)
    : port_(port), serverSocket_(-1), running_(false), marketServer_(marketServer) {
}

WebServer::~WebServer() {
    stop();
}

void WebServer::start() {
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        throw std::runtime_error("Failed to create web server socket");
    }
    
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to set web server socket options");
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(serverSocket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(serverSocket_);
        if (errno == EADDRINUSE) {
            std::ostringstream errorMsg;
            errorMsg << "❌ ERROR: Web server port " << port_ << " is already in use!\n"
                     << "   Another server instance is already running on this port.\n"
                     << "   Please stop the existing server or use a different port.\n"
                     << "   To find and kill the process using this port, run:\n"
                     << "   sudo lsof -i :" << port_ << " | grep LISTEN\n"
                     << "   sudo kill -9 <PID>";
            throw std::runtime_error(errorMsg.str());
        } else {
            std::ostringstream errorMsg;
            errorMsg << "Failed to bind web server socket on port " << port_ 
                     << ": " << strerror(errno) << " (errno: " << errno << ")";
            throw std::runtime_error(errorMsg.str());
        }
    }
    
    if (listen(serverSocket_, 10) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to listen on web server socket");
    }
    
    running_ = true;
    std::cout << "Web server started on port " << port_ << std::endl;
    
    acceptThread_ = std::thread(&WebServer::acceptConnections, this);
}

void WebServer::stop() {
    if (running_) {
        running_ = false;
        if (serverSocket_ >= 0) {
            shutdown(serverSocket_, SHUT_RDWR);
            close(serverSocket_);
            serverSocket_ = -1;
        }
        if (acceptThread_.joinable()) {
            acceptThread_.join();
        }
        
        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            for (auto& pair : clientThreads_) {
                if (pair.second.joinable()) {
                    pair.second.join();
                }
            }
            clientThreads_.clear();
        }
        
        std::cout << "Web server stopped" << std::endl;
    }
}

void WebServer::acceptConnections() {
    while (running_) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);
        
        int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddress, &clientAddrLen);
        if (clientSocket < 0) {
            if (running_) {
                continue;
            }
            break;
        }
        
        std::thread clientThread(&WebServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void WebServer::handleClient(int clientSocket) {
    char buffer[8192];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    // Check if it's a WebSocket upgrade request
    if (request.find("Upgrade: websocket") != std::string::npos ||
        request.find("upgrade: websocket") != std::string::npos) {
        handleWebSocket(clientSocket, request);
    } else {
        handleHttpRequest(clientSocket, request);
    }
}

void WebServer::handleHttpRequest(int clientSocket, const std::string& request) {
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;
    
    // Remove query string
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }
    
    if (path == "/" || path == "/index.html") {
        // Serve the main HTML page
        // Try multiple possible paths
        std::ifstream file;
        std::vector<std::string> paths = {
            "web/index.html",
            "../web/index.html",
            "../../web/index.html",
            "/home/rambo/Documents/market_simu/web/index.html"
        };
        
        bool found = false;
        for (const auto& filePath : paths) {
            file.open(filePath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                sendHttpResponse(clientSocket, 200, "text/html", buffer.str());
                file.close();
                found = true;
                break;
            }
        }
        
        if (!found) {
            sendHttpResponse(clientSocket, 404, "text/plain", "File not found: web/index.html");
        }
    } else if (path == "/api/orderbooks") {
        // Get all orderbooks
        std::string json = getAllOrderBooksJson();
        sendHttpResponse(clientSocket, 200, "application/json", json);
    } else if (path.find("/api/orderbook/") == 0) {
        // Get specific orderbook
        std::string symbol = path.substr(15); // "/api/orderbook/".length()
        std::string json = getOrderBookJson(symbol);
        sendHttpResponse(clientSocket, 200, "application/json", json);
    } else if (path == "/api/accounts") {
        // Get all accounts
        std::string json = getAllAccountsJson();
        sendHttpResponse(clientSocket, 200, "application/json", json);
    } else if (path.find("/api/account/") == 0) {
        // Get specific account
        std::string accountId = path.substr(13); // "/api/account/".length()
        std::string json = getAccountJson(accountId);
        sendHttpResponse(clientSocket, 200, "application/json", json);
    } else if (path == "/api/stats") {
        // Get server statistics
        std::string json = getStatsJson();
        sendHttpResponse(clientSocket, 200, "application/json", json);
    } else {
        sendHttpResponse(clientSocket, 404, "text/plain", "Not found");
    }
    
    close(clientSocket);
}

void WebServer::handleWebSocket(int clientSocket, const std::string& request) {
    // Extract WebSocket key
    std::string key;
    size_t keyPos = request.find("Sec-WebSocket-Key: ");
    if (keyPos != std::string::npos) {
        size_t keyStart = keyPos + 19;
        size_t keyEnd = request.find("\r\n", keyStart);
        if (keyEnd != std::string::npos) {
            key = request.substr(keyStart, keyEnd - keyStart);
        }
    }
    
    // WebSocket accept key (simplified - in production use proper base64 encoding)
    std::string acceptKey = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // For simplicity, we'll use a basic response
    
    // Send WebSocket handshake response
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << acceptKey << "\r\n"
             << "\r\n";
    
    send(clientSocket, response.str().c_str(), response.str().length(), 0);
    
    // Add to WebSocket clients
    {
        std::lock_guard<std::mutex> lock(websocketClientsMutex_);
        websocketClients_.push_back(clientSocket);
    }
    
    // Keep connection alive and send updates
    // For now, just keep the connection open
    // In a full implementation, we'd handle WebSocket frames properly
}

void WebServer::sendHttpResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << body;
    
    send(clientSocket, response.str().c_str(), response.str().length(), 0);
}

void WebServer::sendWebSocketMessage(int clientSocket, const std::string& message) {
    // Simplified WebSocket frame (for production, use proper framing)
    // For now, we'll use a simple text message
    std::string frame = "\x81"; // Text frame, FIN bit set
    size_t len = message.length();
    if (len < 126) {
        frame += static_cast<char>(len);
    } else if (len < 65536) {
        frame += static_cast<char>(126);
        frame += static_cast<char>((len >> 8) & 0xFF);
        frame += static_cast<char>(len & 0xFF);
    }
    frame += message;
    
    send(clientSocket, frame.c_str(), frame.length(), 0);
}

void WebServer::broadcastOrderBookUpdate(const std::string& symbol) {
    // For now, WebSocket broadcasting is disabled
    // Clients will poll instead
    // In the future, this can be enabled for real-time updates
}

std::string WebServer::getOrderBookJson(const std::string& symbol) {
    if (!marketServer_) {
        return "{}";
    }
    
    try {
        auto& orderBook = marketServer_->getOrderBook(symbol);
        
        std::ostringstream json;
        json << "{\"symbol\":\"" << symbol << "\","
             << "\"bestBid\":" << orderBook.getBestBid() << ","
             << "\"bestAsk\":" << orderBook.getBestAsk() << ","
             << "\"buyOrders\":[";
        
        auto buyOrders = orderBook.getBuyOrders();
        bool first = true;
        for (const auto& order : buyOrders) {
            if (!first) json << ",";
            first = false;
            json << "{\"orderId\":\"" << order.orderId << "\","
                 << "\"traderId\":\"" << order.traderId << "\","
                 << "\"price\":" << order.price << ","
                 << "\"quantity\":" << order.quantity << ","
                 << "\"filledQuantity\":" << order.filledQuantity << ","
                 << "\"status\":\"" << (order.status == OrderStatus::PENDING ? "PENDING" :
                                         order.status == OrderStatus::PARTIALLY_FILLED ? "PARTIALLY_FILLED" :
                                         order.status == OrderStatus::FILLED ? "FILLED" : "UNKNOWN") << "\"}";
        }
        
        json << "],\"sellOrders\":[";
        
        auto sellOrders = orderBook.getSellOrders();
        first = true;
        for (const auto& order : sellOrders) {
            if (!first) json << ",";
            first = false;
            json << "{\"orderId\":\"" << order.orderId << "\","
                 << "\"traderId\":\"" << order.traderId << "\","
                 << "\"price\":" << order.price << ","
                 << "\"quantity\":" << order.quantity << ","
                 << "\"filledQuantity\":" << order.filledQuantity << ","
                 << "\"status\":\"" << (order.status == OrderStatus::PENDING ? "PENDING" :
                                         order.status == OrderStatus::PARTIALLY_FILLED ? "PARTIALLY_FILLED" :
                                         order.status == OrderStatus::FILLED ? "FILLED" : "UNKNOWN") << "\"}";
        }
        
        json << "]}";
        return json.str();
    } catch (...) {
        return "{}";
    }
}

std::string WebServer::getAllOrderBooksJson() {
    if (!marketServer_) {
        return "[]";
    }
    
    auto symbols = marketServer_->getOrderBookSymbols();
    std::ostringstream json;
    json << "[";
    
    bool first = true;
    for (const auto& symbol : symbols) {
        if (!first) json << ",";
        first = false;
        json << getOrderBookJson(symbol);
    }
    
    json << "]";
    return json.str();
}

std::string WebServer::getAccountJson(const std::string& accountId) {
    if (!marketServer_) {
        return "{}";
    }
    
    auto account = marketServer_->getAccount(accountId);
    if (!account) {
        return "{}";
    }
    
    std::ostringstream json;
    json << "{\"accountId\":\"" << accountId << "\","
         << "\"balance\":" << account->getBalance() << "}";
    
    return json.str();
}

std::string WebServer::getAllAccountsJson() {
    // This would require exposing accounts from MarketServer
    // For now, return empty array
    return "[]";
}


std::string WebServer::getStatsJson() {
    if (!marketServer_) {
        return "{\"connectedTraders\":0,\"tradersWithOrders\":0}";
    }
    int connectedTraders = marketServer_->getConnectedTradersCount();
    int tradersWithOrders = marketServer_->getTradersWithActiveOrdersCount();
    std::ostringstream json;
    json << "{\"connectedTraders\":" << connectedTraders 
         << ",\"tradersWithOrders\":" << tradersWithOrders << "}";
    return json.str();
}
