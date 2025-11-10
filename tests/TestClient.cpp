#include "TestClient.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/select.h>
#include <thread>
#include <chrono>

TestClient::TestClient(const std::string& host, int port)
    : host_(host), port_(port), socket_(-1) {
}

TestClient::~TestClient() {
    disconnect();
}

bool TestClient::connect() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        return false;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr) <= 0) {
        close(socket_);
        socket_ = -1;
        return false;
    }
    
    // Try to connect with retries
    for (int i = 0; i < 10; ++i) {
        if (::connect(socket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    close(socket_);
    socket_ = -1;
    return false;
}

void TestClient::disconnect() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

std::string TestClient::sendMessage(const std::string& message) {
    if (socket_ < 0) {
        return "";
    }
    
    std::string msg = message;
    if (msg.back() != '\n') {
        msg += "\n";
    }
    
    if (send(socket_, msg.c_str(), msg.length(), 0) < 0) {
        return "";
    }
    
    // Wait for response with timeout
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(socket_, &readfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    int selectResult = select(socket_ + 1, &readfds, nullptr, nullptr, &tv);
    if (selectResult <= 0) {
        return "";
    }
    
    char buffer[4096];
    ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return "";
    }
    
    buffer[bytesRead] = '\0';
    std::string response(buffer);
    
    // Remove trailing newline
    if (!response.empty() && response.back() == '\n') {
        response.pop_back();
    }
    
    return response;
}

bool TestClient::registerTrader(const std::string& traderId) {
    std::string response = sendMessage("REGISTER:" + traderId);
    return response.find("REGISTERED:") == 0;
}

std::string TestClient::submitOrder(const std::string& traderId, const std::string& symbol,
                                    const std::string& side, const std::string& type,
                                    double price, double quantity) {
    std::ostringstream oss;
    oss << "ORDER:" << traderId << ":" << symbol << ":" << side << ":" 
        << type << ":" << price << ":" << quantity;
    return sendMessage(oss.str());
}

std::string TestClient::receiveMessage() {
    if (socket_ < 0) {
        return "";
    }
    
    // Check if data is available (non-blocking)
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(socket_, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    int selectResult = select(socket_ + 1, &readfds, nullptr, nullptr, &tv);
    if (selectResult <= 0) {
        return "";
    }
    
    char buffer[4096];
    ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return "";
    }
    
    buffer[bytesRead] = '\0';
    std::string message(buffer);
    
    // Remove trailing newline
    if (!message.empty() && message.back() == '\n') {
        message.pop_back();
    }
    
    return message;
}

