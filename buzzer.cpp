#include "buzzer.h"
#include "config.h"

// Modes possibles
static bool alarmZone = false;      // hors zone : bip toutes les 3 s
static bool alarmRemoved = false;   // retire : 3 bips rapides toutes les 20 s
static bool alarmBpmBad = false;    // BPM anormal : 2 bips longs toutes les 30 s
static bool alarmSos = false;       // SOS : alarme sonore urgente

// Etat bip en cours
static unsigned long bipStart = 0;
static unsigned long bipDuration = 0;
static bool bipActive = false;

// Séquences à jouer
struct Seq {
  int beepsLeft;       // nombre de bips restants
  int beepMs;          // durée d'un bip
  int gapMs;           // durée du silence entre 2 bips
  unsigned long nextAt;// moment du prochain bip
};

static Seq seqSos      = {0, 200, 100, 0};
static Seq seqZone     = {0, 100, 2900, 0};
static Seq seqRemoved  = {0, 150, 150, 0};
static Seq seqBpmBad   = {0, 400, 400, 0};


static unsigned long lastSeqTrigger = 0;

void buzzer_init() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

// Fonction interne : joue un bip pendant duration ms
static void playBip(unsigned long duration) {
  bipStart = millis();
  bipDuration = duration;
  bipActive = true;
  digitalWrite(PIN_BUZZER, HIGH);
}

void buzzer_set_alarm(bool on) {
  alarmZone = on;
  if (!on) seqZone.beepsLeft = 0;
}

void buzzer_set_removed(bool on) {
  alarmRemoved = on;
  if (!on) seqRemoved.beepsLeft = 0;
}

void buzzer_set_bpm_bad(bool on) {
  alarmBpmBad = on;
  if (!on) seqBpmBad.beepsLeft = 0;
}

void buzzer_set_sos(bool on) {
  alarmSos = on;
  if (!on) seqSos.beepsLeft = 0;
}

void buzzer_stop_all_modes() {
  alarmSos = false;
  alarmZone = false;
  alarmRemoved = false;
  alarmBpmBad = false;
  seqSos.beepsLeft = 0;
  seqZone.beepsLeft = 0;
  seqRemoved.beepsLeft = 0;
  seqBpmBad.beepsLeft = 0;
}

void buzzer_update() {
  unsigned long now = millis();

  // Fin du bip en cours
  if (bipActive && (now - bipStart >= bipDuration)) {
    bipActive = false;
    digitalWrite(PIN_BUZZER, LOW);
  }

  // Priorité : SOS > retrait > BPM > hors zone
  Seq* active = nullptr;
  int totalBeeps = 0;
  unsigned long interval = 0;

  if (alarmSos) {
    active = &seqSos;
    totalBeeps = 4;
    interval = 1500;
  } else if (alarmRemoved) {
    active = &seqRemoved;
    totalBeeps = 3;
    interval = 20000;
  } else if (alarmBpmBad) {
    active = &seqBpmBad;
    totalBeeps = 2;
    interval = 30000;
  } else if (alarmZone) {
    active = &seqZone;
    totalBeeps = 1;
    interval = 3000;
  }


  if (active == nullptr) return;

  // Repos entre 2 séquences : tout est fini et on attend l'intervalle
  if (active->beepsLeft <= 0 && active->nextAt == 0) {
    if (lastSeqTrigger == 0 || (now - lastSeqTrigger >= interval)) {
      active->beepsLeft = totalBeeps;
      active->nextAt = now;
      lastSeqTrigger = now;
    }
  }

  // Jouer les bips de la séquence en cours
  if (active->beepsLeft > 0 && !bipActive && now >= active->nextAt) {
    playBip(active->beepMs);
    active->beepsLeft--;
    if (active->beepsLeft > 0) {
      active->nextAt = now + active->beepMs + active->gapMs;
    } else {
      active->nextAt = 0;
    }
  }
}