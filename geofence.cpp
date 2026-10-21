#include "geofence.h"
#include "config.h"
#include "gps.h"
#include <TinyGPSPlus.h>

static double lastDist = -1;
static bool   outside = false;
static int    outCount = 0;
static unsigned long lastAlertMs = 0;
static String pendingReason = "";

void geofence_init() {
  lastDist = -1;
  outside = false;
  outCount = 0;
  lastAlertMs = 0;
  pendingReason = "";
}

// Calcule la distance entre deux points GPS (formule de Haversine simplifiee,
// celle de TinyGPSPlus est suffisante)
static double distance_m(double lat1, double lon1, double lat2, double lon2) {
  return TinyGPSPlus::distanceBetween(lat1, lon1, lat2, lon2);
}

void geofence_update() {
  if (!gps_fix_ok()) return;   // pas de position : on ne fait rien

  lastDist = distance_m(gps_lat(), gps_lon(), HOME_LAT, HOME_LON);
  

  unsigned long now = millis();

  if (lastDist > RAYON_SORTIE) {
    if (outCount < 250) outCount++;
    if (outCount >= GEOFENCE_CONFIRM_N) {
      if (!outside) {
        // Premiere sortie
        outside = true;
        lastAlertMs = now;
        pendingReason = "first";
      } else if (now - lastAlertMs > REALERT_MS) {
        // Toujours dehors : rappel
        lastAlertMs = now;
        pendingReason = "repeat";
      }
    }
  } else if (lastDist < RAYON_RETOUR) {
    outCount = 0;
    if (outside) {
      outside = false;
      lastAlertMs = 0;
      pendingReason = "back";
    }
  }
}

double geofence_dist()      { return lastDist; }
bool   geofence_is_outside(){ return outside; }

bool geofence_pop_event(String& reason) {
  if (pendingReason.length() == 0) return false;
  reason = pendingReason;
  pendingReason = "";
  return true;
}

void geofence_sim_set(double lat, double lon) {
  // Rien : la simulation se fait via gps.cpp.
  // On garde cette fonction pour compatibilite future.
  (void)lat; (void)lon;
}