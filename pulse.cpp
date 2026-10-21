#include "pulse.h"
#include "config.h"
#include "stress_model.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <math.h>

static MAX30105 sensor;
static bool sensorOk = false;

// Suivi peau
static long lastIR = 0;
static bool skinOn = false;
static unsigned long skinLostSince = 0;

// Suivi BPM
static byte rates[BPM_AVG_SIZE] = {0};
static byte rateSpot = 0;
static byte beatCount = 0;
static unsigned long lastBeat = 0;
static int bpmAvg = 0;

// Suivi Stress (WESAD)
#define RR_BUFFER_SIZE 20
static unsigned long rrBuffer[RR_BUFFER_SIZE] = {0};
static byte rrSpot = 0;
static byte rrCount = 0;
static float currentStress = 0.0f;

// Suivi BPM anormal
static unsigned long bpmBadSince = 0;


bool pulse_init() {
  Wire.begin(D2, D1);
  Wire.setClock(400000);

  sensorOk = sensor.begin(Wire, I2C_SPEED_FAST);
  if (!sensorOk) {
    Serial.println("MAX30102 introuvable !");
    return false;
  }

  // 0x1F = LED moyenne (0x3F sature trop), 4 samples, 2 = 100Hz, 411 = 18-bit, 4096 = moyenne
  sensor.setup(0x1F, 4, 2, 400, 411, 4096);
  Serial.println("MAX30102 OK");

  skinLostSince = millis();
  return true;
}

bool pulse_ok()      { return sensorOk; }
bool pulse_skin_on() { return skinOn; }
long pulse_raw_ir()  { return lastIR; }

int pulse_bpm() {
  if (!skinOn) return 0;
  if (lastBeat > 0 && (millis() - lastBeat > 5000)) return 0;
  return bpmAvg;
}

void pulse_update() {
  if (!sensorOk) return;

  sensor.check();

  while (sensor.available()) {
    long ir = sensor.getFIFOIR();
    sensor.nextSample();
    lastIR = ir;

    // Detection de peau avec hystérésis : seuil strict
    skinOn = (ir >= IR_SKIN_THRESHOLD);

    if (!skinOn) {
      if (skinLostSince == 0) skinLostSince = millis();
      bpmAvg = 0;
      beatCount = 0;
      rateSpot = 0;
      bpmBadSince = 0;
      rrCount = 0;
      rrSpot = 0;
      currentStress = 0.0f;
      lastBeat = 0;
      continue;
    }
    skinLostSince = 0;

    // Detection de battement
    if (checkForBeat(ir)) {
      unsigned long now = millis();
      unsigned long delta = (lastBeat > 0) ? (now - lastBeat) : 0;
      lastBeat = now; // Toujours mettre a jour lastBeat pour eviter le blocage de delta

      // Filtre sur l'intervalle physiologique (330 ms à 1600 ms, soit 37 à 180 BPM)
      if (delta >= 330 && delta <= 1600) {
        float bpm = 60000.0f / delta;
        rates[rateSpot++] = (byte)bpm;
        rateSpot %= BPM_AVG_SIZE;
        if (beatCount < BPM_AVG_SIZE) beatCount++;

        // Moyenne disponible dès le 1er battement détecté
        int sum = 0;
        for (byte i = 0; i < beatCount; i++) sum += rates[i];
        bpmAvg = sum / beatCount;

        // Buffer circulaire pour calcul HRV (WESAD)
        rrBuffer[rrSpot++] = delta;
        rrSpot %= RR_BUFFER_SIZE;
        if (rrCount < RR_BUFFER_SIZE) rrCount++;

        // Evaluation du modele WESAD des 10 battements
        if (rrCount >= 10) {
          double sumRR = 0;
          for (byte i = 0; i < rrCount; i++) sumRR += rrBuffer[i];
          double meanRR = sumRR / rrCount;
          double meanBpm = 60000.0 / meanRR;

          // SDNN : écart-type des intervalles RR
          double varSum = 0;
          for (byte i = 0; i < rrCount; i++) {
            double diff = (double)rrBuffer[i] - meanRR;
            varSum += diff * diff;
          }
          double sdnn = sqrt(varSum / rrCount);

          // RMSSD et pNN50 : différences successives
          double diffSqSum = 0;
          int count50 = 0;
          for (byte i = 0; i < rrCount - 1; i++) {
            byte idx1 = (rrSpot - rrCount + i + RR_BUFFER_SIZE) % RR_BUFFER_SIZE;
            byte idx2 = (rrSpot - rrCount + i + 1 + RR_BUFFER_SIZE) % RR_BUFFER_SIZE;
            long d = (long)rrBuffer[idx2] - (long)rrBuffer[idx1];
            diffSqSum += (double)(d * d);
            if (abs(d) > 50) count50++;
          }
          double rmssd = sqrt(diffSqSum / (rrCount - 1));
          double pnn50 = ((double)count50 / (rrCount - 1)) * 100.0;

          // Entree modele WESAD : {bpm_mean, sdnn, rmssd, pnn50}
          double input[4] = {meanBpm, sdnn, rmssd, pnn50};
          double output[2] = {1.0, 0.0};
          score(input, output);
          currentStress = (float)output[1]; // proba_stress
        }
      }
    }
  }

  // Si aucun battement detecte depuis plus de 5 secondes ou peau absente : pouls retombe a 0
  unsigned long now = millis();
  if (!skinOn || (lastBeat > 0 && (now - lastBeat > 5000))) {
    bpmAvg = 0;
    beatCount = 0;
    rateSpot = 0;
    currentStress = 0.0f;
  }

  // BPM anormal depuis assez longtemps ?
  if (skinOn && bpmAvg > 0 && (bpmAvg < BPM_MIN || bpmAvg > BPM_MAX)) {
    if (bpmBadSince == 0) bpmBadSince = now;
  } else {
    bpmBadSince = 0;
  }
}


bool pulse_removed() {
  if (!sensorOk) return false;
  if (skinOn) return false;
  if (skinLostSince == 0) return false;
  return (millis() - skinLostSince) > REMOVED_DELAY_MS;
}

bool pulse_bpm_bad() {
  if (!sensorOk) return false;
  if (!skinOn) return false;
  if (bpmAvg == 0) return false;
  if (bpmBadSince == 0) return false;
  return (millis() - bpmBadSince) > BPM_BAD_DELAY_MS;
}

float pulse_stress_proba() {
  return currentStress;
}

bool pulse_is_stressed() {
  return (skinOn && currentStress >= 0.70f);
}