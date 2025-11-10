#!/bin/bash
echo "=== Configuration SSH pour GitHub ==="
echo ""

# Générer une clé SSH si elle n'existe pas
if [ ! -f ~/.ssh/id_ed25519.pub ]; then
    echo "Création d'une nouvelle clé SSH..."
    ssh-keygen -t ed25519 -C "github" -f ~/.ssh/id_ed25519 -N ""
    echo ""
fi

# Afficher la clé publique
echo "Votre clé SSH publique :"
echo "----------------------------------------"
cat ~/.ssh/id_ed25519.pub
echo "----------------------------------------"
echo ""
echo "Étapes suivantes :"
echo "1. Copiez la clé ci-dessus"
echo "2. Allez sur https://github.com/settings/ssh/new"
echo "3. Collez la clé et sauvegardez"
echo "4. Ensuite, exécutez :"
echo "   git remote set-url origin git@github.com:mat-rambo/market_simu.git"
echo "   git push -u origin master"
