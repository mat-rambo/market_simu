#!/bin/bash
# Install market simulation as a systemd service

set -e

if [ "$EUID" -ne 0 ]; then 
    echo "Please run with sudo"
    exit 1
fi

# Get the current directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
INSTALL_DIR="/opt/market_simu"

echo "Installing market simulation service..."
echo "Source: $SCRIPT_DIR"
echo "Install: $INSTALL_DIR"
echo ""

# Create user if doesn't exist
if ! id "market" &>/dev/null; then
    useradd -r -s /bin/false market
    echo "Created user 'market'"
fi

# Copy files to /opt
if [ "$SCRIPT_DIR" != "$INSTALL_DIR" ]; then
    echo "Copying files to $INSTALL_DIR..."
    mkdir -p "$INSTALL_DIR"
    cp -r "$SCRIPT_DIR"/* "$INSTALL_DIR"/
    chown -R market:market "$INSTALL_DIR"
fi

# Update service file with correct paths
sed "s|/opt/market_simu|$INSTALL_DIR|g" "$INSTALL_DIR/market-simulation.service" > /tmp/market-simulation.service

# Install service
cp /tmp/market-simulation.service /etc/systemd/system/market-simulation.service
systemctl daemon-reload

echo ""
echo "✅ Service installed!"
echo ""
echo "To start the service:"
echo "  sudo systemctl start market-simulation"
echo ""
echo "To enable on boot:"
echo "  sudo systemctl enable market-simulation"
echo ""
echo "To check status:"
echo "  sudo systemctl status market-simulation"
echo ""
echo "To view logs:"
echo "  sudo journalctl -u market-simulation -f"

