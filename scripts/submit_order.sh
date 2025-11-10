#!/bin/bash
# Simple script to connect to market server and submit orders
# Usage: ./submit_order.sh [host] [port] [trader_id] [symbol] [side] [type] [price] [quantity]

HOST=${1:-localhost}
PORT=${2:-8888}
TRADER_ID=${3:-trader1}
SYMBOL=${4:-AAPL}
SIDE=${5:-BUY}
TYPE=${6:-LIMIT}
PRICE=${7:-150.00}
QUANTITY=${8:-10}

echo "Connecting to market server at $HOST:$PORT"
echo "Trader: $TRADER_ID"
echo "Order: $SIDE $QUANTITY $SYMBOL @ \$$PRICE ($TYPE)"
echo ""

# Use nc (netcat) to connect and send messages
(
    echo "REGISTER:$TRADER_ID"
    sleep 0.5
    echo "ORDER:$TRADER_ID:$SYMBOL:$SIDE:$TYPE:$PRICE:$QUANTITY"
    sleep 1
) | nc $HOST $PORT

echo ""
echo "Done!"

