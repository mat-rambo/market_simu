# Apache/Nginx - Optionnel mais Recommandé pour Production

## Réponse courte : NON, pas nécessaire

Le serveur web est **déjà intégré** dans l'application C++ (`WebServer`). Vous pouvez utiliser directement le serveur sur le port 8080 sans Apache ou Nginx.

## Quand utiliser Nginx/Apache ?

### ✅ Recommandé pour la production si vous voulez :

1. **HTTPS/SSL** - Certificats SSL pour connexions sécurisées
2. **Nom de domaine** - Utiliser `votre-domaine.com` au lieu de `IP:8080`
3. **Meilleure sécurité** - Nginx gère mieux les attaques DDoS
4. **Performance** - Nginx est optimisé pour servir des fichiers statiques
5. **Plusieurs applications** - Gérer plusieurs services sur le même serveur

### ❌ Pas nécessaire si :

- Vous testez en développement
- Vous utilisez juste l'IP et le port directement
- Vous n'avez pas besoin de HTTPS
- Vous n'avez pas de nom de domaine

## Configuration Nginx (Optionnel)

Si vous voulez utiliser Nginx comme reverse proxy :

### 1. Installer Nginx

```bash
sudo apt-get update
sudo apt-get install nginx
```

### 2. Configuration Reverse Proxy

Créez `/etc/nginx/sites-available/market_simulation` :

```nginx
server {
    listen 80;
    server_name votre-domaine.com;  # ou votre IP

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # WebSocket support (si vous l'implémentez plus tard)
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

### 3. Activer le site

```bash
sudo ln -s /etc/nginx/sites-available/market_simulation /etc/nginx/sites-enabled/
sudo nginx -t  # Tester la configuration
sudo systemctl reload nginx
```

### 4. Accéder

Maintenant vous pouvez accéder via :
- `http://votre-domaine.com` (au lieu de `http://IP:8080`)
- Ou `http://votre-IP` (si pas de nom de domaine)

## Configuration HTTPS avec Let's Encrypt (Optionnel)

Si vous avez un nom de domaine :

```bash
# Installer Certbot
sudo apt-get install certbot python3-certbot-nginx

# Obtenir un certificat SSL
sudo certbot --nginx -d votre-domaine.com

# Renouvellement automatique
sudo certbot renew --dry-run
```

Puis accédez via `https://votre-domaine.com`

## Architecture

### Sans Nginx (Simple)
```
Client → http://IP:8080 → WebServer (C++)
```

### Avec Nginx (Production)
```
Client → http://votre-domaine.com → Nginx → http://localhost:8080 → WebServer (C++)
```

## Recommandation

- **Développement/Test** : Utilisez directement le port 8080, pas besoin de Nginx
- **Production** : Utilisez Nginx si vous avez un nom de domaine et voulez HTTPS

## Ports à ouvrir

- **Sans Nginx** : Ouvrez le port 8080
- **Avec Nginx** : Ouvrez les ports 80 (HTTP) et 443 (HTTPS), fermez 8080 (Nginx y accède en local)

```bash
# Sans Nginx
sudo ufw allow 8080/tcp

# Avec Nginx
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw deny 8080/tcp  # Bloquer l'accès direct
```

