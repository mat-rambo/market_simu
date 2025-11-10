#!/usr/bin/env python3
"""
Simple script to connect a trader and submit orders to the market server.
"""

import socket
import time
import sys

def connect_to_server(host='localhost', port=8888):
    """Connect to the market server."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((host, port))
        print(f"✅ Connected to market server at {host}:{port}")
        return sock
    except Exception as e:
        print(f"❌ Failed to connect: {e}")
        sys.exit(1)

def register_trader(sock, trader_id):
    """Register a trader with the server."""
    message = f"REGISTER:{trader_id}\n"
    sock.sendall(message.encode())
    print(f"📝 Sent registration: {message.strip()}")
    
    # Wait for response
    time.sleep(0.5)
    try:
        response = sock.recv(1024).decode()
        print(f"📨 Response: {response.strip()}")
    except:
        pass

def submit_order(sock, trader_id, side, symbol, order_type, price, quantity):
    """Submit an order to the market.
    
    Format: ORDER:traderId:symbol:side:type:price:quantity
    side: BUY or SELL
    type: LIMIT or MARKET
    """
    message = f"ORDER:{trader_id}:{symbol}:{side}:{order_type}:{price}:{quantity}\n"
    sock.sendall(message.encode())
    print(f"📊 Submitted order: {side} {quantity} {symbol} @ ${price} ({order_type})")
    
    # Wait for response
    time.sleep(0.5)
    try:
        response = sock.recv(1024).decode()
        print(f"📨 Response: {response.strip()}")
    except:
        pass

def main():
    host = 'localhost'
    port = 8888
    
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    
    print("=" * 60)
    print("Market Simulation - Trade Submission Script")
    print("=" * 60)
    print()
    
    # Connect to server
    sock = connect_to_server(host, port)
    
    # Register trader
    trader_id = "trader1"
    print(f"\n👤 Registering trader: {trader_id}")
    register_trader(sock, trader_id)
    time.sleep(0.5)
    
    # Submit orders
    print(f"\n📈 Submitting orders...")
    print("-" * 60)
    
    # Order 1: Buy AAPL (Limit order)
    print("\n1️⃣  Buy Order (Limit):")
    submit_order(sock, trader_id, "BUY", "AAPL", "LIMIT", 150.50, 10)
    time.sleep(1)
    
    # Order 2: Sell AAPL (Limit order)
    print("\n2️⃣  Sell Order (Limit):")
    submit_order(sock, trader_id, "SELL", "AAPL", "LIMIT", 151.00, 5)
    time.sleep(1)
    
    # Order 3: Buy GOOGL (Limit order)
    print("\n3️⃣  Buy Order (different symbol, Limit):")
    submit_order(sock, trader_id, "BUY", "GOOGL", "LIMIT", 2800.00, 2)
    time.sleep(1)
    
    print("\n" + "=" * 60)
    print("✅ All orders submitted!")
    print("=" * 60)
    print("\n💡 Check the web interface at http://localhost:8080")
    print("   to see the orderbook updates.")
    
    # Keep connection open for a bit to receive any notifications
    print("\n⏳ Waiting 3 seconds for trade notifications...")
    time.sleep(3)
    
    # Check for any notifications
    try:
        sock.settimeout(0.1)
        while True:
            try:
                data = sock.recv(1024)
                if data:
                    print(f"📬 Notification: {data.decode().strip()}")
                else:
                    break
            except socket.timeout:
                break
    except:
        pass
    
    sock.close()
    print("\n👋 Disconnected from server")

if __name__ == "__main__":
    main()

