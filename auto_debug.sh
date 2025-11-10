#!/bin/bash
# Script pour déboguer automatiquement le crash

export PGPASSWORD=postgres

echo "🔍 Lancement automatique de gdb..."
echo ""

gdb -ex "file ./build/market_simulation" \
    -ex "run 8888 8080" \
    -ex "bt" \
    -ex "info registers" \
    -ex "quit" \
    ./build/market_simulation
