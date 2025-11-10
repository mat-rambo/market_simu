#!/bin/bash
# Script pour vérifier et démarrer PostgreSQL

echo "=== Vérification PostgreSQL ==="
echo ""

# Vérifier si PostgreSQL est installé
if ! command -v psql &> /dev/null; then
    echo "❌ PostgreSQL n'est pas installé"
    echo ""
    echo "Installez-le avec :"
    echo "  sudo apt-get install postgresql postgresql-contrib"
    exit 1
fi

echo "✅ PostgreSQL est installé"
echo ""

# Vérifier si le service est en cours d'exécution
if sudo systemctl is-active --quiet postgresql; then
    echo "✅ Service PostgreSQL est en cours d'exécution"
else
    echo "❌ Service PostgreSQL n'est PAS en cours d'exécution"
    echo ""
    echo "Démarrage du service..."
    sudo systemctl start postgresql
    sudo systemctl enable postgresql
    
    if sudo systemctl is-active --quiet postgresql; then
        echo "✅ Service démarré avec succès"
    else
        echo "❌ Échec du démarrage du service"
        exit 1
    fi
fi

echo ""

# Vérifier si le port écoute
if sudo netstat -tlnp | grep -q ":5432.*LISTEN" || sudo ss -tlnp | grep -q ":5432"; then
    echo "✅ PostgreSQL écoute sur le port 5432"
else
    echo "⚠️  PostgreSQL ne semble pas écouter sur le port 5432"
    echo ""
    echo "Vérifiez la configuration dans :"
    echo "  /etc/postgresql/*/main/postgresql.conf"
    echo ""
    echo "Assurez-vous que :"
    echo "  listen_addresses = 'localhost'  (ou '*' pour toutes les interfaces)"
fi

echo ""

# Tester la connexion
echo "Test de connexion..."
if sudo -u postgres psql -c "SELECT 1;" &> /dev/null; then
    echo "✅ Connexion réussie"
else
    echo "❌ Échec de la connexion"
    echo ""
    echo "Essayez manuellement :"
    echo "  sudo -u postgres psql -c \"SELECT 1;\""
fi

echo ""

# Vérifier si la base de données existe
if sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw market; then
    echo "✅ Base de données 'market' existe"
else
    echo "⚠️  Base de données 'market' n'existe pas"
    echo ""
    echo "Créez-la avec :"
    echo "  sudo -u postgres psql -c \"CREATE DATABASE market;\""
fi

echo ""
echo "=== Résumé ==="
echo ""
echo "Pour démarrer PostgreSQL :"
echo "  sudo systemctl start postgresql"
echo ""
echo "Pour vérifier le statut :"
echo "  sudo systemctl status postgresql"
echo ""
echo "Pour voir les logs :"
echo "  sudo journalctl -u postgresql -f"

