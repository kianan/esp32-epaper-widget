#pragma once
#include <WebServer.h>
#include <DNSServer.h>
#include "WaveshareEPD.h"
#include "screen_menu.h"
#include "touch.h"
#include "wifi_conn.h"
#include <Fonts/FreeSansBold9pt7b.h>

#define PORTAL_SSID  "Widget-Setup"

// ── EPD screens ───────────────────────────────────────────────────────────

static void _drawWifiStatus(WaveshareEPD& epd) {
    auto list = _wifiLoadCreds();
    bool connected = (WiFi.status() == WL_CONNECTED);

    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.setCursor(10, 20); epd.print("WiFi");
    epd.drawLine(0, 27, 200, 27, 0);

    if (connected) {
        epd.setCursor(10, 55); epd.print("Connected:");
        String ssid = WiFi.SSID();
        if (ssid.length() > 15) ssid = ssid.substring(0, 14) + "~";
        epd.setCursor(10, 78); epd.print(ssid);
        epd.setCursor(10, 103); epd.print(WiFi.localIP().toString());
        epd.setCursor(10, 140); epd.print("Tap: add network");
    } else if (list.empty()) {
        epd.setCursor(10, 65);  epd.print("No networks");
        epd.setCursor(10, 87);  epd.print("saved yet.");
        epd.setCursor(10, 125); epd.print("Tap to start");
        epd.setCursor(10, 147); epd.print("setup.");
    } else {
        epd.setCursor(10, 55); epd.print("Not connected.");
        epd.setCursor(10, 78); epd.print("Saved:");
        int y = 101;
        for (int i = 0; i < (int)list.size() && i < 3; i++) {
            String label = list[i].ssid;
            if (label.length() > 15) label = label.substring(0, 14) + "~";
            epd.setCursor(10, y); epd.print(label);
            y += 20;
        }
        epd.setCursor(10, 163); epd.print("Tap: add network");
    }

    epd.setCursor(10, 193); epd.print("Swipe down: back");
    epd.displayBase();
}

static void _drawWifiScanning(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 20);  epd.print("WiFi Setup");
    epd.drawLine(0, 27, 200, 27, 0);
    epd.setCursor(10, 100); epd.print("Scanning...");
    epd.displayBase();
}

static void _drawWifiPortal(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.setCursor(10, 20); epd.print("WiFi Setup");
    epd.drawLine(0, 27, 200, 27, 0);

    epd.setCursor(10, 55);  epd.print("On your phone:");
    epd.setCursor(10, 80);  epd.print("1. Join WiFi:");
    epd.setCursor(22, 100); epd.print("Widget-Setup");
    epd.setCursor(10, 123); epd.print("2. Open browser:");
    epd.setCursor(22, 143); epd.print("192.168.4.1");

    epd.setCursor(10, 180); epd.print("Tap: cancel");
    epd.displayBase();
}

static void _drawWifiSaved(WaveshareEPD& epd, const String& ssid) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 80);  epd.print("Saved!");
    epd.setCursor(10, 108); epd.print(ssid);
    epd.setCursor(10, 148); epd.print("Rebooting...");
    epd.displayBase();
}

// ── Captive portal ────────────────────────────────────────────────────────

static const char _PORTAL_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Widget WiFi</title>
<style>
body{font-family:sans-serif;max-width:380px;margin:28px auto;padding:0 16px}
h2{margin-bottom:20px}
label{display:block;margin:14px 0 4px;font-weight:bold;font-size:15px}
select,input{width:100%;padding:10px;font-size:16px;box-sizing:border-box;
             border:1px solid #ccc;border-radius:6px}
button{width:100%;padding:14px;margin-top:20px;font-size:18px;
       background:#222;color:#fff;border:none;border-radius:8px}
</style>
</head>
<body>
<h2>Widget WiFi Setup</h2>
<form method="POST" action="/save">
<label>Network</label>
<select name="ssid">%NETWORKS%</select>
<label>Password</label>
<input type="password" name="pass" placeholder="WiFi password">
<button type="submit">Save &amp; Connect</button>
</form>
</body></html>
)HTML";

static WebServer  _portalServer(80);
static DNSServer  _portalDns;
static String     _portalNetworkOptions;
static bool       _portalDone      = false;
static bool       _portalCancelled = false;
static String     _portalSavedSsid;

static void _startPortal(WaveshareEPD& epd) {
    _portalDone      = false;
    _portalCancelled = false;
    _portalSavedSsid = "";

    // Scan while still in STA mode (faster than in AP mode)
    _drawWifiScanning(epd);
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    _portalNetworkOptions = "";
    for (int i = 0; i < n; i++) {
        _portalNetworkOptions += "<option value=\"" + WiFi.SSID(i) + "\">"
                               + WiFi.SSID(i) + "</option>\n";
    }

    // Start AP + DNS
    WiFi.mode(WIFI_AP);
    WiFi.softAP(PORTAL_SSID);
    delay(100);
    _portalDns.start(53, "*", WiFi.softAPIP());

    // Routes
    _portalServer.on("/", HTTP_GET, []() {
        String page = FPSTR(_PORTAL_HTML);
        page.replace("%NETWORKS%", _portalNetworkOptions);
        _portalServer.send(200, "text/html", page);
    });

    _portalServer.on("/save", HTTP_POST, []() {
        String ssid = _portalServer.arg("ssid");
        String pass = _portalServer.arg("pass");
        if (ssid.length() > 0) {
            _wifiSaveCred(ssid, pass);
            _portalSavedSsid = ssid;
            _portalServer.send(200, "text/html",
                "<html><body style='font-family:sans-serif;padding:24px'>"
                "<h2>Saved!</h2><p>Connecting to <b>" + ssid +
                "</b>.<br>Device will reboot.</p></body></html>");
            _portalDone = true;
        } else {
            _portalServer.send(400, "text/html",
                "<html><body>Error: no network selected.</body></html>");
        }
    });

    // Captive portal redirect for Android / iOS / Windows probes
    auto redirect = []() {
        _portalServer.sendHeader("Location", "http://192.168.4.1", true);
        _portalServer.send(302, "text/plain", "");
    };
    _portalServer.on("/generate_204",        HTTP_GET, redirect);
    _portalServer.on("/hotspot-detect.html", HTTP_GET, redirect);
    _portalServer.on("/ncsi.txt",            HTTP_GET, redirect);
    _portalServer.onNotFound(redirect);

    _portalServer.begin();
    _drawWifiPortal(epd);

    // Blocking loop until saved or tap-cancelled
    while (!_portalDone && !_portalCancelled) {
        _portalDns.processNextRequest();
        _portalServer.handleClient();

        if (_touchIntFired) {
            _touchIntFired = false;
            TouchResult tr = readTouch();
            if (tr.event == TOUCH_TAP || tr.event == SWIPE_DOWN)
                _portalCancelled = true;
        }
        delay(5);
    }

    _portalServer.stop();
    _portalDns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    if (_portalDone) {
        _drawWifiSaved(epd, _portalSavedSsid);
        delay(2000);
        ESP.restart();
    }
}

// ── Public interface ──────────────────────────────────────────────────────

void screen5Init(WaveshareEPD& epd) {
    _drawWifiStatus(epd);
}

bool updateScreen5(WaveshareEPD& epd, TouchResult tr) {
    if (tr.event == SWIPE_DOWN) return true;
    if (tr.event == TOUCH_TAP) {
        _startPortal(epd);
        _drawWifiStatus(epd);   // redraw status after cancel/reboot
    }
    return false;
}
