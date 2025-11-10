# Configuration PostgreSQL - Résolution des problèmes d'authentification

## Problème : "password authentication failed for user postgres"

### Solution 1 : Définir un mot de passe pour postgres (Recommandé)

```bash
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'votre_mot_de_passe';"
```

Puis modifiez la chaîne de connexion dans `src/MarketServer.cpp` ou utilisez une variable d'environnement.

### Solution 2 : Utiliser l'authentification locale (trust)

Modifiez `/etc/postgresql/*/main/pg_hba.conf` :

```bash
sudo nano /etc/postgresql/*/main/pg_hba.conf
```

Changez :
```
local   all             postgres                                peer
```

En :
```
local   all             postgres                                trust
```

Puis redémarrez PostgreSQL :
```bash
sudo systemctl restart postgresql
```

### Solution 3 : Créer un utilisateur dédié (Recommandé pour production)

```bash
# Créer l'utilisateur
sudo -u postgres psql -c "CREATE USER market_user WITH PASSWORD 'market_pass';"

# Donner les permissions
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE market TO market_user;"
sudo -u postgres psql -c "ALTER DATABASE market OWNER TO market_user;"
```

Puis modifiez la chaîne de connexion :
```
host=localhost port=5432 dbname=market user=market_user password=market_pass
```

### Solution 4 : Utiliser des variables d'environnement

Le serveur peut utiliser les variables d'environnement PostgreSQL :
- `PGHOST=localhost`
- `PGPORT=5432`
- `PGDATABASE=market`
- `PGUSER=postgres`
- `PGPASSWORD=votre_mot_de_passe`

```bash
export PGHOST=localhost
export PGPORT=5432
export PGDATABASE=market
export PGUSER=postgres
export PGPASSWORD=votre_mot_de_passe

./build/market_simulation
```

### Solution 5 : Passer la chaîne de connexion en paramètre

Modifiez `main.cpp` pour accepter une chaîne de connexion en paramètre, ou utilisez un fichier de configuration.

## Vérification

Testez la connexion :
```bash
psql -U postgres -d market -c "SELECT 1;"
```

Ou avec le nouvel utilisateur :
```bash
psql -U market_user -d market -c "SELECT 1;"
```

