#!/usr/bin/env python3
"""
Query orders and trades from PostgreSQL database.
Requires: pip install psycopg2-binary
"""

import sys
import os
from datetime import datetime

try:
    import psycopg2
    from psycopg2.extras import RealDictCursor
except ImportError:
    print("Error: psycopg2 not installed. Install with: pip install psycopg2-binary")
    sys.exit(1)

def get_connection(dbname="market", user="postgres", password="postgres", 
                   host="localhost", port=5432):
    """Get PostgreSQL connection."""
    try:
        conn = psycopg2.connect(
            dbname=dbname,
            user=user,
            password=password,
            host=host,
            port=port
        )
        return conn
    except psycopg2.Error as e:
        print(f"Database connection error: {e}")
        sys.exit(1)

def query_orders(conn, limit=100):
    """Query and display orders from the database."""
    cursor = conn.cursor(cursor_factory=RealDictCursor)
    
    # Get total count
    cursor.execute("SELECT COUNT(*) FROM orders")
    total = cursor.fetchone()['count']
    
    print("=" * 80)
    print(f"Orders Database")
    print(f"Total orders: {total}")
    print("=" * 80)
    print()
    
    # Query recent orders
    query = """
        SELECT order_id, trader_id, symbol, side, type, price, quantity, 
               filled_quantity, status, to_timestamp(timestamp) as order_time
        FROM orders
        ORDER BY timestamp DESC
        LIMIT %s
    """
    
    cursor.execute(query, (limit,))
    orders = cursor.fetchall()
    
    if orders:
        print(f"Recent orders (showing {len(orders)}):")
        print("-" * 80)
        print(f"{'Order ID':<20} {'Trader':<12} {'Symbol':<8} {'Side':<6} {'Type':<8} {'Price':<10} {'Qty':<8} {'Filled':<8} {'Status':<15} {'Time'}")
        print("-" * 80)
        
        for order in orders:
            print(f"{order['order_id']:<20} {order['trader_id']:<12} {order['symbol']:<8} "
                  f"{order['side']:<6} {order['type']:<8} ${order['price']:<9.2f} "
                  f"{order['quantity']:<8.2f} {order['filled_quantity']:<8.2f} "
                  f"{order['status']:<15} {order['order_time']}")
    else:
        print("No orders found.")
    
    cursor.close()

def query_trades(conn, limit=100):
    """Query and display trades from the database."""
    cursor = conn.cursor(cursor_factory=RealDictCursor)
    
    # Get total count
    cursor.execute("SELECT COUNT(*) FROM trades")
    total = cursor.fetchone()['count']
    
    print("=" * 80)
    print(f"Trades Database")
    print(f"Total trades: {total}")
    print("=" * 80)
    print()
    
    # Query recent trades
    query = """
        SELECT trade_id, symbol, buyer_id, seller_id, price, quantity,
               to_timestamp(timestamp) as trade_time
        FROM trades
        ORDER BY timestamp DESC
        LIMIT %s
    """
    
    cursor.execute(query, (limit,))
    trades = cursor.fetchall()
    
    if trades:
        print(f"Recent trades (showing {len(trades)}):")
        print("-" * 80)
        print(f"{'Trade ID':<20} {'Symbol':<8} {'Buyer':<12} {'Seller':<12} {'Price':<10} {'Qty':<8} {'Time'}")
        print("-" * 80)
        
        for trade in trades:
            print(f"{trade['trade_id']:<20} {trade['symbol']:<8} {trade['buyer_id']:<12} "
                  f"{trade['seller_id']:<12} ${trade['price']:<9.2f} {trade['quantity']:<8.2f} "
                  f"{trade['trade_time']}")
    else:
        print("No trades found.")
    
    cursor.close()

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description='Query orders and trades from PostgreSQL database')
    parser.add_argument('--dbname', default='market', help='Database name')
    parser.add_argument('--user', default='postgres', help='Database user')
    parser.add_argument('--password', default='postgres', help='Database password')
    parser.add_argument('--host', default='localhost', help='Database host')
    parser.add_argument('--port', type=int, default=5432, help='Database port')
    parser.add_argument('--limit', type=int, default=100, help='Number of records to show')
    parser.add_argument('--trades', action='store_true', help='Show trades instead of orders')
    
    args = parser.parse_args()
    
    conn = get_connection(args.dbname, args.user, args.password, args.host, args.port)
    
    try:
        if args.trades:
            query_trades(conn, args.limit)
        else:
            query_orders(conn, args.limit)
    finally:
        conn.close()

