/*
  Bracelet Alzheimer - ETAPE 5 : Pouls + retrait bracelet
*/
#include "config.h"
#include "types.h"
#include "net.h"
#include "gps.h"
#include "geofence.h"
#include "alerts.h"
#include "buzzer.h"
#include "button.h"
#include "pulse.h"
#include "display.h"
#include "supabase.h"
#include "stress_model.h"
#include <ESP8266WiFi.h>


static unsigned long idCardUntil = 0;
static unsigned long sosUntil = 0;
static bool removedAlerted = false;
static unsigned long lastBpmAlert = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Bracelet - Etape 5 (Pouls) ===");

  pinMode(LED_BUILTIN, OUTPUT);

  buzzer_init();
  button_init();
  display_init();
  display_boot("Bracelet", "Demarrage...");
  delay(600);

  bool netOk = net_init();
  display_boot(netOk ? "WiFi OK" : "WiFi ECHEC",
               netOk ? WiFi.localIP().toString() : "Verifie config.h");
  delay(1200);

  // Initialisation client Supabase
  Supabase.begin(SUPABASE_URL, SUPABASE_ANON_KEY, SUPABASE_DEVICE_ID);


  gps_init();
  gps_set_raw(false);
  geofence_init();
  alerts_init();
  pulse_init();

  if (netOk) {
    alerts_send(A_BOOT, "Bracelet " + String(PATIENT_NAME) + " demarre",
                "Zone surveillee : " + String(RAYON_SORTIE, 0) + " m", 0);
  }

  display_boot("Zone surveillee", String(RAYON_SORTIE, 0) + " m");
  delay(1200);
}

