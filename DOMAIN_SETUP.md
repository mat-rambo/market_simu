# Configuration avec Nom de Domaine

## Réponse : NON, Nginx n'est pas obligatoire !

Même avec un nom de domaine, vous pouvez utiliser directement le serveur intégré.

## Option 1 : Sans Nginx (Simple)

### Configuration DNS

Configurez votre nom de domaine pour pointer vers l'IP de votre serveur :

```
Type: A
Name: @ (ou votre-domaine.com)
Value: VOTRE_IP_SERVEUR
TTL: 3600
```

### Accès direct

Une fois le DNS configuré, accédez directement via :

```
http://votre-domaine.com:8080
```

**Avantages :**
- ✅ Simple, pas besoin d'installer Nginx
- ✅ Fonctionne immédiatement
- ✅ Moins de configuration

**Inconvénients :**
- ❌ Pas de HTTPS (pas de SSL)
- ❌ Port 8080 visible dans l'URL
- ❌ Moins professionnel

## Option 2 : Avec Nginx (Recommandé pour Production)

### Pourquoi utiliser Nginx avec un nom de domaine ?

1. **HTTPS/SSL** - Certificats SSL gratuits avec Let's Encrypt
2. **URL propre** - `https://votre-domaine.com` au lieu de `http://votre-domaine.com:8080`
3. **Sécurité** - Meilleure protection
4. **Performance** - Optimisé pour servir des fichiers statiques

### Configuration DNS (identique)

```
Type: A
Name: @ (ou votre-domaine.com)
Value: VOTRE_IP_SERVEUR
TTL: 3600
```

### Installation Nginx

```bash
sudo apt-get update
sudo apt-get install nginx
```

### Configuration Reverse Proxy

Créez `/etc/nginx/sites-available/market_simulation` :

```nginx
server {
    listen 80;
    server_name votre-domaine.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

### Activer

```bash
sudo ln -s /etc/nginx/sites-available/market_simulation /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### HTTPS avec Let's Encrypt (Gratuit)

```bash
sudo apt-get install certbot python3-certbot-nginx
sudo certbot --nginx -d votre-domaine.com
```

Maintenant accessible via : `https://votre-domaine.com`

## Comparaison

| Aspect | Sans Nginx | Avec Nginx |
|--------|------------|------------|
| **URL** | `http://domaine.com:8080` | `https://domaine.com` |
| **HTTPS** | ❌ Non | ✅ Oui (gratuit) |
| **Installation** | ✅ Aucune | ⚠️ Nginx + config |
| **Complexité** | ✅ Simple | ⚠️ Moyenne |
| **Sécurité** | ⚠️ Basique | ✅ Meilleure |
| **Professionnel** | ⚠️ Moins | ✅ Oui |

## Recommandation

### Pour développement/test :
- Utilisez directement `http://votre-domaine.com:8080`
- Pas besoin de Nginx

### Pour production :
- Utilisez Nginx avec HTTPS
- URL propre : `https://votre-domaine.com`
- Plus sécurisé et professionnel

## Exemple de Configuration DNS

Si votre domaine est `example.com` et votre IP est `203.0.113.42` :

```
Type: A
Name: @
Value: 203.0.113.42
TTL: 3600
```

Ou pour un sous-domaine :

```
Type: A
Name: market
Value: 203.0.113.42
TTL: 3600
```

Puis accédez via `http://market.example.com:8080` (sans Nginx) ou `https://market.example.com` (avec Nginx).

