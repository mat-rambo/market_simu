#!/bin/bash
# Script pour créer le dépôt GitHub et pousser le code

echo "=== Création du dépôt GitHub et push ==="
echo ""

# Vérifier si GitHub CLI est installé
if command -v gh &> /dev/null; then
    echo "✅ GitHub CLI trouvé"
    echo ""
    echo "Création du dépôt sur GitHub..."
    gh repo create market_simu --public --source=. --remote=origin --push
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Dépôt créé et code poussé avec succès!"
        exit 0
    else
        echo ""
        echo "❌ Erreur lors de la création. Vérifiez que vous êtes connecté: gh auth login"
        exit 1
    fi
else
    echo "⚠️  GitHub CLI non installé"
    echo ""
    echo "Options :"
    echo ""
    echo "1. Installer GitHub CLI et utiliser ce script :"
    echo "   sudo apt-get install gh"
    echo "   gh auth login"
    echo "   ./create_and_push.sh"
    echo ""
    echo "2. Créer manuellement sur GitHub :"
    echo "   👉 https://github.com/new"
    echo "   - Nom: market_simu"
    echo "   - NE PAS initialiser avec README/gitignore"
    echo "   - Créer le dépôt vide"
    echo ""
    echo "   Puis exécutez :"
    echo "   git push -u origin master"
    echo ""
    exit 1
fi

