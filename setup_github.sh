#!/bin/bash
# Script complet pour installer GitHub CLI et créer le dépôt

set -e

echo "=== Configuration GitHub CLI ==="
echo ""

# Étape 1: Installer GitHub CLI
if ! command -v gh &> /dev/null; then
    echo "📦 Installation de GitHub CLI..."
    sudo apt-get update
    sudo apt-get install -y gh
    echo "✅ GitHub CLI installé"
else
    echo "✅ GitHub CLI déjà installé"
fi

echo ""

# Étape 2: Vérifier l'authentification
if ! gh auth status &> /dev/null; then
    echo "🔐 Authentification GitHub requise..."
    echo ""
    echo "Choisissez votre méthode d'authentification :"
    echo "  1. GitHub.com (recommandé)"
    echo "  2. GitHub Enterprise Server"
    echo ""
    echo "Ensuite, choisissez :"
    echo "  1. Login with a web browser"
    echo "  2. Paste an authentication token"
    echo ""
    gh auth login
else
    echo "✅ Déjà authentifié avec GitHub"
    gh auth status
fi

echo ""

# Étape 3: Créer le dépôt et pousser
echo "🚀 Création du dépôt et push du code..."
echo ""

# Vérifier si le remote existe déjà
if git remote get-url origin &> /dev/null; then
    echo "Remote 'origin' existe déjà"
    git remote -v
    echo ""
    read -p "Voulez-vous continuer ? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Annulé"
        exit 1
    fi
fi

# Créer le dépôt
gh repo create market_simu --public --source=. --remote=origin --push

echo ""
echo "✅ Dépôt créé et code poussé avec succès!"
echo ""
echo "Votre dépôt est disponible sur :"
echo "  https://github.com/mat-rambo/market_simu"

