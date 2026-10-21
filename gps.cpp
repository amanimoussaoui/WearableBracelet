#include "gps.h"
#include "config.h"
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <time.h>

// Position simulee au domicile
#define SIM_LAT_HOME   HOME_LAT
#define SIM_LON_HOME   HOME_LON

// Position simulee HORS ZONE (environ 290 m du domicile)
#define SIM_LAT_OUT    36.808700
#define SIM_LON_OUT    10.183300

static SoftwareSerial gpsSerial(PIN_GPS_RX, PIN_GPS_TX);
static TinyGPSPlus gps;

static time_t baseTs = 0;
static unsigned long baseMs = 0;
static bool timeOk = false;
static bool rawMode = false;

// Conversion date -> epoch
static time_t toEpoch(int y, int m, int d, int hh, int mm, int ss) {
  y -= (m <= 2);
  long era = (y >= 0 ? y : y - 399) / 400;
  long yoe = y - era * 400;
  long doy = (153L * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  long days = era * 146097L + doe - 719468L;
  return (time_t)days * 86400L + hh * 3600L + mm * 60L + ss;
}

void gps_init() {
  gpsSerial.begin(9600, SWSERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX, false, 256);
  // Synchronise l'heure par NTP (fallback si GPS ne fournit pas d'heure)
  configTime(TZ_OFFSET_H * 3600, 0, "pool.ntp.org", "time.google.com");
}

void gps_set_raw(bool on) { rawMode = on; }

void gps_update() {
#if (GPS_MODE != 0)
  // En simulation : on vide le buffer serie
  while (gpsSerial.available()) gpsSerial.read();
#else
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (rawMode) Serial.write(c);
    gps.encode(c);
  }

  if (gps.time.isUpdated() && gps.time.isValid() &&
      gps.date.isValid() && gps.date.year() >= 2024) {
    baseTs = toEpoch(gps.date.year(), gps.date.month(), gps.date.day(),
                     gps.time.hour(), gps.time.minute(), gps.time.second())
             + TZ_OFFSET_H * 3600L;
    baseMs = millis();
    timeOk = true;
  }
#endif
}

bool gps_time_ok() {
#if (GPS_MODE != 0)
  return time(nullptr) > 1700000000;
#else
  return timeOk;
#endif
}

bool gps_fix_ok() {
#if (GPS_MODE != 0)
  return true;   // simulation : fix actif
#else
  return gps.location.isValid() && gps.location.age() < 5000;
#endif
}

double gps_lat() {
#if (GPS_MODE == 2)
  return SIM_LAT_OUT;   // Position hors zone (290 m)
#elif (GPS_MODE == 1)
  return SIM_LAT_HOME;  // Domicile (0 m)
#else
  return gps.location.isValid() ? gps.location.lat() : 0.0;
#endif
}

double gps_lon() {
#if (GPS_MODE == 2)
  return SIM_LON_OUT;   // Position hors zone (290 m)
#elif (GPS_MODE == 1)
  return SIM_LON_HOME;  // Domicile (0 m)
#else
  return gps.location.isValid() ? gps.location.lng() : 0.0;
#endif
}

bool gps_get_local(struct tm& t) {
#if (GPS_MODE != 0)
  time_t n = time(nullptr);
  if (n > 1700000000) { localtime_r(&n, &t); return true; }
  return false;
#else
  if (timeOk) {
    time_t ts = baseTs + (millis() - baseMs) / 1000;
    gmtime_r(&ts, &t);
    return true;
  }
  time_t n = time(nullptr);
  if (n > 1700000000) { localtime_r(&n, &t); return true; }
  return false;
#endif
}

String gps_hhmm() {
  struct tm t;
  char b[8];
  if (gps_get_local(t)) {
    sprintf(b, "%02d:%02d", t.tm_hour, t.tm_min);
    return String(b);
  }
  return "--:--";
}

unsigned long gps_chars()           { return gps.charsProcessed(); }
unsigned long gps_checksum_errors() { return gps.failedChecksum(); }
int           gps_satellites()      { return gps.satellites.value(); }