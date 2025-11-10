#!/bin/bash
# Script to set up PostgreSQL database for market simulation

echo "Setting up PostgreSQL database for market simulation..."
echo ""

# Check if PostgreSQL is installed
if ! command -v psql &> /dev/null; then
    echo "PostgreSQL is not installed. Installing..."
    sudo apt-get update
    sudo apt-get install -y postgresql postgresql-contrib libpq-dev
    echo "PostgreSQL installed!"
    echo ""
fi

# Check if PostgreSQL service is running
if ! sudo systemctl is-active --quiet postgresql; then
    echo "Starting PostgreSQL service..."
    sudo systemctl start postgresql
    sudo systemctl enable postgresql
fi

# Create database and user
echo "Creating database and user..."
sudo -u postgres psql << EOF
-- Create database
CREATE DATABASE market;

-- Create user (if not exists)
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_user WHERE usename = 'market_user') THEN
        CREATE USER market_user WITH PASSWORD 'market_pass';
    END IF;
END
\$\$;

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE market TO market_user;

-- Connect to market database and grant schema privileges
\c market
GRANT ALL ON SCHEMA public TO market_user;
EOF

echo ""
echo "✅ Database setup complete!"
echo ""
echo "Connection string:"
echo "  host=localhost port=5432 dbname=market user=market_user password=market_pass"
echo ""
echo "Or use default postgres user:"
echo "  host=localhost port=5432 dbname=market user=postgres password=postgres"
echo ""
echo "To change the password for postgres user:"
echo "  sudo -u postgres psql -c \"ALTER USER postgres PASSWORD 'your_password';\""

