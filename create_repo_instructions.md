# Le dépôt n'existe pas encore sur GitHub

## Solution : Créer le dépôt sur GitHub

### Option 1 : Via l'interface web GitHub (Recommandé)

1. **Allez sur GitHub** : https://github.com/new

2. **Remplissez le formulaire** :
   - Repository name: `market_simu`
   - Description: "Market simulation server in C++"
   - Visibilité: Public ou Private (selon votre choix)
   - **NE COCHEZ PAS** "Initialize this repository with a README"
   - **NE COCHEZ PAS** "Add .gitignore"
   - **NE COCHEZ PAS** "Choose a license"
   - Cliquez sur "Create repository"

3. **Une fois créé, GitHub vous donnera des instructions. IGNOREZ-LES** et utilisez les commandes ci-dessous :

```bash
# L'URL SSH devrait être correcte maintenant
git remote set-url origin git@github.com:mat-rambo/market_simu.git

# Vérifier
git remote -v

# Pousser
git push -u origin master
```

### Option 2 : Via GitHub CLI (si installé)

```bash
# Installer GitHub CLI si nécessaire
sudo apt-get install gh

# Se connecter
gh auth login

# Créer le dépôt
gh repo create market_simu --public --source=. --remote=origin --push
```

### Option 3 : Vérifier si le nom du dépôt est différent

Si le dépôt existe mais avec un nom différent, mettez à jour l'URL :

```bash
# Si le dépôt s'appelle différemment, par exemple "market-simulation"
git remote set-url origin git@github.com:mat-rambo/market-simulation.git
git push -u origin master
```

## Après avoir créé le dépôt

Exécutez ces commandes :

```bash
cd /home/rambo/Documents/market_simu
git remote set-url origin git@github.com:mat-rambo/market_simu.git
git push -u origin master
```

