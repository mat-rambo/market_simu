# Configuration avec Variables d'Environnement (Solution 2)

## Utilisation rapide

### 1. Définir les variables d'environnement

```bash
export PGHOST=localhost
export PGPORT=5432
export PGDATABASE=market
export PGUSER=postgres
export PGPASSWORD=votre_mot_de_passe
```

### 2. Lancer le serveur

```bash
./build/market_simulation
```

Ou utilisez le script helper :

```bash
PGPASSWORD=votre_mot_de_passe ./run_with_env.sh
```

## Variables d'environnement supportées

- `PGHOST` - Hôte PostgreSQL (défaut: localhost)
- `PGPORT` - Port PostgreSQL (défaut: 5432)
- `PGDATABASE` - Nom de la base de données (défaut: market)
- `PGUSER` - Utilisateur PostgreSQL (défaut: postgres)
- `PGPASSWORD` - Mot de passe (requis si authentification par mot de passe)

## Exemple complet

```bash
# Définir les variables
export PGHOST=localhost
export PGPORT=5432
export PGDATABASE=market
export PGUSER=postgres
export PGPASSWORD=postgres

# Lancer le serveur
./build/market_simulation 8888 8080
```

## Persistance des variables

Pour rendre les variables permanentes, ajoutez-les à `~/.bashrc` :

```bash
echo 'export PGHOST=localhost' >> ~/.bashrc
echo 'export PGPORT=5432' >> ~/.bashrc
echo 'export PGDATABASE=market' >> ~/.bashrc
echo 'export PGUSER=postgres' >> ~/.bashrc
echo 'export PGPASSWORD=votre_mot_de_passe' >> ~/.bashrc
source ~/.bashrc
```

## Pour systemd service

Modifiez `/etc/systemd/system/market-simulation.service` :

```ini
[Service]
Environment="PGHOST=localhost"
Environment="PGPORT=5432"
Environment="PGDATABASE=market"
Environment="PGUSER=postgres"
Environment="PGPASSWORD=votre_mot_de_passe"
```

Puis rechargez et redémarrez :

```bash
sudo systemctl daemon-reload
sudo systemctl restart market-simulation
```

