#include "button.h"
#include "config.h"

static bool lastReading = false;
static bool stableState = false;
static unsigned long lastDebounce = 0;
static unsigned long pressStart = 0;
static bool longFired = false;
static ButtonEvent pendingEvent = BTN_NONE;

void button_init() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void button_update() {
  bool reading = (digitalRead(PIN_BUTTON) == LOW);   // actif bas
  unsigned long now = millis();

  // Anti-rebond
  if (reading != lastReading) {
    lastDebounce = now;
    lastReading = reading;
  }

  if ((now - lastDebounce) > BTN_DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;

      if (reading) {
        // front descendant : debut d'appui
        pressStart = now;
        longFired = false;
      } else {
        // front montant : fin d'appui
        if (!longFired && pressStart != 0) {
          unsigned long held = now - pressStart;
          if (held >= 40) {   // clic court detecte des 40ms
            pendingEvent = BTN_SHORT_PRESS;
          }
        }
        pressStart = 0;
        longFired = false;
      }
    }
  }

  // Detection appui long pendant l'appui
  if (stableState && !longFired && pressStart != 0) {
    if ((now - pressStart) >= BTN_SOS_HOLD_MS) {
      longFired = true;
      pendingEvent = BTN_LONG_PRESS;
    }
  }
}

ButtonEvent button_pop_event() {
  ButtonEvent e = pendingEvent;
  pendingEvent = BTN_NONE;
  return e;
}

bool button_is_down() { return stableState; }

unsigned long button_hold_ms() {
  if (!stableState || pressStart == 0) return 0;
  return millis() - pressStart;
}