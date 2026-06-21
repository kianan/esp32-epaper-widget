#pragma once
#include <WiFi.h>
#include <WiFiMulti.h>
#include <LittleFS.h>
#include <vector>

#define WIFI_CONFIG "/wifi.cfg"

// ── Credential storage ─────────────────────────────────────────────────────

struct WifiCred { String ssid, pass; };

static std::vector<WifiCred> _wifiLoadCreds() {
    std::vector<WifiCred> list;
    if (!LittleFS.exists(WIFI_CONFIG)) return list;
    File f = LittleFS.open(WIFI_CONFIG, "r");
    while (f.available()) {
        String ssid = f.readStringUntil('\n'); ssid.trim();
        String pass = f.readStringUntil('\n'); pass.trim();
        if (ssid.length() > 0) list.push_back({ssid, pass});
    }
    f.close();
    return list;
}

static void _wifiSaveCred(const String& ssid, const String& pass) {
    auto list = _wifiLoadCreds();
    bool found = false;
    for (auto& c : list) {
        if (c.ssid == ssid) { c.pass = pass; found = true; break; }
    }
    if (!found) list.push_back({ssid, pass});
    File f = LittleFS.open(WIFI_CONFIG, "w");
    for (auto& c : list) { f.println(c.ssid); f.println(c.pass); }
    f.close();
}

// ── Connection ─────────────────────────────────────────────────────────────

static WiFiMulti _wifiMulti;

void wifiBegin(uint32_t timeoutMs = 8000) {
    auto creds = _wifiLoadCreds();
    if (creds.empty()) return;

    for (auto& c : creds)
        _wifiMulti.addAP(c.ssid.c_str(), c.pass.c_str());

    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (_wifiMulti.run() == WL_CONNECTED) {
#ifdef DEBUG
            Serial.printf("[wifi] connected — %s (%s)\n",
                WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
#endif
            return;
        }
        delay(500);
    }
#ifdef DEBUG
    Serial.println("[wifi] timeout");
#endif
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }
