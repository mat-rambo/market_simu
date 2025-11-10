# PostgreSQL Database Logging

The market server now logs all orders and trades to a PostgreSQL database.

## Installation

### 1. Install PostgreSQL

```bash
sudo apt-get update
sudo apt-get install postgresql postgresql-contrib libpq-dev
```

### 2. Set up the database

Run the setup script:

```bash
./setup_database.sh
```

Or manually:

```bash
# Start PostgreSQL service
sudo systemctl start postgresql

# Create database
sudo -u postgres psql -c "CREATE DATABASE market;"

# Create user (optional)
sudo -u postgres psql -c "CREATE USER market_user WITH PASSWORD 'market_pass';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE market TO market_user;"
```

### 3. Configure connection

The default connection string is:
```
host=localhost port=5432 dbname=market user=postgres password=postgres
```

You can modify this in `src/MarketServer.cpp` or pass it as a parameter.

### 4. Rebuild

```bash
cd build
cmake ..
make market_simulation
```

## Database Schema

### Orders Table
- `order_id` (VARCHAR(255), PRIMARY KEY) - Unique order identifier
- `trader_id` (VARCHAR(255)) - Trader who submitted the order
- `symbol` (VARCHAR(50)) - Trading symbol (e.g., AAPL, GOOGL)
- `side` (VARCHAR(10)) - BUY or SELL
- `type` (VARCHAR(10)) - LIMIT or MARKET
- `price` (DECIMAL(20, 2)) - Order price
- `quantity` (DECIMAL(20, 2)) - Order quantity
- `filled_quantity` (DECIMAL(20, 2)) - Quantity that has been filled
- `status` (VARCHAR(20)) - PENDING, PARTIALLY_FILLED, FILLED, REJECTED
- `timestamp` (BIGINT) - Unix timestamp
- `created_at` (TIMESTAMP) - Database insertion time

### Trades Table
- `trade_id` (VARCHAR(255), PRIMARY KEY) - Unique trade identifier
- `order_id_buy` (VARCHAR(255)) - Buy order ID
- `order_id_sell` (VARCHAR(255)) - Sell order ID
- `symbol` (VARCHAR(50)) - Trading symbol
- `buyer_id` (VARCHAR(255)) - Buyer trader ID
- `seller_id` (VARCHAR(255)) - Seller trader ID
- `price` (DECIMAL(20, 2)) - Trade execution price
- `quantity` (DECIMAL(20, 2)) - Trade quantity
- `timestamp` (BIGINT) - Unix timestamp
- `created_at` (TIMESTAMP) - Database insertion time

## Querying the Database

### Using psql

```bash
psql -U postgres -d market
```

### Example Queries

```sql
-- View recent orders
SELECT * FROM orders ORDER BY timestamp DESC LIMIT 10;

-- View recent trades
SELECT * FROM trades ORDER BY timestamp DESC LIMIT 10;

-- Count orders by symbol
SELECT symbol, COUNT(*) FROM orders GROUP BY symbol;

-- Count trades by symbol
SELECT symbol, COUNT(*) FROM trades GROUP BY symbol;

-- Total volume by symbol
SELECT symbol, SUM(quantity * price) as volume FROM trades GROUP BY symbol;

-- Get all orders for a specific trader
SELECT * FROM orders WHERE trader_id = 'trader1' ORDER BY timestamp DESC;

-- Get all trades for a symbol
SELECT * FROM trades WHERE symbol = 'AAPL' ORDER BY timestamp DESC;

-- Order statistics
SELECT 
    symbol,
    side,
    COUNT(*) as order_count,
    SUM(quantity) as total_quantity,
    AVG(price) as avg_price
FROM orders
GROUP BY symbol, side;

-- Trade statistics
SELECT 
    symbol,
    COUNT(*) as trade_count,
    SUM(quantity) as total_volume,
    AVG(price) as avg_price,
    MIN(price) as min_price,
    MAX(price) as max_price
FROM trades
GROUP BY symbol;
```

### Using the Python Query Script

The `scripts/query_orders.py` script works with PostgreSQL too (it uses SQLite by default, but you can modify it to use psycopg2).

## Performance

- Indexes are created on commonly queried fields (trader_id, symbol, timestamp)
- Uses parameterized queries for safety and performance
- Uses INSERT ... ON CONFLICT for orders to update status and filled quantities
- All database operations are non-blocking (logging happens asynchronously)

## Connection String Format

```
host=localhost port=5432 dbname=market user=postgres password=postgres
```

You can also use environment variables:
- `PGHOST`, `PGPORT`, `PGDATABASE`, `PGUSER`, `PGPASSWORD`
