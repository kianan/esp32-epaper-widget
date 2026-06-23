#pragma once
#include <WebServer.h>
#include <DNSServer.h>
#include <qrcode.h>
#include "WaveshareEPD.h"
#include "screen_menu.h"
#include "touch.h"
#include "wifi_conn.h"
#include <Fonts/FreeSansBold9pt7b.h>

namespace wifi {

#define PORTAL_SSID  "Widget-Setup"
#define QR_MAX_VER   6
#define QR_BUF_SIZE  215

static void _drawStatus(WaveshareEPD& epd)
{
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
        epd.setCursor(10, 78);  epd.print(ssid);
        epd.setCursor(10, 103); epd.print(WiFi.localIP().toString());
        epd.setCursor(10, 140); epd.print("Tap: add network");
        epd.setCursor(10, 163); epd.print("Swipe up: share WiFi");
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
        epd.setCursor(10, 180); epd.print("Swipe up: share WiFi");
    }

    epd.setCursor(10, 193); epd.print("Swipe right: back");
    epd.displayBase();
}

static void _drawScanning(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 20);  epd.print("WiFi Setup");
    epd.drawLine(0, 27, 200, 27, 0);
    epd.setCursor(10, 100); epd.print("Scanning...");
    epd.displayBase();
}

static void _drawSaved(WaveshareEPD& epd, const String& ssid)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 80);  epd.print("Saved!");
    epd.setCursor(10, 108); epd.print(ssid);
    epd.setCursor(10, 148); epd.print("Rebooting...");
    epd.displayBase();
}

static void _drawQR(WaveshareEPD& epd, const char* text,
                    const char* title, const char* hint = nullptr)
{
    QRCode qr;
    uint8_t buf[QR_BUF_SIZE];
    bool ok = false;
    for (uint8_t v = 2; v <= QR_MAX_VER; v++) {
        if (qrcode_initText(&qr, buf, v, ECC_LOW, text) == 0) { ok = true; break; }
    }

    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 18); epd.print(title);
    epd.drawLine(0, 25, 200, 25, 0);

    if (!ok) {
        epd.setFont(NULL); epd.setTextSize(1);
        epd.setCursor(10, 50); epd.print("Credentials too long");
        epd.setCursor(10, 62); epd.print("for QR code.");
        if (hint) { epd.setCursor(5, 193); epd.print(hint); }
        epd.displayBase();
        return;
    }

    int availPx = hint ? 158 : 172;
    int scale   = availPx / qr.size;
    if (scale < 2) scale = 2;
    int qrPx = qr.size * scale;
    int xOff = (200 - qrPx) / 2;
    int yOff = 28 + (availPx - qrPx) / 2;

    epd.fillRect(xOff - scale, yOff - scale, qrPx + scale * 2, qrPx + scale * 2, 1);
    for (int y = 0; y < qr.size; y++)
        for (int x = 0; x < qr.size; x++)
            if (qrcode_getModule(&qr, x, y))
                epd.fillRect(xOff + x * scale, yOff + y * scale, scale, scale, 0);

    if (hint) {
        epd.setFont(NULL); epd.setTextSize(1);
        epd.setCursor(5, 193); epd.print(hint);
    }
    epd.displayBase();
}

static void _drawPortal(WaveshareEPD& epd)
{
    _drawQR(epd, "WIFI:T:nopass;S:" PORTAL_SSID ";;",
            "Scan to join AP", "Then: 192.168.4.1  Tap:back");
}

static void _drawShareQR(WaveshareEPD& epd)
{
    String ssid = WiFi.SSID();
    String pass;
    auto creds = _wifiLoadCreds();
    for (auto& c : creds) { if (c.ssid == ssid) { pass = c.pass; break; } }
    _drawQR(epd, ("WIFI:T:WPA;S:" + ssid + ";P:" + pass + ";;").c_str(),
            "Share WiFi", "Any touch: back");
}

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

static WebServer _portalServer(80);
static DNSServer _portalDns;
static String    _portalNetworkOptions;
static bool      _portalDone      = false;
static bool      _portalCancelled = false;
static String    _portalSavedSsid;

static void _startPortal(WaveshareEPD& epd)
{
    _portalDone      = false;
    _portalCancelled = false;
    _portalSavedSsid = "";

    _drawScanning(epd);
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    _portalNetworkOptions = "";
    for (int i = 0; i < n; i++) {
        _portalNetworkOptions += "<option value=\"" + WiFi.SSID(i) + "\">"
                               + WiFi.SSID(i) + "</option>\n";
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(PORTAL_SSID);
    delay(100);
    _portalDns.start(53, "*", WiFi.softAPIP());

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

    auto redirect = []() {
        _portalServer.sendHeader("Location", "http://192.168.4.1", true);
        _portalServer.send(302, "text/plain", "");
    };
    _portalServer.on("/generate_204",        HTTP_GET, redirect);
    _portalServer.on("/hotspot-detect.html", HTTP_GET, redirect);
    _portalServer.on("/ncsi.txt",            HTTP_GET, redirect);
    _portalServer.onNotFound(redirect);

    _portalServer.begin();
    _drawPortal(epd);

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
        _drawSaved(epd, _portalSavedSsid);
        delay(2000);
        ESP.restart();
    }
}

void screenInit(WaveshareEPD& epd)
{
    _drawStatus(epd);
    if (!wifiConnected()) {
        wifiBegin();
        if (wifiConnected()) _drawStatus(epd);
    }
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    static bool _showingShareQR = false;

    if (_showingShareQR) {
        if (tr.event != TOUCH_NONE) {
            _showingShareQR = false;
            _drawStatus(epd);
        }
        return false;
    }

    if (tr.event == SWIPE_RIGHT) return true;
    if (tr.event == SWIPE_UP && wifiConnected()) {
        _showingShareQR = true;
        _drawShareQR(epd);
        return false;
    }
    if (tr.event == TOUCH_TAP) {
        _startPortal(epd);
        _drawStatus(epd);
    }
    return false;
}

} // namespace wifi
