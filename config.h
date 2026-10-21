#pragma once

// ============ WiFi ============
#define WIFI_SSID   "foyer2_1"
#define WIFI_PASS   "11111111"


// ============ Alertes ntfy ============
// Choisis un nom de topic UNIQUE et difficile à deviner.
// L'aidant s'abonne à ce topic dans l'appli ntfy (Android/iOS/PC).
#define NTFY_TOPIC  "bracelet-sameh-2026-x7k9"

// ============ Identité ============
#define PATIENT_NAME     "Sameh"
#define CAREGIVER_NAME   "AbedElHamid"
#define CAREGIVER_PHONE  "+216 29248524"

// ============ Broches ============
#define PIN_BUZZER   D7
#define PIN_BUTTON   D3
#define PIN_GPS_RX   D5   // GPIO14 : recoit le TX du GPS
#define PIN_GPS_TX   D6   // GPIO12 : envoie vers le RX du GPS
// I2C : D2 = SDA, D1 = SCL (defaut NodeMCU)
// ============ Fuseau horaire ============
#define TZ_OFFSET_H  1        // Tunisie = UTC+1 toute l'annee
// ============ Georeperage ============
#define HOME_LAT       36.806500
#define HOME_LON       10.181500
#define RAYON_SORTIE   200.0     // metres : au-dela, alerte
#define RAYON_RETOUR   180.0     // metres : en-dessous, retour OK
#define GEOFENCE_CONFIRM_N  3    // nb de mesures consecutives hors zone
#define REALERT_MS     300000    // rappel toutes les 5 min si toujours dehors

// ============ Mode GPS ============
// 0 = GPS matériel réel (satellites)
// 1 = Simulation AU DOMICILE (dans la zone, distance = 0 m)
// 2 = Simulation HORS ZONE (en dehors de la zone, distance ≈ 290 m, déclenche l'alarme)
#define GPS_MODE       2         // Activé en mode HORS ZONE pour votre test !
// ============ Bouton ============
#define BTN_DEBOUNCE_MS   50      // anti-rebond
#define BTN_SOS_HOLD_MS   1500    // appui long = SOS (1.5 s)
#define BTN_ID_SHOW_MS    30000   // duree d'affichage de la carte ID

#define SOS_REALERT_MS    60000   // rappel SOS toutes les 60 s tant que appui
// ============ MAX30102 / Pouls ============
#define BPM_MIN              45        // en dessous : alerte (bradycardie)
#define BPM_MAX              120       // au dessus : alerte (tachycardie)
#define BPM_BAD_DELAY_MS     20000     // 20 s de BPM anormal avant alerte
#define REMOVED_DELAY_MS     10000     // 10 s sans peau avant alerte retrait
#define IR_SKIN_THRESHOLD    50000     // seuil IR : peau detectee si > cette valeur (doigt reel = 60000-150000)
#define BPM_AVG_SIZE         4         // moyenne glissante sur N mesures

// ============ Supabase ============
#define SUPABASE_URL          "https://qnvfclydulzujylnmdyr.supabase.co"
#define SUPABASE_ANON_KEY     "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InFudmZjbHlkdWx6dWp5bG5tZHlyIiwicm9sZSI6ImFub24iLCJpYXQiOjE3OTI1NzIzMTgsImV4cCI6MjEwODE0ODMxOH0.QSv-25Km_8dBuYboIb8ml6Z0ChUW7ve0JLRi_CRPqTM"
#define SUPABASE_DEVICE_ID    "f7f4a04d-73f1-475b-b3b2-977bf3e370af"
#define SUPABASE_INTERVAL_MS  5000      // envoi de telemetrie toutes les 5 s (laisse le CPU libre pour le capteur BPM)
