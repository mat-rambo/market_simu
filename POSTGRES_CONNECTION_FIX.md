# Fix PostgreSQL Connection Refused

## Erreur

```
Connection to database failed: could not connect to server: Connection refused
Is the server running on host "localhost" (127.0.0.1) and accepting TCP/IP connections on port 5432?
```

## Solutions

### 1. Démarrer le service PostgreSQL

```bash
# Vérifier le statut
sudo systemctl status postgresql

# Démarrer le service
sudo systemctl start postgresql

# Activer au démarrage
sudo systemctl enable postgresql
```

### 2. Vérifier que PostgreSQL écoute sur le port 5432

```bash
# Vérifier si le port est ouvert
sudo netstat -tlnp | grep 5432
# ou
sudo ss -tlnp | grep 5432
```

Vous devriez voir quelque chose comme :
```
tcp  0  0  127.0.0.1:5432  0.0.0.0:*  LISTEN
```

### 3. Configurer PostgreSQL pour accepter les connexions TCP/IP

Éditez `/etc/postgresql/*/main/postgresql.conf` :

```bash
sudo nano /etc/postgresql/*/main/postgresql.conf
```

Assurez-vous que :
```ini
listen_addresses = 'localhost'  # ou '*' pour toutes les interfaces
port = 5432
```

Puis redémarrez :
```bash
sudo systemctl restart postgresql
```

### 4. Vérifier pg_hba.conf

Éditez `/etc/postgresql/*/main/pg_hba.conf` :

```bash
sudo nano /etc/postgresql/*/main/pg_hba.conf
```

Assurez-vous qu'il y a une ligne pour les connexions locales :
```
local   all             postgres                                peer
# ou
host    all             all             127.0.0.1/32            md5
```

### 5. Script automatique

Utilisez le script de vérification :

```bash
./check_postgres.sh
```

## Vérification rapide

```bash
# 1. Démarrer PostgreSQL
sudo systemctl start postgresql

# 2. Vérifier qu'il fonctionne
sudo systemctl status postgresql

# 3. Tester la connexion
sudo -u postgres psql -c "SELECT 1;"

# 4. Vérifier que la base existe
sudo -u postgres psql -l | grep market

# 5. Si la base n'existe pas, la créer
sudo -u postgres psql -c "CREATE DATABASE market;"
```

## Problèmes courants

### Le service ne démarre pas

Vérifiez les logs :
```bash
sudo journalctl -u postgresql -n 50
```

### Port déjà utilisé

Si un autre service utilise le port 5432 :
```bash
sudo lsof -i :5432
```

### Permissions

Assurez-vous que l'utilisateur peut se connecter :
```bash
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres';"
```

## Après correction

Relancez votre serveur de marché :
```bash
export PGPASSWORD=postgres
./build/market_simulation 8888 8080
```

