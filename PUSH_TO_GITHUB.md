# Guide pour pousser vers GitHub

## Problème d'authentification

Git essaie d'utiliser HTTPS mais ne peut pas obtenir vos identifiants. Voici plusieurs solutions :

## Solution 1 : Utiliser SSH (Recommandé)

### 1. Vérifier si vous avez une clé SSH
```bash
ls -la ~/.ssh/id_*.pub
```

### 2. Si vous n'avez pas de clé SSH, en créer une :
```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
# Appuyez sur Entrée pour accepter l'emplacement par défaut
# Entrez un mot de passe (optionnel)
```

### 3. Ajouter la clé SSH à votre compte GitHub
```bash
# Afficher la clé publique
cat ~/.ssh/id_ed25519.pub
# Copiez le contenu et ajoutez-le dans GitHub :
# Settings > SSH and GPG keys > New SSH key
```

### 4. Changer l'URL du remote vers SSH
```bash
cd /home/rambo/Documents/market_simu
git remote set-url origin git@github.com:mat-rambo/market_simu.git
git push -u origin master
```

## Solution 2 : Utiliser un Personal Access Token (HTTPS)

### 1. Créer un token sur GitHub
- Allez sur GitHub.com > Settings > Developer settings > Personal access tokens > Tokens (classic)
- Cliquez sur "Generate new token (classic)"
- Donnez un nom (ex: "market_simu")
- Sélectionnez les permissions : `repo` (toutes)
- Cliquez sur "Generate token"
- **COPIEZ LE TOKEN** (vous ne pourrez plus le voir après)

### 2. Utiliser le token pour pousser
```bash
cd /home/rambo/Documents/market_simu
git push -u origin master
# Username: votre_username_github
# Password: collez_le_token_ici (pas votre mot de passe GitHub)
```

### 3. Ou configurer le credential helper (pour éviter de retaper)
```bash
git config --global credential.helper store
git push -u origin master
# Entrez username et token une fois, ils seront sauvegardés
```

## Solution 3 : Utiliser GitHub CLI

```bash
# Installer GitHub CLI
sudo apt-get install gh

# Se connecter
gh auth login

# Pousser
git push -u origin master
```

## Vérification

Après avoir configuré l'authentification, testez :
```bash
git push -u origin master
```

Si ça fonctionne, vous verrez :
```
Enumerating objects: X, done.
Counting objects: 100% (X/X), done.
...
To https://github.com/mat-rambo/market_simu.git
 * [new branch]      master -> master
Branch 'master' set up to track remote branch 'master' from 'origin'.
```

## Si le dépôt distant utilise "main" au lieu de "master"

```bash
# Vérifier la branche par défaut sur GitHub
# Si c'est "main", vous pouvez soit :

# Option A : Renommer votre branche locale
git branch -m master main
git push -u origin main

# Option B : Pousser vers master et changer la branche par défaut sur GitHub
git push -u origin master
# Puis sur GitHub : Settings > Branches > Default branch > changer vers "master"
```

