# Market Simulation - Command Line Tools

## Quick Commands

### Using netcat (nc)

**Register and submit a single order:**
```bash
(echo "REGISTER:trader1"; sleep 0.5; echo "ORDER:trader1:AAPL:BUY:LIMIT:150.50:10") | nc localhost 8888
```

**Submit multiple orders:**
```bash
(echo "REGISTER:trader1"; sleep 0.5; \
 echo "ORDER:trader1:AAPL:BUY:LIMIT:150.50:10"; sleep 0.5; \
 echo "ORDER:trader1:AAPL:SELL:LIMIT:151.00:5"; sleep 0.5; \
 echo "ORDER:trader1:GOOGL:BUY:LIMIT:2800.00:2") | nc localhost 8888
```

### Using telnet

**Interactive connection:**
```bash
telnet localhost 8888
```

Then type:
```
REGISTER:trader1
ORDER:trader1:AAPL:BUY:LIMIT:150.50:10
ORDER:trader1:AAPL:SELL:LIMIT:151.00:5
```

## Scripts

### submit_order.sh

Submit a single order quickly:
```bash
./scripts/submit_order.sh [host] [port] [trader_id] [symbol] [side] [type] [price] [quantity]
```

**Examples:**
```bash
# Buy 10 AAPL at $150.50
./scripts/submit_order.sh localhost 8888 trader1 AAPL BUY LIMIT 150.50 10

# Sell 5 AAPL at $151.00
./scripts/submit_order.sh localhost 8888 trader1 AAPL SELL LIMIT 151.00 5

# Buy 2 GOOGL at $2800.00
./scripts/submit_order.sh localhost 8888 trader1 GOOGL BUY LIMIT 2800.00 2
```

### connect_trader.sh

Interactive trader connection:
```bash
./scripts/connect_trader.sh [host] [port] [trader_id]
```

**Example:**
```bash
./scripts/connect_trader.sh localhost 8888 trader1
```

Then type orders interactively:
```
Order> ORDER:trader1:AAPL:BUY:LIMIT:150.50:10
Order> ORDER:trader1:AAPL:SELL:LIMIT:151.00:5
Order> quit
```

## Order Format

```
ORDER:traderId:symbol:side:type:price:quantity
```

- **traderId**: Your trader ID (must match registration)
- **symbol**: Trading symbol (e.g., "AAPL", "GOOGL")
- **side**: "BUY" or "SELL"
- **type**: "LIMIT" or "MARKET"
- **price**: Price for limit orders (use 0.0 for market orders)
- **quantity**: Number of shares

## Examples

**Buy 10 AAPL at $150.50:**
```bash
(echo "REGISTER:trader1"; sleep 0.5; echo "ORDER:trader1:AAPL:BUY:LIMIT:150.50:10") | nc localhost 8888
```

**Sell 5 AAPL at $151.00:**
```bash
(echo "REGISTER:trader1"; sleep 0.5; echo "ORDER:trader1:AAPL:SELL:LIMIT:151.00:5") | nc localhost 8888
```

**Market order (buy 10 AAPL at market price):**
```bash
(echo "REGISTER:trader1"; sleep 0.5; echo "ORDER:trader1:AAPL:BUY:MARKET:0.0:10") | nc localhost 8888
```

## Simulation Scripts

### simulate_100_traders.py

Simulate traders continuously submitting orders for a specified duration (default 1 minute):
```bash
python3 scripts/simulate_100_traders.py
```

**Options:**
```bash
# Run for 1 minute (default)
python3 scripts/simulate_100_traders.py

# Run for 2 minutes
python3 scripts/simulate_100_traders.py --duration 120

# 50 traders, faster order submission (1 second interval)
python3 scripts/simulate_100_traders.py --traders 50 --interval 1.0

# Adjust concurrency (how many traders connect simultaneously)
python3 scripts/simulate_100_traders.py --workers 10

# Full example: 100 traders for 1 minute, order every 2 seconds
python3 scripts/simulate_100_traders.py --host localhost --port 8888 --traders 100 --duration 60 --interval 2.0 --workers 20
```

**What it does:**
- Connects 100 traders (trader1, trader2, ..., trader100)
- Each trader continuously submits orders for the specified duration
- Default: 60 seconds duration, order every 2 seconds = ~30 orders per trader
- Orders are randomly distributed across symbols (AAPL, GOOGL, MSFT, TSLA, AMZN, META, NVDA, NFLX)
- Random buy/sell orders with realistic prices (±5% variation)
- Uses threading for concurrent connections
- Shows real-time progress and final statistics
- Perfect for stress testing and watching live orderbook updates!

