// webview.h - live dashboard on a phone. The R4 makes its own Wi-Fi network and serves one page.
//
// Join the Wi-Fi "HDH-Finder" (password in WEB_PASS below), then open http://192.168.4.1
// The page shows what is being monitored (target MAC, maker, channel), the signal in dBm with
// a bar, the closest approach, a graph of the last minute, a warmer/colder hint, a plain
// recommendation, the sniffer's counts and the nearby suspects. It reads /s about once a second, one request at a time.
//
// Usage:  Web::begin();                       // in setup()
//         Web::poll();                        // in loop(): answers phone requests
//         Web::locked(mac, maker);  Web::unlocked();
//         Web::signal(now, avg, channel);     // each T line
//         Web::stats(total, cams, iot, hidden);          // each S line
//         Web::nearbyStart(count);  Web::nearby(n, mac, cat, avg, ch, maker, name);   // B and D lines
#pragma once
#include <Arduino.h>
#include <WiFiS3.h>
#include <stdarg.h>

namespace Web {

static const char *WEB_SSID = "HDH-Finder";
static const char *WEB_PASS = "hunter2026";  // 8+ characters; change it if you like
static const int HIST = 120;                 // graph points, one per 0.5 s = 1 minute
static const int NEAR_MAX = 60;
static const int NEAR_SHORT = 6;  // quick table; the full list is under "All devices"

struct Near { uint8_t mac[6]; char maker[13]; char name[14]; uint8_t cat; int8_t avg; uint8_t ch; };  // 36 bytes: the R4 has little RAM

static WiFiServer server(80);
static bool up = false;
static bool haveLock = false;
static char mac[18] = "";
static char maker[16] = "";
static int raw = -100, avg = -100, ch = 0, peak = -100;
static int modeNow = -1;  // 0 Wi-Fi, 1 Bluetooth, 2 both, as reported by the C5
static int stTotal = 0, stCams = 0, stIot = 0, stHidden = 0;
static uint32_t lastSignal = 0, lastPush = 0, lockedAt = 0, lastStats = 0;
static int8_t hist[HIST];
static int histN = 0;
static Near nearList[NEAR_MAX];
static int nearN = 0;
static char out[512];  // JSON is built in pieces and written when this fills (the R4 has little RAM)

static const char PAGE[] =
  "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'><title>Finder</title>"
  "<style>button{font-size:16px;padding:8px 12px;margin:2px}body{font-family:sans-serif;background:#111;color:#eee;margin:12px}"
  ".c{background:#1c1c1c;border-radius:10px;padding:12px;margin:10px 0}"
  ".big{font-size:64px;font-weight:bold;line-height:1}.m{color:#999;font-size:14px}"
  ".bar{background:#333;height:26px;border-radius:6px}.bar div{height:26px;border-radius:6px;transition:width .4s}"
  ".r{display:flex;justify-content:space-between;gap:8px;text-align:center}.r div{flex:1;background:#262626;border-radius:8px;padding:8px}"
  ".r b{display:block;font-size:22px}table{width:100%;border-collapse:collapse;font-size:14px}"
  "td{padding:5px 3px;border-top:1px solid #333}.t{color:#4c8}.cam{color:#f66}.iot{color:#fc4}"
  "#rec{font-size:18px;border-left:4px solid #4c8}</style>"
  "<div class=c id=lk style='font-weight:bold'>checking the C5 link...</div>"
  "<div class=c><b>Scan for</b> <button id=m0 onclick=sm(0)>Wi-Fi</button> <button id=m1 onclick=sm(1)>Bluetooth</button> <button id=m2 onclick=sm(2)>Both</button>"
  "<div class=m>Bluetooth devices show as BLE in the ch column.</div></div>"
  "<div class=c><div id=st>connecting...</div><div class=m id=tg></div></div>"
  "<div class=c><div class=big id=big>--</div><div class=m id=sub></div><br><div class=bar><div id=bar></div></div>"
  "<p id=pk></p><canvas id=cv width=600 height=170 style='width:100%;background:#222;border-radius:6px'></canvas>"
  "<div class=m>Graph: last minute. -30 dBm is touching it, -90 is far.</div></div>"
  "<div class=c id=rec>...</div>"
  "<div class='c r'><div><b id=s0>-</b>devices</div><div><b id=s1>-</b>camera makers</div>"
  "<div><b id=s2>-</b>IoT chips</div><div><b id=s3>-</b>hidden nets</div></div>"
  "<div class=c><b>Nearby suspects</b> <span class=m>(BOOT column = presses of the C5's BOOT button to lock it; hold BOOT 1 s first to unlock)</span>"
  "<table id=tb></table></div>"
  "<details class=c id=allbox><summary><b>All devices</b> <span class=m id=alln></span></summary><table id=ta></table></details>"
  "<script>"
  "var g=cv.getContext('2d');"
  "function pct(d){return Math.max(0,Math.min(100,(d+90)*100/60))}"
  "function col(p){return p>75?'#4c8':p>40?'#fc4':'#f66'}"
  "function sus(x){return x[1]>0||x[4]=='hidden net'}"
  "function boot(d){var k=0,r=[];d.forEach(x=>{if(sus(x)){k++;r.push(k)}else r.push('')});return r}"
  "function trend(h){if(h.length<12)return 0;var n=h.length,a=0,b=0;for(var i=0;i<6;i++){a+=h[n-1-i];b+=h[n-7-i]}return (a-b)/6}"
  "function sm(v){fetch('/m'+v)}"
  "function tick(){return fetch('/s').then(r=>r.json()).then(j=>{"
  "lk.textContent=j.ls<3000?'C5 link OK':'NO DATA from the C5. Check: C5 powered and blinking, wire C5 D6 -> shifter A2, shifter B2 -> R4 D0, grounds joined, C5 reflashed.';"
  "lk.style.color=j.ls<3000?'#4c8':'#f66';"
  "for(var q=0;q<3;q++){var b=document.getElementById('m'+q);b.style.background=j.mo==q?'#4c8':'';b.style.color=j.mo==q?'#000':''}"
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
  "if(!j.l){var top=j.d[0];m=top?'Not locked. Top suspect: '+top[0]+(top[4]?' ('+top[4]+')':'')+' at '+top[2]+' dBm. '+(sus(top)?'Press BOOT on the C5 to lock it, then walk toward the rising signal.':'Nothing flagged yet; strongest device shown.'):'Not locked. Nothing heard yet: wait 10-20 seconds.'}"
  "else if(lost)m='The target is quiet. Plugs and cameras talk every few seconds: hold still, then move closer.';"
  "else if(j.a>-40)m='Very close. Look and feel within arm\\'s reach, behind and under things.';"
  "else if(d>2)m='Getting warmer ('+d.toFixed(1)+' dB). Keep going this way.';"
  "else if(d<-2)m='Getting colder ('+d.toFixed(1)+' dB). Turn around or try another direction.';"
  "else m='Signal steady. Sweep the antenna slowly left and right and walk toward the strongest side.';"
  "rec.textContent=m;"
  "s0.textContent=j.s[0];s1.textContent=j.s[1];s2.textContent=j.s[2];s3.textContent=j.s[3];"
  "var t='<tr class=m><td>BOOT<td>MAC<td>maker<td>dBm<td>ch</tr>',bk=boot(j.d);"
  "j.d.forEach((x,i)=>{t+='<tr'+(x[0]==j.m?' class=t':'')+'><td>'+bk[i]+'<td>'+x[0]+'<td class='+(x[1]==1?'cam':x[1]==2?'iot':sus(x)?'iot':'')+'>'+(x[4]||'-')+'<td>'+x[2]+'<td>'+(x[3]||'BLE')+'</tr>'});"
  "tb.innerHTML=t;"
  "}).catch(e=>{st.textContent='no connection';st.style.color='#f66'})}"
  "function all(){if(!allbox.open)return 0;return fetch('/d').then(r=>r.json()).then(j=>{alln.textContent='('+j.d.length+' heard in the last minute)';"
  "var t='<tr class=m><td>BOOT<td>MAC<td>maker<td>name<td>dBm<td>ch</tr>',bk=boot(j.d);"
  "j.d.forEach((x,i)=>{t+='<tr'+(x[0]==j.m?' class=t':'')+'><td>'+bk[i]+'<td>'+x[0]+'<td class='+(x[1]==1?'cam':x[1]==2?'iot':sus(x)?'iot':'')+'>'+(x[4]||'-')+'<td>'+(x[5]||'')+'<td>'+x[2]+'<td>'+(x[3]||'BLE')+'</tr>'});"
  "ta.innerHTML=t}).catch(e=>{})}"
  "var n=0;function run(){var p=tick();if(++n%4==0)p=p.then(()=>all());p.finally(()=>setTimeout(run,600))}run();"
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

inline void mode(int m) { modeNow = m; }

inline void stats(int total, int cams, int iot, int hidden) {
  stTotal = total; stCams = cams; stIot = iot; stHidden = hidden;
  lastStats = millis();
}

inline void nearbyStart(int count) { nearN = constrain(count, 0, NEAR_MAX); }

inline void nearby(int n, const char *m, int cat, int a, int c, const char *k, const char *name) {
  if (n < 0 || n >= NEAR_MAX) return;
  Near &d = nearList[n];
  unsigned v[6] = {0};
  sscanf(m, "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
  for (int i = 0; i < 6; i++) d.mac[i] = (uint8_t)v[i];
  copyClean(d.maker, (k[0] == '-' && !k[1]) ? "" : k, sizeof(d.maker));
  copyClean(d.name, (name[0] == '-' && !name[1]) ? "" : name, sizeof(d.name));
  if (!d.maker[0] && !strcmp(name, "(hidden)")) strcpy(d.maker, "hidden net");  // flagged like a maker match
  d.cat = cat;
  d.avg = a;
  d.ch = c;
  if (n >= nearN) nearN = n + 1;
}

static void (*pump)() = nullptr;  // reads the sniffer link; called between slow Wi-Fi writes so no lines are lost
inline void setPump(void (*f)()) { pump = f; }

static void sendAll(WiFiClient &c, const char *p, size_t len) {
  while (len) {
    size_t k = len > 256 ? 256 : len;
    c.write((const uint8_t *)p, k);
    if (pump) pump();
    p += k;
    len -= k;
  }
}

static const char *macText(const uint8_t *m) {
  static char t[18];
  snprintf(t, sizeof(t), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return t;
}

static int outN = 0;
static void emit(WiFiClient &c, const char *fmt, ...) {  // small pieces into `out`, written when it fills
  char tmp[200];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  if (n < 0) return;
  if (n > (int)sizeof(tmp) - 1) n = sizeof(tmp) - 1;
  if (outN + n > (int)sizeof(out)) { sendAll(c, out, outN); outN = 0; }
  memcpy(out + outN, tmp, n);
  outN += n;
}
static void flushOut(WiFiClient &c) { if (outN) sendAll(c, out, outN); outN = 0; }

static const char JSON_HEAD[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";

inline void poll() {
  if (!up) return;
  WiFiClient c = server.available();
  if (!c) return;
  char req[40] = "";
  uint8_t n = 0;
  uint32_t t0 = millis();
  while (c.connected() && millis() - t0 < 100) {  // first line only: "GET /s HTTP/1.1"
    if (!c.available()) { if (pump) pump(); continue; }
    char b = c.read();
    if (b == '\n') break;
    if (n < sizeof(req) - 1) req[n++] = b;
  }
  outN = 0;
  const char *mq = strstr(req, "GET /m");
  if (mq != nullptr && mq[6] >= '0' && mq[6] <= '2') {  // mode buttons: pass M,<n> on to the C5
    Serial1.print("M,");
    Serial1.println(mq[6]);
    emit(c, "%s", JSON_HEAD);
    emit(c, "{}");
    flushOut(c);
  } else if (strstr(req, "GET /s") != nullptr) {
    uint32_t age = lastSignal ? millis() - lastSignal : 99999;
    emit(c, "%s", JSON_HEAD);
    emit(c, "{\"mo\":%d,\"l\":%d,\"n\":%d,\"m\":\"%s\",\"k\":\"%s\",\"a\":%d,\"r\":%d,\"p\":%d,\"c\":%d,\"age\":%lu,\"ls\":%lu,",
         modeNow, haveLock ? 1 : 0, lastSignal ? 1 : 0, mac, maker, avg, raw, peak, ch, (unsigned long)age,
         (unsigned long)(lastStats ? millis() - lastStats : 99999));
    emit(c, "\"s\":[%d,%d,%d,%d],\"h\":[", stTotal, stCams, stIot, stHidden);
    for (int i = 0; i < histN; i++) emit(c, "%s%d", i ? "," : "", (int)hist[i]);
    emit(c, "],\"d\":[");
    for (int i = 0; i < nearN && i < NEAR_SHORT; i++)
      emit(c, "%s[\"%s\",%u,%d,%u,\"%s\"]", i ? "," : "", macText(nearList[i].mac), nearList[i].cat,
           (int)nearList[i].avg, nearList[i].ch, nearList[i].maker);
    emit(c, "]}");
    flushOut(c);
  } else if (strstr(req, "GET /d") != nullptr) {
    emit(c, "%s", JSON_HEAD);
    emit(c, "{\"m\":\"%s\",\"d\":[", mac);
    for (int i = 0; i < nearN; i++)
      emit(c, "%s[\"%s\",%u,%d,%u,\"%s\",\"%s\"]", i ? "," : "", macText(nearList[i].mac), nearList[i].cat,
           (int)nearList[i].avg, nearList[i].ch, nearList[i].maker, nearList[i].name);
    emit(c, "]}");
    flushOut(c);
  } else if (strstr(req, "GET / ") == nullptr) {  // favicon and other stray requests: answer fast, not with the whole page
    const char *nf = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    sendAll(c, nf, strlen(nf));
  } else {
    const char *head = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
    sendAll(c, head, strlen(head));
    sendAll(c, PAGE, strlen(PAGE));
  }
  c.stop();
}

}  // namespace Web
