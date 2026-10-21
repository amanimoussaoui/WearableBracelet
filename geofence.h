#pragma once
#include <Arduino.h>

// Initialise (rien a faire pour l'instant, mais symetrique)
void geofence_init();

// A appeler regulierement. Met a jour l'etat interne.
void geofence_update();

// Distance actuelle au domicile en metres (-1 si pas de fix)
double geofence_dist();

// Vrai si on est hors zone (avec hysteresis)
bool geofence_is_outside();

// Vrai si l'alerte "sortie" vient d'etre declenchee (a consommer une fois)
// Retourne aussi le motif : "first" (premiere sortie) ou "repeat" (rappel)
bool geofence_pop_event(String& reason);

// Force un changement de position simulee (pour test)
void geofence_sim_set(double lat, double lon);