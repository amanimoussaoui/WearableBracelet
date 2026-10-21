#include "supabase.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
    #include <WiFiClientSecure.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <WiFiClientSecure.h>
#endif

SupabaseClient Supabase;

SupabaseClient::SupabaseClient() : _url(nullptr), _anonKey(nullptr), _deviceId(nullptr) {}

void SupabaseClient::begin(const char* url, const char* anonKey, const char* deviceId) {
    _url = url;
    _anonKey = anonKey;
    _deviceId = deviceId;
}

int SupabaseClient::sendTelemetry(int bpm, float lat, float lon, bool hasGpsFix, int skinContact, int batteryLevel,
                                  bool removed, bool bpmBad, float dist, bool outside, bool sos) {
    if (!_url || !_anonKey || !_deviceId) {
        Serial.println(F("[Supabase] ERREUR: Client non initialise."));
        return -1;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[Supabase] WiFi non connecte"));
        return -2;
    }

    WiFiClientSecure client;
    client.setInsecure(); // Ignore la validation SSL stricte sur microcontroleur
#if defined(ESP8266)
    client.setBufferSizes(1024, 512); // Indispensable pour eviter l'epuisement de RAM sur ESP8266
#endif
    client.setTimeout(4000);

    HTTPClient http;
    http.setTimeout(4000);
    http.setReuse(false);

    // Utilisation de la fonction RPC push_telemetry
    String endpoint = String(_url) + "/rest/v1/rpc/push_telemetry";

    if (!http.begin(client, endpoint)) {
        Serial.println(F("[Supabase] Connexion HTTPS echouee"));
        return -3;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", _anonKey);
    http.addHeader("Authorization", String("Bearer ") + _anonKey);

    // Construction du payload pour la fonction RPC push_telemetry
    String json = "{";
    json += "\"p_key\":\"" + String(_deviceId) + "\",";
    json += "\"p_bpm\":" + String(bpm) + ",";
    json += "\"p_skin\":" + String(skinContact ? "true" : "false") + ",";
    json += "\"p_removed\":" + String(removed ? "true" : "false") + ",";
    json += "\"p_bpm_bad\":" + String(bpmBad ? "true" : "false") + ",";
    json += "\"p_dist\":" + String(dist, 1) + ",";
    json += "\"p_outside\":" + String(outside ? "true" : "false") + ",";
    json += "\"p_sos\":" + String(sos ? "true" : "false") + ",";

    if (hasGpsFix && lat != 0.0f && lon != 0.0f) {
        json += "\"p_lat\":" + String(lat, 6) + ",";
        json += "\"p_lon\":" + String(lon, 6) + ",";
        json += "\"p_fix\":true";
    } else {
        json += "\"p_lat\":null,";
        json += "\"p_lon\":null,";
        json += "\"p_fix\":false";
    }
    json += "}";

    int httpCode = http.POST(json);

    if (httpCode == 200 || httpCode == 201 || httpCode == 204) {
        Serial.print(F("[Supabase] Telemetrie envoyee -> OK (HTTP "));
        Serial.print(httpCode);
        Serial.println(F(")"));
    } else {
        Serial.print(F("[Supabase] Erreur envoi HTTP "));
        Serial.println(httpCode);
        if (httpCode < 0) {
            Serial.println(http.errorToString(httpCode).c_str());
        }
    }

    http.end();
    client.stop();
    return httpCode;
}

int SupabaseClient::sendEvent(const char* type, const char* title, const char* detail, const char* severity) {
    if (!_url || !_anonKey || !_deviceId) {
        return -1;
    }

    if (WiFi.status() != WL_CONNECTED) {
        return -2;
    }

    // Si SOS ou sortie de zone, la RPC push_telemetry declenche automatiquement l'evenement
    if (strcmp(type, "SOS") == 0) {
        return sendTelemetry(0, 0.0f, 0.0f, false, 1, 100, false, false, 0.0f, false, true);
    }
    if (strcmp(type, "OUT_OF_ZONE") == 0) {
        return sendTelemetry(0, 0.0f, 0.0f, false, 1, 100, false, false, 0.0f, true, false);
    }

    WiFiClientSecure client;
    client.setInsecure();
#if defined(ESP8266)
    client.setBufferSizes(1024, 512);
#endif
    client.setTimeout(4000);

    HTTPClient http;
    http.setTimeout(4000);
    http.setReuse(false);
    String endpoint = String(_url) + "/rest/v1/events";

    if (!http.begin(client, endpoint)) {
        return -3;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", _anonKey);
    http.addHeader("Authorization", String("Bearer ") + _anonKey);
    http.addHeader("Prefer", "return=minimal");

    String json = "{";
    json += "\"device_id\":\"" + String(_deviceId) + "\",";
    json += "\"type\":\"" + String(type) + "\",";
    json += "\"title\":\"" + String(title) + "\",";
    json += "\"detail\":\"" + String(detail) + "\",";
    json += "\"severity\":\"" + String(severity) + "\"";
    json += "}";

    int httpCode = http.POST(json);

    if (httpCode > 0) {
        Serial.print(F("[Supabase] Evenement "));
        Serial.print(type);
        Serial.print(F(" -> HTTP "));
        Serial.println(httpCode);
    }

    http.end();
    client.stop();
    return httpCode;
}
