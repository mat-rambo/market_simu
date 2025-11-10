#!/usr/bin/env python3
"""
Simulation script that connects traders and has them continuously submit orders.
Runs for a specified duration (default 1 minute).
"""

import socket
import time
import random
import threading
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

def submit_order(sock, trader_id, symbols, base_prices):
    """Submit a single random order."""
    try:
        symbol = random.choice(symbols)
        side = random.choice(["BUY", "SELL"])
        
        base_price = base_prices.get(symbol, 100.0)
        # Add some variation (±5%)
        price = round(base_price * (1 + random.uniform(-0.05, 0.05)), 2)
        
        # Random quantity (1-20 shares)
        quantity = random.randint(1, 20)
        
        # Submit order
        order_msg = f"ORDER:{trader_id}:{symbol}:{side}:LIMIT:{price}:{quantity}\n"
        sock.sendall(order_msg.encode())
        
        # Try to receive response
        try:
            sock.settimeout(0.1)
            sock.recv(1024)
        except:
            pass
        
        return True
    except:
        return False

def trader_continuous_trading(host, port, trader_id, duration_seconds=60, order_interval=2.0):
    """Connect a trader and continuously submit orders for the specified duration."""
    orders_submitted = 0
    start_time = time.time()
    
    try:
        # Connect to server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))
        
        # Register trader
        message = f"REGISTER:{trader_id}\n"
        sock.sendall(message.encode())
        time.sleep(0.1)
        
        # Wait for registration confirmation
        try:
            response = sock.recv(1024).decode()
            if "REGISTERED" not in response:
                print(f"❌ {trader_id}: Registration failed")
                sock.close()
                return False
        except:
            pass
        
        # Symbols and prices
        symbols = ["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "META", "NVDA", "NFLX"]
        base_prices = {
            "AAPL": 150.0,
            "GOOGL": 2800.0,
            "MSFT": 400.0,
            "TSLA": 250.0,
            "AMZN": 150.0,
            "META": 350.0,
            "NVDA": 500.0,
            "NFLX": 450.0
        }
        
        # Submit initial order immediately
        if submit_order(sock, trader_id, symbols, base_prices):
            orders_submitted += 1
        
        # Continue submitting orders until duration expires
        while (time.time() - start_time) < duration_seconds:
            time.sleep(order_interval)
            
            if (time.time() - start_time) >= duration_seconds:
                break
            
            if submit_order(sock, trader_id, symbols, base_prices):
                orders_submitted += 1
        
        sock.close()
        elapsed = time.time() - start_time
        print(f"✅ {trader_id}: Submitted {orders_submitted} orders in {elapsed:.1f}s")
        return orders_submitted
        
    except Exception as e:
        print(f"❌ {trader_id}: Error - {e}")
        return 0

def run_simulation(host='51.77.193.65', port=8888, num_traders=100, duration_seconds=60, order_interval=2.0, max_workers=20):
    """Run the simulation with multiple traders continuously trading."""
    print("=" * 70)
    print("Market Simulation - Continuous Trading")
    print("=" * 70)
    print(f"Host: {host}")
    print(f"Port: {port}")
    print(f"Traders: {num_traders}")
    print(f"Duration: {duration_seconds} seconds")
    print(f"Order interval: {order_interval} seconds per trader")
    print(f"Max concurrent connections: {max_workers}")
    print("=" * 70)
    print()
    print("🚀 Starting simulation...")
    print()
    
    start_time = time.time()
    total_orders = 0
    successful_traders = 0
    failed_traders = 0
    
    # Use ThreadPoolExecutor to manage concurrent connections
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # Submit all trader tasks
        futures = {
            executor.submit(trader_continuous_trading, host, port, f"trader{i+1}", duration_seconds, order_interval): i+1
            for i in range(num_traders)
        }
        
        # Process results as they complete
        for future in as_completed(futures):
            trader_num = futures[future]
            try:
                orders = future.result()
                if orders > 0:
                    successful_traders += 1
                    total_orders += orders
                else:
                    failed_traders += 1
            except Exception as e:
                print(f"❌ trader{trader_num}: Exception - {e}")
                failed_traders += 1
    
    elapsed_time = time.time() - start_time
    
    print()
    print("=" * 70)
    print("Simulation Complete!")
    print("=" * 70)
    print(f"✅ Successful traders: {successful_traders}")
    print(f"❌ Failed traders: {failed_traders}")
    print(f"⏱️  Total time: {elapsed_time:.2f} seconds")
    print(f"📈 Total orders submitted: {total_orders}")
    print(f"📊 Average orders per trader: {total_orders/successful_traders if successful_traders > 0 else 0:.1f}")
    print(f"⚡ Orders per second: {total_orders/elapsed_time:.1f}")
    print("=" * 70)
    print()
    print(f"💡 Check the web interface at http://{host}:8080")
    print("   to see all the orders in the orderbooks!")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Simulate multiple traders continuously submitting orders')
    parser.add_argument('--host', default='51.77.193.65', help='Market server host (default: 51.77.193.65)')
    parser.add_argument('--port', type=int, default=8888, help='Market server port (default: 8888)')
    parser.add_argument('--traders', type=int, default=100, help='Number of traders (default: 100)')
    parser.add_argument('--duration', type=int, default=60, help='Simulation duration in seconds (default: 60)')
    parser.add_argument('--interval', type=float, default=2.0, help='Seconds between orders per trader (default: 2.0)')
    parser.add_argument('--workers', type=int, default=20, help='Max concurrent connections (default: 20)')
    
    args = parser.parse_args()
    
    run_simulation(
        host=args.host,
        port=args.port,
        num_traders=args.traders,
        duration_seconds=args.duration,
        order_interval=args.interval,
        max_workers=args.workers
    )

if __name__ == "__main__":
    main()

