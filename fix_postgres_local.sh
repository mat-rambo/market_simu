#!/bin/bash
# Script pour corriger PostgreSQL sur machine locale

echo "=== Correction PostgreSQL - Machine Locale ==="
echo ""

# Vérifier si PostgreSQL est installé
if ! command -v psql &> /dev/null; then
    echo "❌ PostgreSQL n'est pas installé"
    echo ""
    echo "Installez-le avec :"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install postgresql postgresql-contrib libpq-dev"
    exit 1
fi

echo "✅ PostgreSQL est installé"
echo ""

# Vérifier si le service est en cours d'exécution
if systemctl --user is-active --quiet postgresql 2>/dev/null || sudo systemctl is-active --quiet postgresql 2>/dev/null; then
    echo "✅ Service PostgreSQL est en cours d'exécution"
else
    echo "❌ Service PostgreSQL n'est PAS en cours d'exécution"
    echo ""
    read -p "Voulez-vous démarrer PostgreSQL ? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if systemctl --user start postgresql 2>/dev/null; then
            echo "✅ Service démarré (user mode)"
        elif sudo systemctl start postgresql 2>/dev/null; then
            echo "✅ Service démarré (system mode)"
            sudo systemctl enable postgresql
        else
            echo "❌ Impossible de démarrer le service"
            echo ""
            echo "Essayez manuellement :"
            echo "  sudo systemctl start postgresql"
            exit 1
        fi
    else
        echo "Service non démarré. Vous devrez le démarrer manuellement."
        exit 1
    fi
fi

echo ""

# Vérifier si le port écoute
if netstat -tlnp 2>/dev/null | grep -q ":5432.*LISTEN" || ss -tlnp 2>/dev/null | grep -q ":5432"; then
    echo "✅ PostgreSQL écoute sur le port 5432"
else
    echo "⚠️  PostgreSQL ne semble pas écouter sur le port 5432"
    echo ""
    echo "Vérifiez la configuration dans :"
    echo "  /etc/postgresql/*/main/postgresql.conf"
    echo ""
    echo "Assurez-vous que :"
    echo "  listen_addresses = 'localhost'"
fi

echo ""

# Tester la connexion
echo "Test de connexion..."
if psql -U postgres -d postgres -c "SELECT 1;" &> /dev/null 2>&1 || sudo -u postgres psql -c "SELECT 1;" &> /dev/null 2>&1; then
    echo "✅ Connexion réussie"
else
    echo "⚠️  Connexion échouée (peut nécessiter un mot de passe)"
    echo ""
    echo "Essayez manuellement :"
    echo "  psql -U postgres"
    echo "  ou"
    echo "  sudo -u postgres psql"
fi

echo ""

# Vérifier si la base de données existe
if psql -U postgres -lqt 2>/dev/null | cut -d \| -f 1 | grep -qw market || sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw market; then
    echo "✅ Base de données 'market' existe"
else
    echo "⚠️  Base de données 'market' n'existe pas"
    echo ""
    read -p "Voulez-vous la créer ? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if psql -U postgres -c "CREATE DATABASE market;" 2>/dev/null; then
            echo "✅ Base de données créée"
        elif sudo -u postgres psql -c "CREATE DATABASE market;" 2>/dev/null; then
            echo "✅ Base de données créée"
        else
            echo "❌ Échec de la création"
            echo ""
            echo "Créez-la manuellement :"
            echo "  sudo -u postgres psql -c \"CREATE DATABASE market;\""
        fi
    fi
fi

echo ""
echo "=== Résumé ==="
echo ""
echo "Pour démarrer PostgreSQL manuellement :"
echo "  sudo systemctl start postgresql"
echo ""
echo "Pour vérifier le statut :"
echo "  sudo systemctl status postgresql"
echo ""
echo "Pour créer la base de données :"
echo "  sudo -u postgres psql -c \"CREATE DATABASE market;\""
echo ""
echo "Pour définir un mot de passe :"
echo "  sudo -u postgres psql -c \"ALTER USER postgres PASSWORD 'postgres';\""
echo ""
echo "Ensuite, lancez le serveur avec :"
echo "  export PGPASSWORD=postgres"
echo "  ./build/market_simulation"

