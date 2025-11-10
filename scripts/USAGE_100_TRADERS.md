# Utilisation du script 100 traders

## Utilisation de base (localhost)

```bash
python3 scripts/simulate_100_traders.py
```

Cela lancera 100 traders sur `localhost:8888` pendant 60 secondes.

## Options disponibles

```bash
python3 scripts/simulate_100_traders.py --help
```

### Exemples

**Sur votre machine locale :**
```bash
python3 scripts/simulate_100_traders.py
# ou explicitement
python3 scripts/simulate_100_traders.py --host localhost --port 8888
```

**Sur un serveur distant (VPS) :**
```bash
python3 scripts/simulate_100_traders.py --host 51.77.193.65 --port 8888
```

**Avec moins de traders et une durée plus courte (pour tester) :**
```bash
python3 scripts/simulate_100_traders.py --traders 10 --duration 30
```

**Avec plus de traders et une durée plus longue :**
```bash
python3 scripts/simulate_100_traders.py --traders 200 --duration 300 --workers 50
```

## Paramètres

- `--host` : IP ou hostname du serveur (défaut: `localhost`)
- `--port` : Port du serveur de marché (défaut: `8888`)
- `--traders` : Nombre de traders (défaut: `100`)
- `--duration` : Durée en secondes (défaut: `60`)
- `--interval` : Intervalle entre les ordres par trader en secondes (défaut: `2.0`)
- `--workers` : Nombre maximum de connexions concurrentes (défaut: `20`)

## Notes

- Le script se connecte au serveur, enregistre chaque trader, puis soumet des ordres continuellement
- Chaque trader soumet un ordre toutes les 2 secondes par défaut
- Les ordres sont aléatoires (BUY/SELL, différents symboles, prix variés)
- Le script affiche un résumé à la fin avec les statistiques
