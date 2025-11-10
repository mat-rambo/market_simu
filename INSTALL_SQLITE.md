# Installing SQLite3 Development Headers

To enable SQL database logging, you need to install the SQLite3 development package.

## Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libsqlite3-dev
```

## After installation:
Rebuild the project:
```bash
cd build
cmake ..
make market_simulation
```

The database file `market_orders.db` will be created in the directory where you run the server.

