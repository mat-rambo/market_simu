#!/bin/bash
# Script pour déboguer le crash avec gdb

export PGPASSWORD=postgres

echo "🔍 Démarrage du débogage avec gdb..."
echo ""
echo "Commandes gdb utiles :"
echo "  - file ./build/market_simulation  (charger l'exécutable)"
echo "  - run 8888 8080                   (lancer le programme)"
echo "  - bt                              (afficher la stack trace après crash)"
echo "  - info registers                  (afficher les registres)"
echo "  - quit                            (quitter gdb)"
echo ""
echo "Lancement de gdb..."
echo ""

gdb ./build/market_simulation
