#!/bin/bash
# Interactive script to connect as a trader and submit multiple orders
# Usage: ./connect_trader.sh [host] [port] [trader_id]

HOST=${1:-localhost}
PORT=${2:-8888}
TRADER_ID=${3:-trader1}

echo "=========================================="
echo "Market Simulation - Trader Connection"
echo "=========================================="
echo "Host: $HOST"
echo "Port: $PORT"
echo "Trader ID: $TRADER_ID"
echo ""
echo "Connecting..."
echo ""

# Create a named pipe for communication
PIPE=$(mktemp -u)
mkfifo $PIPE

# Start netcat in background, reading from pipe
nc $HOST $PORT < $PIPE &
NC_PID=$!

# Function to send command
send_command() {
    echo "$1" > $PIPE
    sleep 0.3
}

# Register trader
echo "📝 Registering trader: $TRADER_ID"
send_command "REGISTER:$TRADER_ID"

echo ""
echo "✅ Connected! You can now submit orders."
echo ""
echo "Order format: ORDER:traderId:symbol:side:type:price:quantity"
echo "Example: ORDER:$TRADER_ID:AAPL:BUY:LIMIT:150.50:10"
echo ""
echo "Type 'quit' or 'exit' to disconnect"
echo ""

# Interactive loop
while true; do
    read -p "Order> " order
    
    if [[ "$order" == "quit" || "$order" == "exit" ]]; then
        break
    fi
    
    if [[ "$order" =~ ^ORDER: ]]; then
        send_command "$order"
        echo "✅ Order sent"
    else
        echo "❌ Invalid format. Use: ORDER:traderId:symbol:side:type:price:quantity"
    fi
done

# Cleanup
kill $NC_PID 2>/dev/null
rm -f $PIPE
echo ""
echo "👋 Disconnected"

