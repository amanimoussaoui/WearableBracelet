#include "alerts.h"
#include "config.h"
#include "net.h"
#include "gps.h"
#include "supabase.h"

#define HIST_MAX 10
#define QUEUE_MAX 6

static String hist[HIST_MAX];
static int histCount = 0;

static String queue_[QUEUE_MAX];
static int queueCount = 0;
static unsigned long nextTry = 0;

void alerts_init() {
  histCount = 0;
  queueCount = 0;
  nextTry = 0;
}

void alerts_add_history(const String& s) {
  for (int i = min(histCount, HIST_MAX - 1); i > 0; i--) hist[i] = hist[i - 1];
  hist[0] = s;
  if (histCount < HIST_MAX) histCount++;
}

int alerts_history_count() { return histCount; }
String alerts_history_get(int i) {
  if (i < 0 || i >= histCount) return "";
  return hist[i];
}

// Encode un texte pour une URL (pour le lien Google Maps)
static String urlEncode(const String& s) {
  String o;
  char buf[4];
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') o += c;
    else { sprintf(buf, "%%%02X", (uint8_t)c); o += buf; }
  }
  return o;
}

// Construit le corps du message
static String buildMessage(AlertType t, const String& title, const String& detail) {
  String msg = title;
  if (detail.length() > 0) msg += "\n" + detail;

  // Lien Google Maps si on a une position
  if (gps_fix_ok()) {
    String link = "https://maps.google.com/?q=" +
                  String(gps_lat(), 6) + "," + String(gps_lon(), 6);
    msg += "\n📍 " + link;
  }

  // Distance du domicile
  extern double geofence_dist();
  double d = geofence_dist();
  if (d >= 0) {
    msg += "\n🏠 " + String(d, 0) + " m du domicile";
  }

  msg += "\n🔋 Batterie : " + String(100) + " %";   // placeholder, on branchera la batterie a l'etape 6
  return msg;
}

// Choisit le titre ntfy selon le type
static String titleFor(AlertType t) {
  switch (t) {
    case A_SOS:     return "🚨 SOS - Appel a l'aide";
    case A_OUT:     return "🚨 Sortie de zone";
    case A_BACK:    return "✅ Retour en zone";
    case A_REMOVED: return "⚠️ Bracelet retire";
    case A_BPM:     return "🚨 BPM anormal";
    case A_BAT:     return "⚠️ Batterie faible";
    case A_STRESS:  return "⚠️ Stress eleve detecte";
    case A_BOOT:    return "ℹ️ Demarrage";
    default:        return "ℹ️ Info";
  }

}

void alerts_send(AlertType type, const String& title, const String& detail, int priority) {
  String msg = buildMessage(type, title, detail);
  String fullTitle = titleFor(type);

  // Historique local
  alerts_add_history(gps_hhmm() + " " + title);

  // Envoi vers Supabase
  const char* typeStr = "INFO";
  const char* sevStr = "info";
  switch (type) {
    case A_SOS:     typeStr = "SOS";          sevStr = "urgent";  break;
    case A_OUT:     typeStr = "OUT_OF_ZONE";  sevStr = "urgent";  break;
    case A_BACK:    typeStr = "IN_ZONE";      sevStr = "info";    break;
    case A_REMOVED: typeStr = "REMOVED";      sevStr = "urgent";  break;
    case A_BPM:     typeStr = "BPM_ABNORMAL"; sevStr = "warning"; break;
    case A_BAT:     typeStr = "BATTERY_LOW";  sevStr = "warning"; break;
    case A_STRESS:  typeStr = "STRESS";       sevStr = "warning"; break;
    case A_BOOT:    typeStr = "BOOT";         sevStr = "info";    break;
    default: break;

  }
  Supabase.sendEvent(typeStr, title.c_str(), detail.c_str(), sevStr);

  // Envoi direct avec priorité (ntfy)
  if (net_send(fullTitle, msg, priority)) return;

  // File d'attente
  if (queueCount < QUEUE_MAX) {
    queue_[queueCount++] = String(priority) + "|" + fullTitle + "|" + msg;
  }
}


void alerts_update() {
  if (queueCount == 0) return;
  if (millis() < nextTry) return;

  String entry = queue_[0];
  int sep = entry.indexOf('|');
  String title = entry.substring(0, sep);
  String msg   = entry.substring(sep + 1);

  if (net_send(title, msg)) {
    for (int i = 0; i < queueCount - 1; i++) queue_[i] = queue_[i + 1];
    queueCount--;
    nextTry = 0;
  } else {
    nextTry = millis() + 20000;   // reessaie dans 20 s
  }
}
void alerts_sos() {
  String detail = "L'aidant " + String(CAREGIVER_NAME) + " est prevenu.";
  alerts_send(A_SOS, "SOS - Appel a l'aide", detail, 2);
}