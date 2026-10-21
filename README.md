# 🛡️ SafeMind — Application Mobile de Sécurité pour Personnes Atteintes d'Alzheimer

**SafeMind** est une application mobile Expo / React Native haute performance conçue pour les aidants et proches de personnes atteintes de la maladie d'Alzheimer. Connectée à un bracelet intelligent (ESP8266 + capteur optique MAX30102 + module GPS), elle assure un suivi bienveillant en temps réel : rythme cardiaque, géorepérage dynamique avec seuils d'hystérésis, détection de retrait de bracelet et alertes critiques immédiates.

---

## 🎨 Identité Visuelle & Design

L'application adopte un style **Dark Glassmorphism Vert Néon** ultra-soigné :
- **Fond d'écran** : `#050D08` avec halo radial supérieur vert émeraude `#12532A`.
- **Cartes & Conteneurs** : Surfaces sombres semi-transparentes `#0D1A12` / `#122319` avec bordures néon `rgba(109, 245, 138, 0.18)` et coins arrondis (20–24 px).
- **Carte Hero (Accueil)** :
  - Dégradé vert clair vibrant (`#7DFF9B` → `#B9FFCC` → `#5BE481`) en situation normale avec texte sombre.
  - Dégradé corail/rouge dynamique (`#FF7B7F` → `#FFA5A9` → `#FFC1C3`) lors d'une **Sortie de zone** ou d'un **BPM critique**.
- **Animations Reanimated** :
  - Cœur qui bat en temps réel au rythme exact du BPM mesuré.
  - Halo radar pulsant sur le marqueur du patient et le logo.
  - Bannière de notification in-app en verre dépoli (`BlurView`) qui glisse du haut.
  - Barre de navigation flottante avec pilule active en vert vif.

---

## 📱 Écrans et Parcours Utilisateur

1. **Onboarding Interactif (3 écrans)** *(au 1er lancement ou via "Revoir l'introduction")* :
   - **Écran 1 · Welcome** : Logo officiel SafeMind animé (bouclier en verre, tracé du pin GPS, cœur battant, ondes concentriques radar), accroche *"Veiller sur ceux qui comptent"*, retour haptique au toucher du logo, bouton *"Commencer"* ou *"Passer l'introduction"*.
   - **Écran 2 · Utilité** : Carrousel interactif de 3 cartes avec simulations concrètes :
     - *Géorepérage* : curseur pour éloigner le patient et voir le cercle passer du vert au rouge.
     - *Rythme cardiaque* : tracé ECG SVG avec bouton de simulation de tachycardie (142 BPM).
     - *Alertes instantanées* : déclenchement d'une alerte test flottante.
   - **Écran 3 · Instructions pas à pas** : Stepper interactif en 4 étapes avec validation (port du bracelet, allumage Wi-Fi, réglage de la zone, autorisation des notifications et test sonore).
2. **Connexion & Appairage (`app/login.tsx`)** :
   - Saisie du topic sécurisé du bracelet (ex: `safemind-sameh-01`).
   - Stockage chiffré dans `expo-secure-store`.
   - Bouton d'accès immédiat au **Mode Démo**.
3. **Accueil (`app/(tabs)/index.tsx`)** :
   - Salutation personnalisée à l'aidant et avatar.
   - Carte Hero BPM en direct + état de zone + distance au domicile.
   - Mini-carte stylisée de la position avec repère maison et patient.
   - Indicateurs d'état : niveau de batterie, détection de contact peau (*Bracelet porté / retiré*), signal en ligne.
   - Graphique de tendance ECG en direct.
   - Panneau de simulation interactive (distances rapides 60m, 140m, 203m, 310m et bascule BPM anormal).
4. **Carte Satellite Plein Écran (`app/(tabs)/map.tsx`)** :
   - Vue satellite / hybride haute définition.
   - Cercle de géorepérage en surbrillance (vert en zone, rouge translucide hors zone).
   - Marqueur maison 🏡 et avatar patient avec pastille verte en ligne.
   - Boutons flottants : bascule satellite/standard, recentrage automatique, bouton **Itinéraire GPS** (ouvre Google Maps ou Apple Maps), bouton **Appel d'urgence** direct.
5. **Historique des Alertes (`app/(tabs)/alerts.tsx`)** :
   - Liste horodatée des événements (Sortie de zone, Retour en zone, Tachycardie, Bracelet retiré, SOS).
   - Code couleur d'urgence (rouge pour alertes critiques, jaune pour avertissements, vert pour info).
   - Récapitulatif des seuils configurés.
