#!/bin/bash
# Deployment script for VPS

set -e

echo "=========================================="
echo "Market Simulation - VPS Deployment"
echo "=========================================="
echo ""

# Check if running as root or with sudo
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root or with sudo"
    exit 1
fi

# Update system
echo "📦 Updating system packages..."
apt-get update

# Install dependencies
echo "📦 Installing build dependencies..."
apt-get install -y build-essential cmake git

echo "📦 Installing PostgreSQL..."
apt-get install -y postgresql postgresql-contrib libpq-dev

echo "📦 Installing Python..."
apt-get install -y python3 python3-pip

echo "📦 Installing netcat..."
apt-get install -y netcat

# Set up PostgreSQL
echo "🗄️  Setting up PostgreSQL database..."
if ! sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw market; then
    sudo -u postgres psql -c "CREATE DATABASE market;"
    echo "✅ Database 'market' created"
else
    echo "ℹ️  Database 'market' already exists"
fi

# Build project
echo "🔨 Building project..."
if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
make market_simulation
cd ..

echo ""
echo "✅ Build complete!"
echo ""
echo "To run the server:"
echo "  ./build/market_simulation 8888 8080"
echo ""
echo "To set up as a systemd service, see DEPLOYMENT.md"

