#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <Arduino.h>

/**
 * Client Supabase pour ESP8266 / ESP32 (SafeMind Bracelet)
 * Utilise la fonction RPC officielle push_telemetry avec RLS
 */
class SupabaseClient {
public:
    SupabaseClient();
    
    // Initialisation avec l'URL de base, la cle publique anon et l'UUID de l'appareil
    void begin(const char* url, const char* anonKey, const char* deviceId);

    // Envoi de la telemetrie periodique via la RPC push_telemetry
    int sendTelemetry(int bpm, float lat, float lon, bool hasGpsFix, int skinContact, int batteryLevel = 100,
                      bool removed = false, bool bpmBad = false, float dist = 0.0f, bool outside = false, bool sos = false);

    // Envoi d'un evenement critique immediat (OUT_OF_ZONE, BPM_ABNORMAL, REMOVED, SOS, etc.)
    int sendEvent(const char* type, const char* title, const char* detail, const char* severity = "urgent");

private:
    const char* _url;
    const char* _anonKey;
    const char* _deviceId;
};

extern SupabaseClient Supabase;

#endif
