#!/bin/bash
# Script rapide pour résoudre l'authentification PostgreSQL

set -e

echo "=== Configuration rapide PostgreSQL ==="
echo ""

# Vérifier si on est root ou sudo
if [ "$EUID" -ne 0 ]; then 
    echo "⚠️  Ce script nécessite sudo"
    echo ""
    echo "Exécutez :"
    echo "  sudo ./quick_fix_postgres.sh"
    exit 1
fi

echo "Choisissez une option :"
echo "1. Définir le mot de passe 'postgres' pour l'utilisateur postgres"
echo "2. Créer un utilisateur dédié 'market_user' avec mot de passe 'market_pass'"
echo "3. Modifier pg_hba.conf pour utiliser 'trust' (moins sécurisé)"
echo ""
read -p "Votre choix (1-3) : " choice

case $choice in
    1)
        echo ""
        echo "Définition du mot de passe pour postgres..."
        sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres';"
        echo ""
        echo "✅ Mot de passe défini !"
        echo ""
        echo "Maintenant, lancez le serveur avec :"
        echo "  export PGPASSWORD=postgres"
        echo "  ./build/market_simulation"
        ;;
    2)
        echo ""
        echo "Création de l'utilisateur market_user..."
        sudo -u postgres psql -c "CREATE USER market_user WITH PASSWORD 'market_pass';" 2>/dev/null || echo "Utilisateur existe déjà"
        sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE market TO market_user;" 2>/dev/null || echo "Permissions déjà accordées"
        echo ""
        echo "✅ Utilisateur créé !"
        echo ""
        echo "Maintenant, lancez le serveur avec :"
        echo "  export PGUSER=market_user"
        echo "  export PGPASSWORD=market_pass"
        echo "  ./build/market_simulation"
        ;;
    3)
        echo ""
        echo "⚠️  ATTENTION : Cette option est moins sécurisée !"
        echo "Modification de pg_hba.conf..."
        
        PG_VERSION=$(sudo -u postgres psql -t -c "SHOW server_version_num;" 2>/dev/null | tr -d ' ' | cut -c1-2)
        PG_HBA="/etc/postgresql/${PG_VERSION}/main/pg_hba.conf"
        
        if [ ! -f "$PG_HBA" ]; then
            echo "❌ Fichier pg_hba.conf non trouvé : $PG_HBA"
            exit 1
        fi
        
        # Backup
        cp "$PG_HBA" "${PG_HBA}.backup"
        echo "✅ Backup créé : ${PG_HBA}.backup"
        
        # Modifier
        sed -i 's/local.*postgres.*peer/local   all             postgres                                trust/' "$PG_HBA"
        sed -i 's/local.*postgres.*md5/local   all             postgres                                trust/' "$PG_HBA"
        
        echo "✅ pg_hba.conf modifié"
        echo ""
        echo "Redémarrage de PostgreSQL..."
        systemctl restart postgresql
        echo ""
        echo "✅ PostgreSQL redémarré !"
        echo ""
        echo "Vous pouvez maintenant lancer le serveur sans mot de passe :"
        echo "  ./build/market_simulation"
        ;;
    *)
        echo "❌ Choix invalide"
        exit 1
        ;;
esac

