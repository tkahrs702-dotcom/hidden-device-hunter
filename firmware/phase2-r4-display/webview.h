// webview.h - live dashboard on a phone. The R4 makes its own Wi-Fi network and serves one page.
//
// Join the Wi-Fi "HDH-Finder" (password in WEB_PASS below), then open http://192.168.4.1
// The page shows what is being monitored (target MAC, maker, channel), the signal in dBm with
// a bar, the closest approach, a graph of the last minute, a warmer/colder hint, a plain
// recommendation, the sniffer's counts and the nearby suspects. It reads /s twice a second.
//
// Usage:  Web::begin();                       // in setup()
//         Web::poll();                        // in loop(): answers phone requests
//         Web::locked(mac, maker);  Web::unlocked();
//         Web::signal(now, avg, channel);     // each T line
//         Web::stats(total, cams, iot, hidden);          // each S line
//         Web::nearbyStart(count);  Web::nearby(n, mac, cat, avg, ch, maker);   // B and D lines
#pragma once
#include <Arduino.h>
#include <WiFiS3.h>

namespace Web {

static const char *WEB_SSID = "HDH-Finder";
static const char *WEB_PASS = "hunter2026";  // 8+ characters; change it if you like
static const int HIST = 120;                 // graph points, one per 0.5 s = 1 minute
static const int NEAR_MAX = 6;

struct Near { char mac[18]; char maker[24]; uint8_t cat; int8_t avg; uint8_t ch; };

static WiFiServer server(80);
static bool up = false;
static bool haveLock = false;
static char mac[18] = "";
static char maker[24] = "";
static int raw = -100, avg = -100, ch = 0, peak = -100;
static int stTotal = 0, stCams = 0, stIot = 0, stHidden = 0;
static uint32_t lastSignal = 0, lastPush = 0, lockedAt = 0;
static int8_t hist[HIST];
static int histN = 0;
static Near nearList[NEAR_MAX];
static int nearN = 0;
static char out[1500];  // one JSON reply, sent in a single write (each WiFi write is slow)

static const char PAGE[] =
  "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'><title>Finder</title>"
  "<style>body{font-family:sans-serif;background:#111;color:#eee;margin:12px}"
  ".c{background:#1c1c1c;border-radius:10px;padding:12px;margin:10px 0}"
  ".big{font-size:64px;font-weight:bold;line-height:1}.m{color:#999;font-size:14px}"
  ".bar{background:#333;height:26px;border-radius:6px}.bar div{height:26px;border-radius:6px;transition:width .4s}"
  ".r{display:flex;justify-content:space-between;gap:8px;text-align:center}.r div{flex:1;background:#262626;border-radius:8px;padding:8px}"
  ".r b{display:block;font-size:22px}table{width:100%;border-collapse:collapse;font-size:14px}"
  "td{padding:5px 3px;border-top:1px solid #333}.t{color:#4c8}.cam{color:#f66}.iot{color:#fc4}"
  "#rec{font-size:18px;border-left:4px solid #4c8}</style>"
  "<div class=c><div id=st>connecting...</div><div class=m id=tg></div></div>"
  "<div class=c><div class=big id=big>--</div><div class=m id=sub></div><br><div class=bar><div id=bar></div></div>"
  "<p id=pk></p><canvas id=cv width=600 height=170 style='width:100%;background:#222;border-radius:6px'></canvas>"
  "<div class=m>Graph: last minute. -30 dBm is touching it, -90 is far.</div></div>"
  "<div class=c id=rec>...</div>"
  "<div class='c r'><div><b id=s0>-</b>devices</div><div><b id=s1>-</b>camera makers</div>"
  "<div><b id=s2>-</b>IoT chips</div><div><b id=s3>-</b>hidden nets</div></div>"
  "<div class=c><b>Nearby suspects</b> <span class=m>(press BOOT on the C5 to lock the next one)</span>"
  "<table id=tb></table></div>"
  "<script>"
  "var g=cv.getContext('2d');"
  "function pct(d){return Math.max(0,Math.min(100,(d+90)*100/60))}"
  "function col(p){return p>75?'#4c8':p>40?'#fc4':'#f66'}"
  "function trend(h){if(h.length<12)return 0;var n=h.length,a=0,b=0;for(var i=0;i<6;i++){a+=h[n-1-i];b+=h[n-7-i]}return (a-b)/6}"
  "function tick(){fetch('/s').then(r=>r.json()).then(j=>{"
  "var heard=j.l&&j.n&&j.age<3000,lost=j.l&&!heard;"
  "st.textContent=!j.l?'Not locked: scanning all channels':(lost?'Target not heard':'Tracking target');"
  "st.style.color=!j.l?'#fc4':(lost?'#f66':'#4c8');"
  "tg.textContent=j.l?j.m+'  '+(j.k||'no maker match')+(j.c?'  channel '+j.c:''):'Pick a target on the C5 (BOOT button) or type its MAC';"
  "big.textContent=heard?j.a+' dBm':'--';"
  "sub.textContent=heard?'now '+j.r+' dBm   '+Math.round(pct(j.a))+'% strength':'';"
  "bar.style.width=(heard?pct(j.a):0)+'%';bar.style.background=col(pct(j.a));"
  "pk.textContent=(j.l&&j.n)?'Closest so far: '+j.p+' dBm ('+Math.round(pct(j.p))+'%)':'';"
  "g.clearRect(0,0,600,170);g.strokeStyle='#333';for(var y=0;y<=170;y+=34){g.beginPath();g.moveTo(0,y);g.lineTo(600,y);g.stroke()}"
  "g.strokeStyle='#4c8';g.lineWidth=2;g.beginPath();j.h.forEach((v,i)=>{var x=i*600/119,y=170-pct(v)*1.7;i?g.lineTo(x,y):g.moveTo(x,y)});g.stroke();"
  "var d=trend(j.h),m;"
  "if(!j.l){var top=j.d[0];m=top?'Not locked. Strongest suspect: '+top[0]+(top[4]?' ('+top[4]+')':'')+' at '+top[2]+' dBm. Press BOOT on the C5 to lock it.':'Not locked. Nothing heard yet: wait 10-20 seconds.'}"
  "else if(lost)m='The target is quiet. Plugs and cameras talk every few seconds: hold still, then move closer.';"
  "else if(j.a>-40)m='Very close. Look and feel within arm\\'s reach, behind and under things.';"
  "else if(d>2)m='Getting warmer ('+d.toFixed(1)+' dB). Keep going this way.';"
  "else if(d<-2)m='Getting colder ('+d.toFixed(1)+' dB). Turn around or try another direction.';"
  "else m='Signal steady. Sweep the antenna slowly left and right and walk toward the strongest side.';"
  "rec.textContent=m;"
  "s0.textContent=j.s[0];s1.textContent=j.s[1];s2.textContent=j.s[2];s3.textContent=j.s[3];"
  "var t='<tr class=m><td>MAC<td>maker<td>dBm<td>ch</tr>';"
  "j.d.forEach(x=>{t+='<tr'+(x[0]==j.m?' class=t':'')+'><td>'+x[0]+'<td class='+(x[1]==1?'cam':x[1]==2?'iot':'')+'>'+(x[4]||'-')+'<td>'+x[2]+'<td>'+x[3]+'</tr>'});"
  "tb.innerHTML=t;"
  "}).catch(e=>{st.textContent='no connection';st.style.color='#f66'})}"
  "setInterval(tick,500);tick();"
  "</script>";

static void copyClean(char *dst, const char *src, size_t n) {  // keeps JSON valid
  size_t i = 0;
  for (; src[i] && i < n - 1; i++) dst[i] = (src[i] == '"' || src[i] == '\\') ? '_' : src[i];
  dst[i] = 0;
}

inline void begin() {
  if (WiFi.status() == WL_NO_MODULE) return;
  if (WiFi.beginAP(WEB_SSID, WEB_PASS) != WL_AP_LISTENING) return;
  server.begin();
  up = true;
}

inline void locked(const char *m, const char *k) {
  haveLock = true;
  copyClean(mac, m, sizeof(mac));
  copyClean(maker, k, sizeof(maker));
  peak = -100;
  histN = 0;
  lastSignal = 0;
  lockedAt = millis();
}

inline void unlocked() { haveLock = false; }

inline void signal(int now, int a, int c) {
  raw = now;
  avg = a;
  ch = c;
  if (a > peak) peak = a;
  uint32_t t = millis();
  lastSignal = t;
  if (t - lastPush >= 500) {
    lastPush = t;
    if (histN < HIST) hist[histN++] = (int8_t)a;
    else { memmove(hist, hist + 1, HIST - 1); hist[HIST - 1] = (int8_t)a; }
  }
}

inline void stats(int total, int cams, int iot, int hidden) {
  stTotal = total; stCams = cams; stIot = iot; stHidden = hidden;
}

inline void nearbyStart(int count) { nearN = constrain(count, 0, NEAR_MAX); }

inline void nearby(int n, const char *m, int cat, int a, int c, const char *k) {
  if (n < 0 || n >= NEAR_MAX) return;
  Near &d = nearList[n];
  copyClean(d.mac, m, sizeof(d.mac));
  copyClean(d.maker, k, sizeof(d.maker));
  d.cat = cat;
  d.avg = a;
  d.ch = c;
  if (n >= nearN) nearN = n + 1;
}

static void sendAll(WiFiClient &c, const char *p, size_t len) {
  while (len) {
    size_t k = len > 512 ? 512 : len;
    c.write((const uint8_t *)p, k);
    p += k;
    len -= k;
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
    char b = c.read();
    if (b == '\n') break;
    if (n < sizeof(req) - 1) req[n++] = b;
  }
  if (strstr(req, "GET /s") != nullptr) {
    uint32_t age = lastSignal ? millis() - lastSignal : 99999;
    int o = snprintf(out, sizeof(out),
                     "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n"
                     "{\"l\":%d,\"n\":%d,\"m\":\"%s\",\"k\":\"%s\",\"a\":%d,\"r\":%d,\"p\":%d,\"c\":%d,\"age\":%lu,"
                     "\"s\":[%d,%d,%d,%d],\"h\":[",
                     haveLock ? 1 : 0, lastSignal ? 1 : 0, mac, maker, avg, raw, peak, ch, (unsigned long)age,
                     stTotal, stCams, stIot, stHidden);
    for (int i = 0; i < histN && o < (int)sizeof(out) - 700; i++)
      o += snprintf(out + o, sizeof(out) - o, "%s%d", i ? "," : "", (int)hist[i]);
    o += snprintf(out + o, sizeof(out) - o, "],\"d\":[");
    for (int i = 0; i < nearN && o < (int)sizeof(out) - 100; i++)
      o += snprintf(out + o, sizeof(out) - o, "%s[\"%s\",%u,%d,%u,\"%s\"]", i ? "," : "", nearList[i].mac, nearList[i].cat,
                    (int)nearList[i].avg, nearList[i].ch, nearList[i].maker);
    o += snprintf(out + o, sizeof(out) - o, "]}");
    if (o > (int)sizeof(out) - 1) o = sizeof(out) - 1;
    sendAll(c, out, o);
  } else {
    const char *head = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
    sendAll(c, head, strlen(head));
    sendAll(c, PAGE, strlen(PAGE));
  }
  c.stop();
}

}  // namespace Web
