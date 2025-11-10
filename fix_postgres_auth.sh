#!/bin/bash
# Script pour configurer l'authentification PostgreSQL

echo "=== Configuration PostgreSQL ==="
echo ""

# Option 1: Définir un mot de passe pour l'utilisateur postgres
echo "Option 1: Définir un mot de passe pour postgres"
echo "Exécutez:"
echo "  sudo -u postgres psql -c \"ALTER USER postgres PASSWORD 'votre_mot_de_passe';\""
echo ""

# Option 2: Modifier pg_hba.conf pour permettre l'authentification peer/local
echo "Option 2: Modifier pg_hba.conf pour l'authentification locale"
echo "Éditez: /etc/postgresql/*/main/pg_hba.conf"
echo "Changez la ligne:"
echo "  local   all             postgres                                peer"
echo "En:"
echo "  local   all             postgres                                trust"
echo "Puis redémarrez: sudo systemctl restart postgresql"
echo ""

# Option 3: Créer un utilisateur dédié
echo "Option 3: Créer un utilisateur dédié pour le marché"
echo "Exécutez:"
echo "  sudo -u postgres psql -c \"CREATE USER market_user WITH PASSWORD 'market_pass';\""
echo "  sudo -u postgres psql -c \"GRANT ALL PRIVILEGES ON DATABASE market TO market_user;\""
echo "  sudo -u postgres psql -c \"ALTER DATABASE market OWNER TO market_user;\""
echo ""

echo "Ensuite, modifiez src/MarketServer.cpp pour utiliser:"
echo "  host=localhost port=5432 dbname=market user=market_user password=market_pass"

