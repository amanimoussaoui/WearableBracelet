#pragma once
#include <Arduino.h>
#include "types.h"

// Initialise l'OLED. Retourne true si trouve.
bool display_init();

// Affiche un ecran simple centre.
void display_boot(const String& line1, const String& line2);

// Affiche un ecran d'alerte minimal.
void display_alert(AlertType t, const String& detail);
// Ecran principal : heure + date + message rassurant.
// A appeler en boucle dans loop().
void display_main();
// Ecran special "HORS ZONE"
void display_out_of_zone(double dist);
// Carte d'identite numerique (affichee sur appui court)
void display_id_card();

// Ecran SOS (affiche sur appui long)
void display_sos();

// Barre de progression pour l'appui long en cours
void display_hold_progress(unsigned long held, unsigned long total);
// Ecran "BPM anormal"
void display_bpm_warning(int bpm);

// Ecran "bracelet retire"
void display_removed();