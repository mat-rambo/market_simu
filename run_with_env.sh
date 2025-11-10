#!/bin/bash
# Script pour lancer le serveur avec les variables d'environnement PostgreSQL

echo "=== Market Simulation Server ==="
echo ""

# Vérifier si les variables d'environnement sont définies
if [ -z "$PGPASSWORD" ]; then
    echo "⚠️  PGPASSWORD non défini"
    echo ""
    echo "Définissez les variables d'environnement PostgreSQL :"
    echo ""
    echo "  export PGHOST=localhost"
    echo "  export PGPORT=5432"
    echo "  export PGDATABASE=market"
    echo "  export PGUSER=postgres"
    echo "  export PGPASSWORD=votre_mot_de_passe"
    echo ""
    echo "Ou utilisez ce script avec les valeurs par défaut :"
    echo "  PGPASSWORD=postgres ./run_with_env.sh"
    echo ""
    read -p "Voulez-vous utiliser les valeurs par défaut ? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        export PGHOST=${PGHOST:-localhost}
        export PGPORT=${PGPORT:-5432}
        export PGDATABASE=${PGDATABASE:-market}
        export PGUSER=${PGUSER:-postgres}
        export PGPASSWORD=${PGPASSWORD:-postgres}
        echo "✅ Utilisation des valeurs par défaut"
    else
        echo "❌ Veuillez définir PGPASSWORD avant de continuer"
        exit 1
    fi
else
    echo "✅ Variables d'environnement PostgreSQL détectées"
    echo "   PGHOST=${PGHOST:-localhost}"
    echo "   PGPORT=${PGPORT:-5432}"
    echo "   PGDATABASE=${PGDATABASE:-market}"
    echo "   PGUSER=${PGUSER:-postgres}"
    echo "   PGPASSWORD=***"
fi

echo ""
echo "🚀 Démarrage du serveur..."
echo ""

# Ports par défaut
MARKET_PORT=${1:-8888}
WEB_PORT=${2:-8080}

# Lancer le serveur
cd "$(dirname "$0")"
./build/market_simulation "$MARKET_PORT" "$WEB_PORT"

