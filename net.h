#pragma once
#include <Arduino.h>

// Initialise le WiFi (bloquant, avec timeout).
bool net_init();

// Maintient la connexion (a appeler regulierement dans loop()).
void net_update();

// Envoie une notification ntfy.
// priority : 0 = normal, 1 = high, 2 = urgent
bool net_send(const String& title, const String& message, int priority = 0);