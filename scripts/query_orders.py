#!/usr/bin/env python3
"""
Simple script to query orders from the SQLite database.
"""

import sqlite3
import sys
from datetime import datetime

def query_orders(db_path="market_orders.db", limit=100):
    """Query and display orders from the database."""
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Get total count
        cursor.execute("SELECT COUNT(*) FROM orders")
        total = cursor.fetchone()[0]
        
        print("=" * 80)
        print(f"Orders Database: {db_path}")
        print(f"Total orders: {total}")
        print("=" * 80)
        print()
        
        # Query recent orders
        query = """
            SELECT order_id, trader_id, symbol, side, type, price, quantity, 
                   filled_quantity, status, datetime(timestamp, 'unixepoch') as order_time
            FROM orders
            ORDER BY timestamp DESC
            LIMIT ?
        """
        
        cursor.execute(query, (limit,))
        orders = cursor.fetchall()
        
        if orders:
            print(f"Recent orders (showing {len(orders)}):")
            print("-" * 80)
            print(f"{'Order ID':<20} {'Trader':<12} {'Symbol':<8} {'Side':<6} {'Type':<8} {'Price':<10} {'Qty':<8} {'Filled':<8} {'Status':<15} {'Time'}")
            print("-" * 80)
            
            for order in orders:
                order_id, trader_id, symbol, side, type_, price, qty, filled, status, order_time = order
                print(f"{order_id:<20} {trader_id:<12} {symbol:<8} {side:<6} {type_:<8} ${price:<9.2f} {qty:<8.2f} {filled:<8.2f} {status:<15} {order_time}")
        else:
            print("No orders found.")
        
        conn.close()
        
    except sqlite3.Error as e:
        print(f"Database error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

def query_trades(db_path="market_orders.db", limit=100):
    """Query and display trades from the database."""
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Get total count
        cursor.execute("SELECT COUNT(*) FROM trades")
        total = cursor.fetchone()[0]
        
        print("=" * 80)
        print(f"Trades Database: {db_path}")
        print(f"Total trades: {total}")
        print("=" * 80)
        print()
        
        # Query recent trades
        query = """
            SELECT trade_id, symbol, buyer_id, seller_id, price, quantity,
                   datetime(timestamp, 'unixepoch') as trade_time
            FROM trades
            ORDER BY timestamp DESC
            LIMIT ?
        """
        
        cursor.execute(query, (limit,))
        trades = cursor.fetchall()
        
        if trades:
            print(f"Recent trades (showing {len(trades)}):")
            print("-" * 80)
            print(f"{'Trade ID':<20} {'Symbol':<8} {'Buyer':<12} {'Seller':<12} {'Price':<10} {'Qty':<8} {'Time'}")
            print("-" * 80)
            
            for trade in trades:
                trade_id, symbol, buyer, seller, price, qty, trade_time = trade
                print(f"{trade_id:<20} {symbol:<8} {buyer:<12} {seller:<12} ${price:<9.2f} {qty:<8.2f} {trade_time}")
        else:
            print("No trades found.")
        
        conn.close()
        
    except sqlite3.Error as e:
        print(f"Database error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description='Query orders and trades from database')
    parser.add_argument('--db', default='market_orders.db', help='Database file path')
    parser.add_argument('--limit', type=int, default=100, help='Number of records to show')
    parser.add_argument('--trades', action='store_true', help='Show trades instead of orders')
    
    args = parser.parse_args()
    
    if args.trades:
        query_trades(args.db, args.limit)
    else:
        query_orders(args.db, args.limit)

