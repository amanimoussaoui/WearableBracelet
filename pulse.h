#pragma once
#include <Arduino.h>

// Initialise le capteur MAX30102
bool pulse_init();

// A appeler souvent : lit le capteur, met a jour BPM + peau
void pulse_update();

// Vrai si le capteur est present et initialise
bool pulse_ok();

// Vrai si de la peau est detectee sur le capteur
bool pulse_skin_on();

// BPM moyen lisse (0 si pas encore de mesure)
int pulse_bpm();

// Valeur brute IR lue par le capteur
long pulse_raw_ir();

// Vrai si le bracelet est considere comme retire (peau absente > 10 s)
bool pulse_removed();

// Vrai si le BPM est anormal depuis assez longtemps
bool pulse_bpm_bad();

// Score de stress calcule par stress_model.h (0.0 a 1.0)
float pulse_stress_proba();

// Vrai si stress detecte
bool pulse_is_stressed();