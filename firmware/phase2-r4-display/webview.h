// webview.h - live readout on a phone. The R4 makes its own Wi-Fi network and serves one page.
//
// Join the Wi-Fi "HDH-Finder" (password in WEB_PASS below), then open http://192.168.4.1
// The page shows the signal in dBm, a big bar, the closest approach since the lock and a
// graph of the last minute. It reads /s twice a second.
//
// Usage:  Web::begin();                       // in setup()
//         Web::poll();                        // in loop(): answers phone requests
//         Web::locked(mac, maker);            // sniffer locked a target (resets closest approach)
//         Web::unlocked();
//         Web::signal(avgDbm, channel);       // each T line
#pragma once
#include <Arduino.h>
#include <WiFiS3.h>

namespace Web {

static const char *WEB_SSID = "HDH-Finder";
static const char *WEB_PASS = "hunter2026";  // 8+ characters; change it if you like
static const int HIST = 120;                 // graph points, one per 0.5 s = 1 minute

static WiFiServer server(80);
static bool up = false;
static bool haveLock = false;
static char mac[18] = "";
static char maker[24] = "";
static int avg = -100, ch = 0, peak = -100;
static uint32_t lastSignal = 0;
static uint32_t lastPush = 0;
static int8_t hist[HIST];
static int histN = 0;

static const char PAGE[] =
  "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
  "<title>Finder</title><body style='font-family:sans-serif;background:#111;color:#eee;margin:16px'>"
  "<div id=st style='font-size:18px'>connecting...</div>"
  "<div id=big style='font-size:72px;font-weight:bold;margin:8px 0'>--</div>"
  "<div style='background:#333;height:28px;border-radius:6px'><div id=bar style='height:28px;width:0;background:#4c8;border-radius:6px'></div></div>"
  "<p id=pk style='font-size:20px'></p>"
  "<canvas id=cv width=600 height=160 style='width:100%;background:#222;border-radius:6px'></canvas>"
  "<p style='color:#888'>-30 dBm is touching it, -90 is far. Graph: last minute.</p>"
  "<script>"
  "var cv=document.getElementById('cv'),g=cv.getContext('2d');"
  "function pct(d){return Math.max(0,Math.min(100,(d+90)*100/60))}"
  "function tick(){fetch('/s').then(r=>r.json()).then(j=>{"
  "var lost=j.l&&j.age>3000;"
  "st.textContent=!j.l?'Not locked (scanning)':(lost?'Target not heard':'Locked '+j.m+' '+j.k+' ch'+j.c);"
  "big.textContent=(j.l&&j.n)?j.a+' dBm':'--';"
  "bar.style.width=(j.l&&j.n?pct(j.a):0)+'%';bar.style.background=lost?'#c44':'#4c8';"
  "pk.textContent=(j.l&&j.n)?'Closest so far: '+j.p+' dBm ('+Math.round(pct(j.p))+'%)':'';"
  "g.clearRect(0,0,600,160);g.strokeStyle='#4c8';g.beginPath();"
  "j.h.forEach((v,i)=>{var x=i*600/119,y=160-pct(v)*1.6;i?g.lineTo(x,y):g.moveTo(x,y)});g.stroke();"
  "}).catch(e=>{st.textContent='no connection'})}"
  "setInterval(tick,500);tick();"
  "</script>";

inline void begin() {
  if (WiFi.status() == WL_NO_MODULE) return;
  if (WiFi.beginAP(WEB_SSID, WEB_PASS) != WL_AP_LISTENING) return;
  server.begin();
  up = true;
}

inline void locked(const char *m, const char *k) {
  haveLock = true;
  strncpy(mac, m, sizeof(mac) - 1);
  strncpy(maker, k, sizeof(maker) - 1);
  peak = -100;
  histN = 0;
  lastSignal = 0;
}

inline void unlocked() { haveLock = false; }

inline void signal(int a, int c) {
  avg = a;
  ch = c;
  if (a > peak) peak = a;
  uint32_t now = millis();
  lastSignal = now;
  if (now - lastPush >= 500) {
    lastPush = now;
    if (histN < HIST) hist[histN++] = (int8_t)a;
    else { memmove(hist, hist + 1, HIST - 1); hist[HIST - 1] = (int8_t)a; }
  }
}

inline void poll() {
  if (!up) return;
  WiFiClient c = server.available();
  if (!c) return;
  char req[40] = "";
  uint8_t n = 0;
  uint32_t t0 = millis();
  while (c.connected() && millis() - t0 < 100) {  // first line only: "GET /s HTTP/1.1"
    if (!c.available()) continue;
    char ch1 = c.read();
    if (ch1 == '\n') break;
    if (n < sizeof(req) - 1) req[n++] = ch1;
  }
  bool wantJson = strstr(req, "GET /s") != nullptr;
  if (wantJson) {
    c.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n");
    char head[160];
    uint32_t age = lastSignal ? millis() - lastSignal : 99999;
    snprintf(head, sizeof(head), "{\"l\":%d,\"n\":%d,\"m\":\"%s\",\"k\":\"%s\",\"a\":%d,\"p\":%d,\"c\":%d,\"age\":%lu,\"h\":[",
             haveLock ? 1 : 0, lastSignal ? 1 : 0, mac, maker, avg, peak, ch, (unsigned long)age);
    c.print(head);
    for (int i = 0; i < histN; i++) {
      c.print((int)hist[i]);
      if (i < histN - 1) c.print(",");
    }
    c.print("]}");
  } else {
    c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
    c.print(PAGE);
  }
  c.stop();
}

}  // namespace Web
