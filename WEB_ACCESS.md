# Accès à l'Interface Web

## Accès Local (sur le serveur)

Si vous êtes connecté directement au serveur :

```
http://localhost:8080
```

ou

```
http://127.0.0.1:8080
```

## Accès à Distance (depuis votre ordinateur)

### 1. Trouver l'IP de votre serveur

```bash
# Sur le serveur, exécutez :
hostname -I
# ou
ip addr show
# ou
curl ifconfig.me
```

### 2. Accéder via l'IP publique

```
http://VOTRE_IP_SERVEUR:8080
```

Exemple :
```
http://192.168.1.100:8080
http://203.0.113.42:8080
```

### 3. Configurer le Firewall

Assurez-vous que le port 8080 est ouvert :

```bash
# UFW (Ubuntu)
sudo ufw allow 8080/tcp
sudo ufw status

# iptables
sudo iptables -A INPUT -p tcp --dport 8080 -j ACCEPT
```

### 4. Si vous utilisez un nom de domaine

Si vous avez configuré un nom de domaine pointant vers votre serveur :

```
http://votre-domaine.com:8080
```

## Ports par Défaut

- **Port 8888** : Serveur de marché (connexions traders)
- **Port 8080** : Interface web

Vous pouvez changer ces ports lors du lancement :

```bash
./build/market_simulation 8888 8080
```

## Vérification

### Vérifier que le serveur écoute

```bash
# Sur le serveur
netstat -tlnp | grep 8080
# ou
ss -tlnp | grep 8080
```

Vous devriez voir :
```
tcp  0  0  0.0.0.0:8080  0.0.0.0:*  LISTEN
```

### Tester la connexion

```bash
# Depuis votre ordinateur local
curl http://VOTRE_IP_SERVEUR:8080
```

## Problèmes Courants

### Le port est fermé

```bash
# Vérifier le firewall
sudo ufw status
sudo ufw allow 8080/tcp
```

### Le serveur n'écoute que sur localhost

Par défaut, le serveur écoute sur `0.0.0.0` (toutes les interfaces). Si ce n'est pas le cas, vérifiez le code de `WebServer.cpp`.

### Connexion refusée

1. Vérifiez que le serveur est bien lancé
2. Vérifiez que le port 8080 n'est pas utilisé par un autre service
3. Vérifiez les règles du firewall

## Utilisation avec Nginx (Optionnel)

Pour un accès plus professionnel, vous pouvez configurer Nginx comme reverse proxy :

```nginx
server {
    listen 80;
    server_name votre-domaine.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

Puis accédez via :
```
http://votre-domaine.com
```