void loop() {
  gps_update();
  geofence_update();
  pulse_update();
  alerts_update();
  buzzer_update();
  button_update();
  net_update();

  // ---- Evenements geofence ----
  String reason;
  if (geofence_pop_event(reason)) {
    double d = geofence_dist();
    if (reason == "first") {
      alerts_send(A_OUT, "SORTIE DE ZONE",
                  "Distance : " + String(d, 0) + " m", 2);
      Serial.printf("!! SORTIE DE ZONE (%0.f m)\n", d);
    } else if (reason == "repeat") {
      alerts_send(A_OUT, "TOUJOURS HORS ZONE",
                  "Distance : " + String(d, 0) + " m", 2);
      Serial.printf("!! TOUJOURS HORS ZONE (%0.f m)\n", d);
    } else if (reason == "back") {
      alerts_send(A_BACK, "Retour en zone",
                  "Distance : " + String(d, 0) + " m", 0);
      Serial.printf("OK Retour en zone (%0.f m)\n", d);
    }
  }

  // ---- Evenements bouton ----
  ButtonEvent ev = button_pop_event();
  if (ev == BTN_SHORT_PRESS) {
    if (millis() < idCardUntil) {
      idCardUntil = 0; // Referme si deja ouvert
      Serial.println(">> Clic court : fermeture carte identite");
    } else {
      idCardUntil = millis() + 15000; // Affiche 'Je suis Sameh' pendant 15s
      Serial.println(">> Clic court : affichage 'Je suis " + String(PATIENT_NAME) + "'");
    }
  } else if (ev == BTN_LONG_PRESS) {
    sosUntil = millis() + 15000;
    idCardUntil = 0;
    alerts_sos();
    Supabase.sendTelemetry(
      pulse_bpm(),
      gps_fix_ok() ? (float)gps_lat() : 0.0f,
      gps_fix_ok() ? (float)gps_lon() : 0.0f,
      gps_fix_ok(),
      pulse_skin_on() ? 1 : 0,
      100,
      pulse_removed(),
      pulse_bpm_bad(),
      (float)geofence_dist(),
      geofence_is_outside(),
      true
    );
    Serial.println(">> Appui long : SOS declenche vers Supabase et ntfy !");
  }


  // ---- Alerte retrait bracelet ----
  if (pulse_removed() && !removedAlerted) {
    removedAlerted = true;
    String detail = "URGENT : " + String(PATIENT_NAME) +
                    " a enleve son bracelet.\n" +
                    "Verifiez qu'il/elle va bien.\n" +
                    "Appelez : " + String(CAREGIVER_PHONE);
    alerts_send(A_REMOVED, "BRACELET RETIRE", detail, 2);
    Serial.println("!! Bracelet retire - alerte envoyee");
  }
  if (!pulse_removed()) removedAlerted = false;

  // ---- Alerte BPM anormal ----
  if (pulse_bpm_bad() && millis() - lastBpmAlert > 300000) {
    lastBpmAlert = millis();
    alerts_send(A_BPM, "BPM anormal : " + String(pulse_bpm()),
                pulse_bpm() < BPM_MIN ? "Rythme trop lent" : "Rythme trop rapide", 2);
    Serial.printf("!! BPM anormal : %d\n", pulse_bpm());
  }

  // ---- Alerte Stress eleve (modele WESAD) ----
  static unsigned long lastStressAlert = 0;
  if (pulse_is_stressed() && millis() - lastStressAlert > 300000) {
    lastStressAlert = millis();
    int pct = (int)(pulse_stress_proba() * 100.0f);
    alerts_send(A_STRESS, "Stress eleve detecte",
                "Indice de stress WESAD : " + String(pct) + " %", 1);
    Serial.printf("!! Stress eleve : %d%%\n", pct);
  }

  // ---- Envoi telemetrie vers Supabase (dynamique et temps reel) ----
  static unsigned long lastSupabaseSend = 0;
  static int lastSentBpm = -999;
  static bool lastSentSkin = false;

  int curBpm = pulse_bpm();
  bool curSkin = pulse_skin_on();
  unsigned long elapsedSinceSend = millis() - lastSupabaseSend;

  // Envoi toutes les 5s (SUPABASE_INTERVAL_MS), OU des que contact peau ou BPM change
  bool needSend = (elapsedSinceSend >= SUPABASE_INTERVAL_MS);
  if (curSkin != lastSentSkin && elapsedSinceSend >= 2000) needSend = true;
  if (curSkin && curBpm > 0 && abs(curBpm - lastSentBpm) >= 4 && elapsedSinceSend >= 3500) needSend = true;

  if (needSend) {
    lastSupabaseSend = millis();
    lastSentBpm = curBpm;
    lastSentSkin = curSkin;

    Supabase.sendTelemetry(
      curBpm,
      gps_fix_ok() ? (float)gps_lat() : 0.0f,
      gps_fix_ok() ? (float)gps_lon() : 0.0f,
      gps_fix_ok(),
      curSkin ? 1 : 0,
      100, // Batterie %
      pulse_removed(),
      pulse_bpm_bad(),
      (float)geofence_dist(),
      geofence_is_outside(),
      false
    );
  }


  // ---- Buzzer pilote par les etats ----
  buzzer_set_sos(millis() < sosUntil);
  buzzer_set_alarm(geofence_is_outside());
  buzzer_set_removed(pulse_removed());
  buzzer_set_bpm_bad(pulse_bpm_bad());

  // ---- Affichage ----
  static unsigned long tDisp = 0;
  if (millis() - tDisp > 150) {
    tDisp = millis();
    unsigned long now = millis();

    if (now < sosUntil) {
      display_sos();
    } else if (button_is_down() && button_hold_ms() > 400) {
      display_hold_progress(button_hold_ms(), BTN_SOS_HOLD_MS);

    } else if (now < idCardUntil) {
      display_id_card();
    } else if (geofence_is_outside()) {
      display_out_of_zone(geofence_dist());
    } else if (pulse_removed()) {
      display_removed();
    } else if (pulse_bpm_bad()) {
      display_bpm_warning(pulse_bpm());
    } else {
      display_main();
    }
  }

  // ---- Debug principal ----
  static unsigned long tDbg = 0;
  if (millis() - tDbg > 2000) {
    tDbg = millis();
    Serial.printf("[DBG] fix=%d dist=%.0fm outside=%d bpm=%d ir=%ld skin=%d "
                  "stress=%.0f%% removed=%d bpmbad=%d\n",
                  gps_fix_ok(), geofence_dist(),
                  geofence_is_outside() ? 1 : 0,
                  pulse_bpm(), pulse_raw_ir(), pulse_skin_on() ? 1 : 0,
                  pulse_stress_proba() * 100.0f,
                  pulse_removed() ? 1 : 0,
                  pulse_bpm_bad() ? 1 : 0);
  }
}   // <-- cette accolade ferme loop()