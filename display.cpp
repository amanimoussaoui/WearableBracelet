#include "display.h"
#include "config.h"
#include "gps.h"
#include "geofence.h"
#include "pulse.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 oled(128, 64, &Wire, -1);
static bool oledOk = false;

bool display_init() {
  Wire.begin(D2, D1);
  Wire.setClock(400000);
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // essaie 0x3D si ecran noir
  if (!oledOk) {
    Serial.println("OLED introuvable (essaie 0x3D)");
    return false;
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.display();
  return true;
}

static void centered(const String& s, int y, int size) {
  oled.setTextSize(size);
  int x = (128 - (int)s.length() * 6 * size) / 2;
  if (x < 0) x = 0;
  oled.setCursor(x, y);
  oled.print(s);
}

void display_boot(const String& line1, const String& line2) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  centered(line1, 20, 1);
  centered(line2, 36, 1);
  oled.display();
}

void display_alert(AlertType t, const String& detail) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  const char* tag = "INFO";
  const char* l1  = "";
  if (t == A_SOS)     { tag = "URGENT";    l1 = "SOS"; }
  if (t == A_OUT)     { tag = "URGENT";    l1 = "HORS ZONE"; }
  if (t == A_BPM)     { tag = "URGENT";    l1 = "BPM ANORMAL"; }
  if (t == A_BACK)    { tag = "INFO";      l1 = "RETOUR OK"; }
  if (t == A_REMOVED) { tag = "ATTENTION"; l1 = "NON PORTE"; }
  if (t == A_BAT)     { tag = "ATTENTION"; l1 = "BATTERIE"; }
  if (t == A_BOOT)    { tag = "INFO";      l1 = "DEMARRE"; }

  oled.fillRect(0, 0, 128, 11, SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK);
  centered(tag, 2, 1);
  oled.setTextColor(SSD1306_WHITE);
  centered(l1, 22, 2);
  centered(detail, 52, 1);
  oled.display();
}
// Jours / mois en francais (sans accents : police par defaut)
static const char* JOURS[] = {
  "dimanche", "lundi", "mardi", "mercredi",
  "jeudi", "vendredi", "samedi"
};
static const char* MOIS[] = {
  "janvier", "fevrier", "mars", "avril", "mai", "juin",
  "juillet", "aout", "septembre", "octobre", "novembre", "decembre"
};

void display_main() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  // Ligne du haut : BPM a gauche, GPS au milieu, batterie a droite
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  if (pulse_ok() && pulse_skin_on()) {
    if (pulse_bpm() > 0) {
      oled.print("BPM ");
      oled.print(pulse_bpm());
    } else {
      oled.print("BPM ...");
    }
  }

  if (!gps_fix_ok()) {
    oled.setCursor(52, 0);
    oled.print("GPS...");
  }

  if (geofence_is_outside()) {
    oled.setCursor(90, 0);
    oled.print("HORS");
  }

  struct tm t;
  if (!gps_get_local(t)) {
    centered("Recherche heure...", 24, 1);
    oled.display();
    return;
  }

  // Heure en grand
  char buf[8];
  sprintf(buf, "%02d:%02d", t.tm_hour, t.tm_min);
  centered(String(buf), 14, 3);

  // Ligne du bas
  String line;
  switch ((millis() / 4000) % 3) {
    case 0:
      line = String(t.tm_hour < 18 ? "Bonjour " : "Bonsoir ") + PATIENT_NAME;
      break;
    case 1:
      line = String(JOURS[t.tm_wday]) + " " + String(t.tm_mday) + " " + MOIS[t.tm_mon];
      break;
    default:
      line = String(JOURS[t.tm_wday]) + " " +
             (t.tm_hour < 12 ? "matin" :
             (t.tm_hour < 18 ? "apres-midi" : "soir"));
      break;
  }
  centered(line, 52, 1);

  oled.display();
}
void display_out_of_zone(double dist) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  // Bandeau URGENT clignotant
  bool outline = ((millis() / 500) % 2);
  if (outline) {
    oled.drawRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_WHITE);
  } else {
    oled.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
  }
  centered("URGENT", 2, 1);
  oled.setTextColor(SSD1306_WHITE);

  centered("HORS", 18, 2);
  centered("ZONE", 36, 2);

  char buf[24];
  snprintf(buf, sizeof(buf), "%d m du domicile", (int)dist);
  centered(buf, 56, 1);

  oled.display();
}

void display_id_card() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  centered("Je suis", 0, 1);
  centered(PATIENT_NAME, 10, 2);
  centered("Troubles de memoire", 32, 1);

  oled.drawFastHLine(0, 42, 128, SSD1306_WHITE);

  centered("Appelez mon aidant", 45, 1);
  centered(CAREGIVER_PHONE, 55, 1);

  oled.display();
}

void display_sos() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  bool outline = ((millis() / 300) % 2);
  if (outline) {
    oled.drawRect(0, 0, 128, 20, SSD1306_WHITE);
    oled.setTextColor(SSD1306_WHITE);
  } else {
    oled.fillRect(0, 0, 128, 20, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
  }
  centered("SOS", 4, 2);
  oled.setTextColor(SSD1306_WHITE);

  centered("APPEL SOS", 24, 2);
  centered("Aidant prevenu", 44, 1);
  centered(CAREGIVER_PHONE, 55, 1);

  oled.display();
}

void display_hold_progress(unsigned long held, unsigned long total) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  centered("SOS", 6, 2);
  centered("Maintenez...", 28, 1);

  oled.drawRect(10, 44, 108, 12, SSD1306_WHITE);
  int w = (int)(104.0 * held / total);
  if (w > 104) w = 104;
  if (w > 0) oled.fillRect(12, 46, w, 8, SSD1306_WHITE);

  oled.display();
}

void display_bpm_warning(int bpm) {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  bool outline = ((millis() / 500) % 2);
  if (outline) {
    oled.drawRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_WHITE);
  } else {
    oled.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
  }
  centered("URGENT", 2, 1);
  oled.setTextColor(SSD1306_WHITE);

  // Petit coeur
  oled.fillCircle(24, 26, 7, SSD1306_WHITE);
  oled.fillCircle(38, 26, 7, SSD1306_WHITE);
  oled.fillTriangle(17, 30, 45, 30, 31, 46, SSD1306_WHITE);

  // BPM en grand
  centered(String(bpm), 18, 3);
  centered("BPM", 44, 1);
  centered(bpm < BPM_MIN ? "Rythme trop lent" : "Rythme trop rapide", 56, 1);

  oled.display();
}

void display_removed() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  bool outline = ((millis() / 700) % 2);
  if (outline) {
    oled.drawRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_WHITE);
  } else {
    oled.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
  }
  centered("ATTENTION", 2, 1);
  oled.setTextColor(SSD1306_WHITE);

  // Symbole bracelet
  oled.drawCircle(64, 26, 14, SSD1306_WHITE);
  oled.drawCircle(64, 26, 16, SSD1306_WHITE);
  oled.drawLine(50, 42, 78, 10, SSD1306_WHITE);   // barre oblique

  centered("NON PORTE", 46, 1);
  centered("Remettez le bracelet", 56, 1);

  oled.display();
}