# Bosconnect - Système Domotique Intelligent

![Logo Bosconnect](docs/logo_bosconnect.png)

## Présentation du projet

Bosconnect est un projet de domotique visant à concevoir un système connecté polyvalent pour la surveillance et le contrôle de l'environnement domestique. Développé dans le cadre d'un projet académique, il propose une solution complète combinant matériel personnalisé et interface web ergonomique.

## Objectifs

1. **Créer une centrale domotique accessible** permettant à tout utilisateur de surveiller et contrôler son habitat
2. **Développer une solution intégrée** associant capteurs environnementaux et contrôle d'appareils
3. **Offrir une interface intuitive** à la fois sur le dispositif physique et à distance via le web
4. **Permettre l'automatisation de tâches** basée sur des conditions temporelles ou environnementales

## Fonctionnement du système

Le système Bosconnect est basé sur deux composants fondamentaux qui fonctionnent en parfaite synergie :

### 1. Module matériel ESP32

Au cœur du système se trouve une carte électronique basée sur l'ESP32 qui :
- **Collecte des données environnementales** via plusieurs capteurs (température, pression, luminosité, gaz)
- **Contrôle des appareils électriques** grâce à des relais intégrés et un variateur
- **Gère l'accès** par un système RFID pour la sécurité
- **Offre une interface locale** par écran tactile TFT
- **Communique avec le cloud** via connexion Wi-Fi

![Schéma de la carte ESP32](docs/esp32_diagram.png)

### 2. Plateforme cloud Firebase

Le système cloud complète l'appareil physique en :
- **Stockant les données** des capteurs en temps réel
- **Permettant le contrôle à distance** des appareils connectés
- **Visualisant les tendances** par des graphiques d'historique
- **Gérant les automatisations** temporelles ou conditionnelles
- **Sécurisant l'accès** par authentification utilisateur

![Interface web](docs/web_interface.png)

## Architecture fonctionnelle

Le fonctionnement de Bosconnect repose sur cette architecture bidirectionnelle :

1. **Acquisition de données** : Les capteurs mesurent en continu les paramètres environnementaux
2. **Traitement local** : L'ESP32 analyse ces informations et peut réagir selon des paramètres prédéfinis
3. **Synchronisation cloud** : Les données sont envoyées à Firebase pour stockage et analyse
4. **Visualisation** : L'utilisateur consulte les informations via l'écran TFT local ou l'interface web
5. **Contrôle** : L'utilisateur ou les automatisations peuvent déclencher des actions sur les relais et le variateur
6. **Sécurité** : Le système RFID permet de limiter l'accès physique au dispositif

## Caractéristiques techniques principales

- **Capteurs** : BMP280 (température/pression), BH1750 (luminosité), MQ9 (gaz)
- **Actionneurs** : 2 relais doubles, 1 variateur de puissance, 1 relais pour contrôle d'accès RFID
- **Interface locale** : Écran tactile TFT
- **Connectivité** : Wi-Fi pour connexion au cloud
- **Plateforme cloud** : Firebase (authentification, base de données en temps réel)

## Application pratique

Bosconnect peut être utilisé pour diverses applications domotiques :
- Surveillance de la qualité de l'air intérieur
- Contrôle d'éclairage intelligent (on/off et variation)
- Automatisation d'appareils électriques selon des planifications temporelles
- Sécurisation d'accès par technologie RFID
- Création de scénarios conditionnels (si température > X alors action Y)

![Interface TFT](docs/tft_interface.png)

---

© 2024 BOSCONNECT. Développé par Bosco.
