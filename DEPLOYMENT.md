# Market Simulation - VPS Deployment Guide

This guide will help you deploy the market simulation on a VPS (Virtual Private Server).

## Prerequisites

- Ubuntu 20.04+ or Debian 10+ VPS
- Root or sudo access
- At least 1GB RAM
- PostgreSQL installed (or use the setup script)

## Quick Deployment

### 1. Clone the Repository

```bash
git clone <your-repo-url>
cd market_simu
```

### 2. Install Dependencies

```bash
# Update system
sudo apt-get update

# Install build tools
sudo apt-get install -y build-essential cmake git

# Install PostgreSQL
sudo apt-get install -y postgresql postgresql-contrib libpq-dev

# Install Python (for scripts)
sudo apt-get install -y python3 python3-pip

# Install netcat (for testing)
sudo apt-get install -y netcat
```

### 3. Set Up Database

```bash
# Run the setup script
chmod +x setup_database.sh
./setup_database.sh
```

Or manually:

```bash
sudo -u postgres psql -c "CREATE DATABASE market;"
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'your_secure_password';"
```

### 4. Build the Project

```bash
mkdir -p build
cd build
cmake ..
make market_simulation
cd ..
```

### 5. Configure Database Connection

Edit `src/MarketServer.cpp` and update the connection string if needed:

```cpp
orderLogger_("host=localhost port=5432 dbname=market user=postgres password=your_password")
```

Or set environment variables:
```bash
export PGHOST=localhost
export PGPORT=5432
export PGDATABASE=market
export PGUSER=postgres
export PGPASSWORD=your_password
```

### 6. Run as a Service (systemd)

Create a systemd service file:

```bash
sudo nano /etc/systemd/system/market-simulation.service
```

Add the following:

```ini
[Unit]
Description=Market Simulation Server
After=network.target postgresql.service

[Service]
Type=simple
User=your_user
WorkingDirectory=/path/to/market_simu
ExecStart=/path/to/market_simu/build/market_simulation 8888 8080
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

Enable and start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable market-simulation
sudo systemctl start market-simulation
sudo systemctl status market-simulation
```

### 7. Configure Firewall

```bash
# Allow market server port (8888)
sudo ufw allow 8888/tcp

# Allow web interface port (8080)
sudo ufw allow 8080/tcp

# Enable firewall
sudo ufw enable
```

### 8. Access the Web Interface

Open in your browser:
```
http://your-vps-ip:8080
```

## Docker Deployment (Alternative)

### Create Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y build-essential cmake git postgresql postgresql-contrib libpq-dev python3 python3-pip netcat && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make market_simulation

EXPOSE 8888 8080

CMD ["./build/market_simulation", "8888", "8080"]
```

### Build and Run

```bash
docker build -t market-simulation .
docker run -d -p 8888:8888 -p 8080:8080 --name market market-simulation
```

## Environment Variables

You can configure the server using environment variables:

```bash
export MARKET_PORT=8888
export WEB_PORT=8080
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=market
export DB_USER=postgres
export DB_PASSWORD=your_password
```

## Monitoring

### View Logs

```bash
# Systemd service logs
sudo journalctl -u market-simulation -f

# Or if running directly
tail -f /var/log/market_simulation.log
```

### Check Database

```bash
psql -U postgres -d market -c "SELECT COUNT(*) FROM orders;"
psql -U postgres -d market -c "SELECT COUNT(*) FROM trades;"
```

## Security Considerations

1. **Change default PostgreSQL password**
2. **Use firewall** to restrict access
3. **Consider using nginx** as reverse proxy for HTTPS
4. **Use environment variables** for sensitive data
5. **Run as non-root user** in production
6. **Enable PostgreSQL SSL** for remote connections

## Troubleshooting

### Database Connection Issues

```bash
# Check PostgreSQL is running
sudo systemctl status postgresql

# Check connection
psql -U postgres -d market -c "SELECT 1;"
```

### Port Already in Use

```bash
# Find process using port
sudo lsof -i :8888
sudo lsof -i :8080

# Kill process
sudo kill -9 <PID>
```

### Build Errors

```bash
# Clean and rebuild
rm -rf build
mkdir build
cd build
cmake ..
make
```

## Backup Database

```bash
# Backup
pg_dump -U postgres market > market_backup.sql

# Restore
psql -U postgres market < market_backup.sql
```

