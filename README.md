# Market Simulation

A high-performance C++ market simulation server with real-time order matching, trade settlement, and web-based monitoring interface.

## Features

- **Real-time Order Matching**: Price-time priority matching engine
- **Trade Settlement**: Automatic account updates and position management
- **Web Interface**: Live orderbook visualization with auto-refresh
- **PostgreSQL Logging**: All orders and trades logged to database
- **Multi-trader Support**: Concurrent trader connections
- **Order Types**: Market and Limit orders
- **Partial Fills**: Support for partial order execution

## Architecture

- **MarketServer**: Core server handling trader connections and order processing
- **MatchingEngine**: Matches buy/sell orders based on price-time priority
- **SettlementEngine**: Settles trades and updates trader accounts
- **OrderBook**: Maintains buy/sell order queues per symbol
- **WebServer**: HTTP server for web interface and API endpoints
- **OrderLogger**: PostgreSQL database logging for all orders and trades

## Quick Start

### Prerequisites

- C++17 compiler (g++ 7+ or clang++)
- CMake 3.15+
- PostgreSQL 12+ and libpq-dev (PostgreSQL development headers)
- Python 3.6+ (for scripts)

**Install dependencies (Ubuntu/Debian):**
```bash
sudo apt-get install build-essential cmake postgresql postgresql-contrib libpq-dev python3 python3-pip
```

### Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake postgresql postgresql-contrib libpq-dev

# Clone repository
git clone <repo-url>
cd market_simu

# Set up database
./setup_database.sh

# Build
mkdir build
cd build
cmake ..
make market_simulation
```

### Run

```bash
# Start server (market port 8888, web port 8080)
./build/market_simulation

# Or specify ports
./build/market_simulation 8888 8080
```

### Access

- **Market Server**: `localhost:8888`
- **Web Interface**: `http://localhost:8080`

## Usage

### Connect as Trader

```bash
# Using netcat
(echo "REGISTER:trader1"; sleep 0.5; echo "ORDER:trader1:AAPL:BUY:LIMIT:150.50:10") | nc localhost 8888
```

### Submit Orders

Order format: `ORDER:traderId:symbol:side:type:price:quantity`

- `side`: BUY or SELL
- `type`: LIMIT or MARKET
- `price`: Price for limit orders (0.0 for market orders)

### Run Simulation

```bash
# 100 traders for 1 minute
python3 scripts/simulate_100_traders.py --traders 100 --duration 60
```

## Project Structure

```
market_simu/
├── include/          # Header files
├── src/              # Source files
├── tests/            # Test files
├── scripts/          # Utility scripts
├── web/              # Web interface files
├── CMakeLists.txt    # Build configuration
├── README.md         # This file
├── DEPLOYMENT.md     # VPS deployment guide
└── DATABASE.md       # Database documentation
```

## Testing

```bash
cd build
make market_tests
ctest
```

## Documentation

- [DEPLOYMENT.md](DEPLOYMENT.md) - VPS deployment instructions
- [DATABASE.md](DATABASE.md) - Database schema and queries
- [scripts/README.md](scripts/README.md) - Script usage guide

## License

[Your License Here]

## Contributing

[Contributing Guidelines]
