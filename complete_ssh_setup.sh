#!/bin/bash
# Script pour compléter la configuration SSH après avoir ajouté la clé sur GitHub

echo "=== Configuration SSH pour GitHub ==="
echo ""

# Vérifier si la clé SSH existe
if [ ! -f ~/.ssh/id_ed25519.pub ]; then
    echo "❌ Clé SSH non trouvée. Exécutez d'abord: ./setup_ssh_push.sh"
    exit 1
fi

echo "1. Test de la connexion SSH à GitHub..."
ssh -T git@github.com 2>&1 | head -3

echo ""
echo "2. Changement de l'URL du remote vers SSH..."
git remote set-url origin git@github.com:mat-rambo/market_simu.git

echo "✅ URL changée vers SSH"
echo ""

echo "3. Vérification de la configuration..."
git remote -v

echo ""
echo "4. Prêt à pousser ! Exécutez :"
echo "   git push -u origin master"
echo ""
echo "Ou voulez-vous que je pousse maintenant ? (y/n)"
read -r response
if [[ "$response" =~ ^[Yy]$ ]]; then
    echo ""
    echo "Poussage vers GitHub..."
    git push -u origin master
else
    echo "Vous pouvez pousser plus tard avec: git push -u origin master"
fi

