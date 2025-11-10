# Fix PostgreSQL SSL Errors

## Problème

Erreurs SSL lors de la connexion à PostgreSQL :
- `SSL error: called a function you should not call`
- `SSL error: bad length`
- `SSL SYSCALL error: EOF detected`

## Solution

Le code a été mis à jour pour désactiver SSL pour les connexions locales en ajoutant `sslmode=disable` à la chaîne de connexion.

## Si vous voulez utiliser SSL (optionnel)

### 1. Configurer PostgreSQL pour SSL

```bash
# Éditer postgresql.conf
sudo nano /etc/postgresql/*/main/postgresql.conf

# Activer SSL
ssl = on
ssl_cert_file = '/etc/ssl/certs/ssl-cert-snakeoil.pem'
ssl_key_file = '/etc/ssl/private/ssl-cert-snakeoil.key'
```

### 2. Redémarrer PostgreSQL

```bash
sudo systemctl restart postgresql
```

### 3. Modifier le code pour utiliser SSL

Dans `src/OrderLogger.cpp`, changez :
```cpp
envConnStr << "sslmode=disable";
```

En :
```cpp
envConnStr << "sslmode=require";  // ou "prefer", "allow"
```

## Modes SSL disponibles

- `disable` - Pas de SSL (par défaut maintenant)
- `allow` - SSL si disponible
- `prefer` - SSL si disponible, sinon pas de SSL
- `require` - SSL requis
- `verify-ca` - SSL avec vérification du certificat
- `verify-full` - SSL avec vérification complète

## Pour les connexions distantes

Si vous vous connectez à un PostgreSQL distant, vous devrez peut-être activer SSL :

```bash
export PGSSLMODE=require
```

Ou dans la chaîne de connexion :
```
host=remote_host port=5432 dbname=market user=user password=pass sslmode=require
```

