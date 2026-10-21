#include "net.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

bool net_init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connexion");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK : ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("WiFi ECHEC");
  return false;
}

void net_update() {
  static unsigned long last = 0;
  if (millis() - last < 10000) return;
  last = millis();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi perdu, reconnexion...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

bool net_send(const String& title, const String& message, int priority) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();          // ntfy.sh : pas de verif certificat
  client.setTimeout(4000);

  HTTPClient http;
  http.setTimeout(4000);

  String url = String("https://ntfy.sh/") + NTFY_TOPIC;

  if (!http.begin(client, url)) {
    Serial.println("ntfy : http.begin a echoue");
    return false;
  }

  http.addHeader("Content-Type", "text/plain; charset=utf-8");
  http.addHeader("Title", title);

  // Priorite et tags selon le niveau
  if (priority >= 2) {
    http.addHeader("Priority", "urgent");
    http.addHeader("Tags", "rotating_light,warning");
  } else if (priority >= 1) {
    http.addHeader("Priority", "high");
    http.addHeader("Tags", "warning");
  } else {
    http.addHeader("Priority", "default");
    http.addHeader("Tags", "information_source");
  }

  int code = http.POST((uint8_t*)message.c_str(), message.length());
  if (code == 200) {
    Serial.printf("ntfy OK (prio %d)\n", priority);
  } else {
    Serial.printf("ntfy erreur HTTP %d\n", code);
  }
  http.end();
  return code == 200;
}