#pragma once

// Types d'alerte utilises dans tout le projet.
enum AlertType {
  A_NONE = 0,
  A_BOOT,       // demarrage
  A_SOS,        // bouton SOS
  A_OUT,        // sortie de zone
  A_BACK,       // retour en zone
  A_REMOVED,    // bracelet retire
  A_BPM,        // BPM anormal
  A_BAT,        // batterie faible
  A_STRESS      // stress eleve
};