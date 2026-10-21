#pragma once
#include <Arduino.h>
#include "types.h"

// Initialise le module alertes
void alerts_init();

// Envoie une alerte immediate, avec priorite (0=normal, 1=high, 2=urgent)
void alerts_send(AlertType type, const String& title, const String& detail, int priority = 0);

// Traite la file d'attente (a appeler souvent dans loop())
void alerts_update();

// Ajoute une entree a l'historique (pour le dashboard web)
void alerts_add_history(const String& s);

// Retourne l'historique (max 10)
int  alerts_history_count();
String alerts_history_get(int i);
// Envoie une alerte SOS
void alerts_sos();