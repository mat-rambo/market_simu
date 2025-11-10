#include "MarketServer.h"
#include "WebServer.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

std::unique_ptr<MarketServer> g_server;
std::unique_ptr<WebServer> g_webServer;

void signalHandler(int signal) {
    if (g_webServer) {
        std::cout << "\nShutting down web server..." << std::endl;
        g_webServer->stop();
    }
    if (g_server) {
        std::cout << "Shutting down market server..." << std::endl;
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8888;
    int webPort = 8080;
    
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    if (argc > 2) {
        webPort = std::stoi(argv[2]);
    }
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        g_server = std::make_unique<MarketServer>(port);
        g_webServer = std::make_unique<WebServer>(webPort, g_server.get());
        
        // Start web server in a separate thread
        std::thread webThread([&]() {
            try {
                g_webServer->start();
            } catch (const std::exception& e) {
                std::cerr << "Web server error: " << e.what() << std::endl;
            }
        });
        
        // Start market server (non-blocking, starts accept thread)
        g_server->start();
        
        std::cout << "Market simulation server is running." << std::endl;
        std::cout << "Market server: localhost:" << port << std::endl;
        std::cout << "Web interface: http://localhost:" << webPort << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;
        
        // Keep the main thread alive
        while (g_server->isRunning() || g_webServer->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // Wait for web server thread
        if (webThread.joinable()) {
            webThread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