6. **Réglages (`app/(tabs)/settings.tsx`)** :
   - Noms du patient et de l'aidant.
   - Rayons d'hystérésis (Sortie > 200 m / Retour < 180 m).
   - Seuils de fréquence cardiaque (min 45 / max 120 BPM).
   - Numéro d'urgence principal.
   - Bascule du Mode Démonstration et des notifications.
   - Bouton *"Revoir l'introduction"* pour relancer l'onboarding.
   - Déconnexion / Changement de bracelet.

---

## 🚀 Démarrage Rapide

### Prérequis
- [Node.js](https://nodejs.org/) (v18 ou supérieur)
- Application mobile **Expo Go** sur votre smartphone iOS/Android, ou un émulateur.

### Installation
```bash
# 1. Cloner ou naviguer dans le dossier du projet
cd mobileWearable

# 2. Installer les dépendances
npm install

# 3. Lancer le serveur de développement Expo
npx expo start
```

Scannez le QR Code affiché dans votre terminal avec l'appareil photo (iOS) ou l'application **Expo Go** (Android).

---

## 🧪 Mode Démonstration (Sans Matériel)

L'application est **immédiatement utilisable sans bracelet physique** grâce à son simulateur intégré :
1. Sur l'écran de connexion, appuyez sur **"Lancer le mode démo (sans bracelet)"**.
2. Sur l'écran d'accueil :
   - Utilisez les boutons de distance (`60 m`, `140 m`, `203 m`, `310 m`) pour observer le passage automatique en **Hors zone** (> 200 m) avec déclenchement de notification et passage de la carte en rouge.
   - Appuyez sur **"Simuler un BPM anormal"** pour simuler une crise cardiaque (> 120 BPM) : le tracé ECG passe en rouge et l'alerte retentit.
   - Appuyez sur le badge **"Bracelet porté"** pour simuler un retrait du bracelet par le patient.

---

## 🔌 Raccordement d'un Vrai Bracelet Connecté

Le bracelet transmet ses données via le serveur public ou privé [ntfy.sh](https://ntfy.sh) en HTTPS sans configuration de routeur complexe.

### 1. Architecture des Topics ntfy.sh
Pour un code bracelet `safemind-monpatient` :
- **Topic Télémétrie** : `safemind-monpatient-live`
  - Envoyé toutes les 5 à 10 secondes.
  - Format JSON :
    ```json
    {
      "bpm": 74,
      "lat": 36.8065,
      "lon": 10.1815,
      "skin": 1,
      "bat": 87,
      "ts": 1760000000
    }
    ```
- **Topic Alertes Prioritaires** : `safemind-monpatient`
  - Envoyé instantanément lors d'un événement avec priorité HTTP :
    - `Title`: `SORTIE DE ZONE` | `BPM anormal : 135` | `BRACELET RETIRE` | `SOS - Appel a l'aide`
    - `Priority`: `urgent` (5) ou `high` (4)
    - `Message`: Description lisible pour l'aidant

### 2. Exemple de Code ESP8266 (Arduino C++)

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "VOTRE_WIFI";
const char* password = "VOTRE_MOT_DE_PASSE";
const char* topic = "safemind-sameh-01"; // Topic choisi

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// Envoi de la télémétrie périodique (toutes les 5 secondes)
void sendTelemetry(int bpm, float lat, float lon, int skinContact, int battery) {
  WiFiClientSecure client;
  client.setInsecure(); // Accepte le certificat SSL ntfy.sh
  HTTPClient http;
  
  String url = String("https://ntfy.sh/") + topic + "-live";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"bpm\":" + String(bpm) + 
                   ",\"lat\":" + String(lat, 6) + 
                   ",\"lon\":" + String(lon, 6) + 
                   ",\"skin\":" + String(skinContact) + 
                   ",\"bat\":" + String(battery) + 
                   ",\"ts\":" + String(millis() / 1000) + "}";
                   
  http.POST(payload);
  http.end();
}

// Envoi d'une alerte critique (ex: Chute ou SOS)
void sendUrgentAlert(String alertTitle, String alertMessage) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  String url = String("https://ntfy.sh/") + topic;
  http.begin(client, url);
  http.addHeader("Title", alertTitle);
  http.addHeader("Priority", "urgent");
  http.addHeader("Tags", "warning,rotating_light");
  
  http.POST(alertMessage);
  http.end();
}

void loop() {
  // Lire capteur MAX30102 et GPS NEO-6M
  int currentBpm = 74; // Remplacer par lecture capteur
  float currentLat = 36.8065;
  float currentLon = 10.1815;
  int skin = 1; // 1 si doigt/poignet détecté
  int battery = 85;

  sendTelemetry(currentBpm, currentLat, currentLon, skin, battery);
  delay(5000);
}
```

---

## 🗺️ Configuration des Clés Google Maps (Android)

Sur iOS, l'application utilise nativement **Apple Maps** sans aucune clé requise.
Sur Android, pour une utilisation en build natif autonome (EAS Build / APK), configurez votre clé Google Maps SDK dans `app.json` :

1. Obtenez une clé API sur la [Google Cloud Console](https://console.cloud.google.com/) avec l'API **Maps SDK for Android** activée.
2. Ajoutez votre clé dans [app.json](file:///c:/Users/amani/Desktop/5SLEAM/mobileWearable/app.json) :
```json
"android": {
  "config": {
    "googleMaps": {
      "apiKey": "VOTRE_CLE_GOOGLE_MAPS_ICI"
    }
  }
}
```

---

## 🛠️ Structure du Projet

```
mobileWearable/
├── app/
│   ├── _layout.tsx              # Racine Expo Router, canaux de notifications
│   ├── index.tsx                # Redirection intelligente (onboarding / login / tabs)
│   ├── login.tsx                # Connexion, saisie du topic et mode démo
│   ├── onboarding/
│   │   ├── _layout.tsx
│   │   ├── welcome.tsx          # Logo animé, devise, bouton commencer
│   │   ├── benefits.tsx         # Carrousel interactif (GPS, ECG, alertes)
│   │   └── instructions.tsx     # Stepper en 4 étapes et permissions
│   └── (tabs)/
│       ├── _layout.tsx          # Tab bar flottante en verre dépoli
│       ├── index.tsx            # Écran Accueil (HeroCard, télémétrie, simulation)
│       ├── map.tsx              # Carte satellite plein écran & géorepérage
│       ├── alerts.tsx           # Historique des alertes & seuils
│       └── settings.tsx         # Paramétrage, hystérésis, replay onboarding
├── src/
│   ├── components/
│   │   ├── AnimatedLogo.tsx     # Bouclier SVG, radar, cœur battant
│   │   ├── HeroCard.tsx         # Carte BPM dynamique (vert néon / corail danger)
│   │   ├── BpmChart.tsx         # Tracé d'onde ECG SVG
│   │   ├── GlassCard.tsx        # Composant carte Dark Glassmorphism
│   │   ├── AlertItem.tsx        # Élément de liste d'alerte avec icône carrée
│   │   ├── PatientMarker.tsx    # Marqueur avatar avec halo pulsé et pastille verte
│   │   ├── InAppNotificationBanner.tsx # Bannière d'alerte flottante en verre
│   │   ├── MiniMap.tsx          # Vue radar miniature urbaine stylisée
│   │   ├── PrimaryButton.tsx    # Bouton tactile avec haptique et spring
│   │   ├── PaginationDots.tsx   # Indicateur de pagination animé
│   │   └── StepIndicator.tsx    # Stepper interactif des étapes
│   ├── services/
│   │   ├── ntfyClient.ts        # Client polling ntfy.sh, parsing JSON, hystérésis
│   │   └── notifications.ts     # expo-notifications & expo-haptics
│   ├── store/
│   │   └── useStore.ts          # Zustand store central avec persistance
│   ├── theme/
│   │   ├── colors.ts            # Palette néon dark (#050D08, #6DF58A, #FF5A5F)
│   │   └── typography.ts        # Hiérarchie typographique
│   └── utils/
│       ├── haversine.ts         # Formule mathématique de Haversine
│       └── formatters.ts        # Formatage des distances et dates
├── assets/                      # Icônes et images SafeMind
└── app.json                     # Configuration Expo & plugins
```

---

## 🔒 Sécurité et Confidentialité

- Aucun mot de passe ni identifiant n'est codé en dur.
- Le topic ntfy.sh est chiffré dans l'enclave sécurisée de l'appareil (`expo-secure-store`).
- Les données de santé et de positionnement restent la propriété de la famille.
