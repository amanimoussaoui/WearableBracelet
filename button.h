#pragma once
#include <Arduino.h>

// Types d'evenements bouton
enum ButtonEvent {
  BTN_NONE = 0,
  BTN_SHORT_PRESS,   // appui court : carte ID
  BTN_LONG_PRESS     // appui long : SOS
};

void button_init();

// A appeler a chaque tour de loop()
void button_update();

// Recupere le dernier evenement (et le consomme)
ButtonEvent button_pop_event();

// Vrai si le bouton est actuellement enfonce
bool button_is_down();

// Temps d'appui en cours (ms), 0 si pas appuye
unsigned long button_hold_ms();