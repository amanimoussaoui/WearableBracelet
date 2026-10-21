#pragma once
#include <Arduino.h>

// Initialise la broche du buzzer
void buzzer_init();

// A appeler a chaque tour de loop()
void buzzer_update();

// Active/desactive le bip periodique
// Quand actif : bip court toutes les 3 secondes
void buzzer_set_alarm(bool on);
// Mode "bracelet retire" : 3 bips rapides toutes les 20 s
void buzzer_set_removed(bool on);

// Mode "BPM anormal" : 2 bips longs toutes les 30 s
void buzzer_set_bpm_bad(bool on);

// Mode SOS : alarme puissante et rapide
void buzzer_set_sos(bool on);

// Arrete tous les modes (mais pas les bips ponctuels)
void buzzer_stop_all_modes();