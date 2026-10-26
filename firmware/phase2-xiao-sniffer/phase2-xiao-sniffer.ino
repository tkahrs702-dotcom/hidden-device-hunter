// Hidden Device Hunter - Phase 2: Wi-Fi sniffer + directional antenna
// Board: Seeed XIAO ESP32-C5 (FQBN esp32:esp32:XIAO_ESP32C5), XIAO ESP32-C3 or XIAO ESP32-S3,
//        esp32 core 3.3.x. The same D6/D7 pins and wiring work on all three.
// Antenna: u.FL -> RP-SMA pigtail -> Alfa APA-M04 panel (aim the flat face)
// Link to the R4: D6 (TX) -> level shifter A2 ... B2 -> R4 D0 (RX)
//                 D7 (RX) <- level shifter A1 ... B1 <- R4 D1 (TX)   (not used yet, wired for later)
//
// What it does
//   Listens on every 2.4 GHz channel, and on the C5 every 5 GHz channel too (receive only,
//   it never transmits), and records every
//   radio it hears: routers, phones, cameras, plugs. Each one gets its signal strength,
//   channel, packet count and, from the first half of its MAC address, its maker.
//   Makers of cameras/doorbells are tagged CAM; Wi-Fi chips used in cheap cameras and
//   smart gadgets are tagged IOT.
//   Lock onto one device and the XIAO stays on its channel and streams its signal to the
//   R4, which beeps and shows bars. Point the panel antenna around: the signal peaks when
//   the flat face points at the device.
//
// USB Serial Monitor: 115200 baud, line ending "Newline".
//   l          list devices heard in the last 60 s, strongest first
//   c          list only CAM / IOT / hidden devices
//   <number>   lock onto that device from the last list
//   m <MAC>    lock onto a MAC typed in full, e.g.  m 24:0A:C4:12:34:56
//   u          unlock (go back to scanning all channels)
//   h          help
// BOOT button on the XIAO (works away from the computer; GPIO28 on the C5, GPIO0 on the S3):
//   short press = lock the next suspect (CAM first, then IOT, then strongest)
//   hold 1 s    = unlock
// The lock is saved, so the XIAO picks it up again after a power cycle.
// Status LED (C5 only): one short blink every 2 s = scanning, fast blinking = locked target heard,
// blink every 0.5 s = locked but target not heard.

#include <WiFi.h>
#include <Preferences.h>
#include "esp_wifi.h"
#include "oui_table.h"

// ---------- settings you can change ----------
// US channels. On a dual-band chip (ESP32-C5) the 5 GHz ones are added. 52-144 are DFS
// channels; listening there is fine (the board never transmits). Delete them to sweep faster.
static const uint8_t CHANNELS[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
#if SOC_WIFI_SUPPORT_5G
  36, 40, 44, 48, 149, 153, 157, 161, 165,                                // 5 GHz
  52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144,  // 5 GHz DFS
#endif
};
static const uint8_t NUM_CH = sizeof(CHANNELS);
static const uint32_t HOP_MS = 220;          // time on each channel while scanning
static const uint32_t LOST_MS = 4000;        // locked target silent this long -> search all channels
static const uint32_t LIST_AGE_MS = 60000;   // "recent" = heard in the last 60 s
#if CONFIG_IDF_TARGET_ESP32C5
static const int BUTTON_PIN = 28;            // BOOT button
static const char *BOARD_NAME = "XIAO ESP32-C5";
#elif CONFIG_IDF_TARGET_ESP32C3
static const int BUTTON_PIN = 9;
static const char *BOARD_NAME = "XIAO ESP32-C3";
#else
static const int BUTTON_PIN = 0;
static const char *BOARD_NAME = "XIAO ESP32-S3";
#endif
static const int LINK_RX = D7;
static const int LINK_TX = D6;
static const uint32_t LINK_BAUD = 115200;
// Target to lock on the first boot after flashing, so no typing is needed in the field.
// Set to "" to start unlocked. Change DEFAULT_LOCK_VER whenever you change the MAC, so it applies once.
static const char *DEFAULT_LOCK = "10:5A:17:6C:02:4D";  // Taelor's Tuya device
static const uint8_t DEFAULT_LOCK_VER = 1;
// ---------------------------------------------

enum Kind : uint8_t { K_OTHER = 0, K_BEACON = 1, K_PROBE_REQ = 2, K_PROBE_RESP = 3 };
enum Flags : uint8_t { F_AP = 1, F_CLIENT = 2, F_HIDDEN = 4, F_RANDOM = 8 };

struct Pkt {
  uint8_t mac[6];
  int8_t rssi;
  uint8_t ch;
  uint8_t kind;
  char ssid[33];
};

struct Dev {
  bool used;
  uint8_t mac[6];
  int8_t last;
  float avg;
  uint8_t ch;
  uint32_t pkts;
  uint32_t lastSeen;
  uint8_t flags;
  uint8_t cat;    // 0 none, 1 CAM, 2 IOT
  uint8_t label;  // index into OUI_LABELS
  char ssid[33];
};

static const int MAX_DEVS = 160;
static Dev devs[MAX_DEVS];
static QueueHandle_t pktQueue;
static Preferences prefs;

static bool locked = false;
static uint8_t lockMac[6];
static int lockIdx = -1;
static uint32_t lockLastHeard = 0;
static float lockAvg = 0;
static bool lockAvgValid = false;

static uint8_t chIndex = 0;
static bool chBad[NUM_CH];                   // channels the radio refused (skipped while hopping)
static uint32_t lastHop = 0;
static uint8_t currentCh = 1;

static int lastList[64];
static int lastListCount = 0;
static int cycleCursor = -1;

// ---------- radio callback (runs in the Wi-Fi task: keep it short) ----------
static void sniff(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;
  const wifi_promiscuous_pkt_t *p = (const wifi_promiscuous_pkt_t *)buf;
  const uint8_t *f = p->payload;
  uint16_t len = p->rx_ctrl.sig_len;
  if (len < 24) return;
  Pkt k;
  memcpy(k.mac, f + 10, 6);           // address 2 = the radio that sent this frame
  if (k.mac[0] & 0x01) return;        // group address, not a real device
  k.rssi = p->rx_ctrl.rssi;
  k.ch = p->rx_ctrl.channel;
  k.kind = K_OTHER;
  k.ssid[0] = 0;
  uint8_t ftype = (f[0] >> 2) & 0x3;
  uint8_t sub = (f[0] >> 4) & 0xF;
  if (ftype == 0) {
    if (sub == 8 || sub == 5) {       // beacon / probe response: an access point
      k.kind = (sub == 8) ? K_BEACON : K_PROBE_RESP;
      if (len > 38 && f[36] == 0) {   // tag 0 = network name
        uint8_t l = f[37];
        if (l > 32) l = 32;
        if (38 + l <= len) {
          memcpy(k.ssid, f + 38, l);
          k.ssid[l] = 0;
        }
      }
    } else if (sub == 4) {
      k.kind = K_PROBE_REQ;           // a phone/gadget asking "is my network here?"
    }
  }
  xQueueSend(pktQueue, &k, 0);
}

// ---------- helpers ----------
static void macStr(const uint8_t *m, char *out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

static bool parseMac(const char *s, uint8_t *out) {
  unsigned v[6];
  if (sscanf(s, "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) != 6) return false;
  for (int i = 0; i < 6; i++) out[i] = (uint8_t)v[i];
  return true;
}

static void lookupVendor(const uint8_t *mac, uint8_t &cat, uint8_t &label) {
  cat = 0;
  label = 0;
  if (mac[0] & 0x02) return;  // locally administered (random) address: no maker info
  uint32_t oui = ((uint32_t)mac[0] << 16) | ((uint32_t)mac[1] << 8) | mac[2];
  int lo = 0, hi = OUI_COUNT - 1;
  while (lo <= hi) {
    int mid = (lo + hi) / 2;
    if (OUI_TABLE[mid].oui == oui) {
      cat = OUI_TABLE[mid].cat;
      label = OUI_TABLE[mid].label;
      return;
    }
    if (OUI_TABLE[mid].oui < oui) lo = mid + 1;
    else hi = mid - 1;
  }
}

static const char *catName(uint8_t c) { return c == 1 ? "CAM" : (c == 2 ? "IOT" : ""); }

static int findDev(const uint8_t *mac) {
  for (int i = 0; i < MAX_DEVS; i++)
    if (devs[i].used && memcmp(devs[i].mac, mac, 6) == 0) return i;
  return -1;
}

static int addDev(const uint8_t *mac) {
  int slot = -1;
  uint32_t oldest = UINT32_MAX;
  for (int i = 0; i < MAX_DEVS; i++) {
    if (!devs[i].used) { slot = i; break; }
    if (devs[i].lastSeen < oldest && !(locked && memcmp(devs[i].mac, lockMac, 6) == 0)) {
      oldest = devs[i].lastSeen;
      slot = i;
    }
  }
  Dev &d = devs[slot];
  memset(&d, 0, sizeof(d));
  d.used = true;
  memcpy(d.mac, mac, 6);
  d.avg = -100;
  if (mac[0] & 0x02) d.flags |= F_RANDOM;
  lookupVendor(mac, d.cat, d.label);
  return slot;
}

static bool validChannel(uint8_t ch) {
#if SOC_WIFI_SUPPORT_5G
  if (ch >= 32 && ch <= 177) return true;
#endif
  return ch >= 1 && ch <= 14;
}

static esp_err_t setChannel(uint8_t ch) {
  if (!validChannel(ch)) return ESP_ERR_INVALID_ARG;
  esp_err_t err = esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  if (err == ESP_OK) currentCh = ch;
  return err;
}

// Next channel in the sweep. A channel the radio refuses is reported once and skipped after.
static void hopNext() {
  for (int tries = 0; tries < NUM_CH; tries++) {
    chIndex = (chIndex + 1) % NUM_CH;
    if (chBad[chIndex]) continue;
    esp_err_t err = setChannel(CHANNELS[chIndex]);
    if (err == ESP_OK) return;
    if (err != ESP_ERR_INVALID_ARG) return;  // temporary failure: try again next hop
    chBad[chIndex] = true;
    Serial.printf("Channel %u not allowed by the radio, skipping it.\n", CHANNELS[chIndex]);
  }
}

// ---------- link to the R4 ----------
static void sendLock() {
  char mac[18];
  macStr(lockMac, mac);
  int i = findDev(lockMac);
  uint8_t cat = 0, label = 0;
  if (i >= 0) { cat = devs[i].cat; label = devs[i].label; }
  else lookupVendor(lockMac, cat, label);
  Serial1.printf("L,%s,%u,%s\n", mac, cat, cat ? OUI_LABELS[label] : "");
}

static void lockOn(const uint8_t *mac) {
  memcpy(lockMac, mac, 6);
  locked = true;
  lockIdx = findDev(mac);
  lockLastHeard = millis();
  lockAvgValid = false;
  prefs.putBytes("lock", lockMac, 6);
  if (lockIdx >= 0 && devs[lockIdx].ch) setChannel(devs[lockIdx].ch);
  char m[18];
  macStr(lockMac, m);
  Serial.printf("LOCKED %s", m);
  if (lockIdx >= 0 && devs[lockIdx].cat) Serial.printf("  (%s %s)", catName(devs[lockIdx].cat), OUI_LABELS[devs[lockIdx].label]);
  if (lockIdx >= 0 && devs[lockIdx].ssid[0]) Serial.printf("  \"%s\"", devs[lockIdx].ssid);
  Serial.println("\nStaying on its channel. Sweep the antenna slowly; the number peaks when you aim at it.");
  sendLock();
}

static void unlock() {
  locked = false;
  lockIdx = -1;
  prefs.remove("lock");
  Serial1.print("U\n");
  Serial.println("UNLOCKED. Scanning all channels.");
}

// ---------- lists ----------
static int buildList(bool suspectsOnly) {
  uint32_t now = millis();
  lastListCount = 0;
  for (int i = 0; i < MAX_DEVS && lastListCount < 64; i++) {
    Dev &d = devs[i];
    if (!d.used || now - d.lastSeen > LIST_AGE_MS) continue;
    if (suspectsOnly && !d.cat && !(d.flags & F_HIDDEN)) continue;
    lastList[lastListCount++] = i;
  }
  // Sort: suspects first (CAM, then IOT), then strongest average signal.
  for (int a = 1; a < lastListCount; a++) {
    int v = lastList[a], b = a - 1;
    auto rank = [](const Dev &d) { return d.cat == 1 ? 0 : (d.cat == 2 ? 1 : 2); };
    while (b >= 0) {
      Dev &x = devs[lastList[b]], &y = devs[v];
      bool before = suspectsOnly ? (rank(y) < rank(x) || (rank(y) == rank(x) && y.avg > x.avg)) : (y.avg > x.avg);
      if (!before) break;
      lastList[b + 1] = lastList[b];
      b--;
    }
    lastList[b + 1] = v;
  }
  return lastListCount;
}

static void printList(bool suspectsOnly) {
  int n = buildList(suspectsOnly);
  uint32_t now = millis();
  Serial.println(suspectsOnly ? "\nSuspects (camera makers, IoT chips, hidden networks) heard in the last 60 s:"
                              : "\nAll devices heard in the last 60 s, strongest first:");
  Serial.println(" #  MAC                type    maker            ch  avg dBm  pkts  seen  name / notes");
  Serial.println("--  -----------------  ------  --------------  ---  -------  ----  ----  -------------");
  for (int k = 0; k < n; k++) {
    Dev &d = devs[lastList[k]];
    char m[18];
    macStr(d.mac, m);
    const char *type = (d.flags & F_AP) ? "AP" : ((d.flags & F_CLIENT) ? "client" : "device");
    char maker[24] = "";
    if (d.cat) snprintf(maker, sizeof(maker), "%s %s", catName(d.cat), OUI_LABELS[d.label]);
    else if (d.flags & F_RANDOM) snprintf(maker, sizeof(maker), "(random MAC)");
    char notes[48] = "";
    if (d.flags & F_HIDDEN) snprintf(notes, sizeof(notes), "HIDDEN network");
    else if (d.ssid[0]) snprintf(notes, sizeof(notes), "\"%s\"", d.ssid);
    Serial.printf("%2d  %s  %-6s  %-14.14s  %3u  %7d  %4lu  %3lus  %s\n", k + 1, m, type, maker, d.ch, (int)d.avg,
                  (unsigned long)d.pkts, (unsigned long)((now - d.lastSeen) / 1000), notes);
  }
  if (n == 0) Serial.println("(nothing yet - give it 10-20 seconds)");
  Serial.println("Type a number to lock onto it.");
}

static void printHelp() {
  Serial.println("\nCommands: l = list all   c = suspects only   <number> = lock   m <MAC> = lock MAC   u = unlock   h = help");
  Serial.println("BOOT button: short press = next suspect, hold 1 s = unlock");
}

static void handleUsb() {
  static char buf[40];
  static uint8_t len = 0;
  while (Serial.available()) {
    char c = Serial.read();
    // Enter arrives as \r (arduino-cli monitor), \n or \r\n; any of them ends the line.
    if (c != '\r' && c != '\n') {
      if (len < sizeof(buf) - 1) buf[len++] = c;
      continue;
    }
    buf[len] = 0;
    len = 0;
    if (!buf[0]) continue;  // the \n of a \r\n pair
    char c0 = tolower(buf[0]);
    if (c0 == 'h' || c0 == '?') printHelp();
    else if (c0 == 'l') printList(false);
    else if (c0 == 'c') printList(true);
    else if (c0 == 'u') unlock();
    else if (c0 == 'm') {
      uint8_t mac[6];
      if (parseMac(buf + 1 + strspn(buf + 1, " "), mac)) lockOn(mac);
      else Serial.println("Type it like: m 24:0A:C4:12:34:56");
    } else {
      int n = atoi(buf);
      if (n >= 1 && n <= lastListCount) lockOn(devs[lastList[n - 1]].mac);
      else Serial.println("Unknown command. Type h for help.");
    }
  }
}

// BOOT button: short press = next suspect, hold = unlock
static void handleButton() {
  static bool wasDown = false;
  static uint32_t downAt = 0;
  static bool longDone = false;
  bool down = digitalRead(BUTTON_PIN) == LOW;
  uint32_t now = millis();
  if (down && !wasDown) { downAt = now; longDone = false; }
  if (down && !longDone && now - downAt > 1000) {
    longDone = true;
    unlock();
  }
  if (!down && wasDown && !longDone && now - downAt > 30) {
    int n = buildList(true);
    if (n == 0) n = buildList(false);
    if (n > 0) {
      cycleCursor = (cycleCursor + 1) % n;
      Serial.printf("Button: suspect %d of %d\n", cycleCursor + 1, n);
      lockOn(devs[lastList[cycleCursor]].mac);
    } else {
      Serial.println("Button: nothing heard yet.");
    }
  }
  wasDown = down;
}

// ---------- status LED ----------
// Blink patterns only (never steady), so they read the same whether the LED is active-low or -high.
static void updateLed(uint32_t now, bool isLocked, bool heard) {
#ifdef LED_BUILTIN
  uint32_t period = !isLocked ? 2000 : (heard ? 200 : 500);
  uint32_t on = !isLocked ? 60 : period / 2;
  digitalWrite(LED_BUILTIN, (now % period) < on ? LOW : HIGH);
#endif
}

// ---------- main ----------
void setup() {
  Serial.begin(115200);
  Serial1.begin(LINK_BAUD, SERIAL_8N1, LINK_RX, LINK_TX);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 1500) {}

  pktQueue = xQueueCreate(128, sizeof(Pkt));
  prefs.begin("hdh", false);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_country_code("US", false);  // US channel plan (1-11, and 5 GHz on the C5)
#if SOC_WIFI_SUPPORT_5G
  if (esp_wifi_set_band_mode(WIFI_BAND_MODE_AUTO) != ESP_OK) Serial.println("WARNING: could not turn on 5 GHz, 2.4 GHz only.");
#endif
  wifi_promiscuous_filter_t filt = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA};
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniff);
  esp_wifi_set_promiscuous(true);
  setChannel(CHANNELS[0]);

  Serial.printf("\n=== Hidden Device Hunter - Phase 2 sniffer (%s) ===\n", BOARD_NAME);
#if SOC_WIFI_SUPPORT_5G
  Serial.printf("Listening on %u channels, 2.4 + 5 GHz (one sweep about %lu s).\n", NUM_CH,
                (unsigned long)((NUM_CH * HOP_MS + 500) / 1000));
#endif
  Serial.printf("Maker list: %u entries. Receive-only: this board never transmits.\n", OUI_COUNT);
  printHelp();

  uint8_t saved[6];
  if (DEFAULT_LOCK[0] && prefs.getUChar("deflock", 0) != DEFAULT_LOCK_VER && parseMac(DEFAULT_LOCK, saved)) {
    prefs.putUChar("deflock", DEFAULT_LOCK_VER);  // once per DEFAULT_LOCK_VER; B and u still change it later
    Serial.print("Default lock from this build. ");
    lockOn(saved);
  } else if (prefs.getBytes("lock", saved, 6) == 6) {
    Serial.print("Saved lock found. ");
    lockOn(saved);
  } else {
    Serial1.print("U\n");
  }
}

void loop() {
  uint32_t now = millis();
  Pkt k;
  static uint32_t lastT = 0;
  bool targetHeard = false;
  int8_t targetRssi = 0;

  while (xQueueReceive(pktQueue, &k, 0) == pdTRUE) {
    int i = findDev(k.mac);
    if (i < 0) i = addDev(k.mac);
    Dev &d = devs[i];
    d.last = k.rssi;
    d.avg = (d.pkts == 0) ? k.rssi : (0.8f * d.avg + 0.2f * k.rssi);
    d.ch = k.ch;
    d.pkts++;
    d.lastSeen = now;
    if (k.kind == K_BEACON || k.kind == K_PROBE_RESP) {
      d.flags |= F_AP;
      if (k.kind == K_BEACON) {
        if (k.ssid[0] == 0) d.flags |= F_HIDDEN;
        else { strncpy(d.ssid, k.ssid, 32); d.flags &= ~F_HIDDEN; }
      }
    } else if (k.kind == K_PROBE_REQ) {
      d.flags |= F_CLIENT;
    }
    if (locked && memcmp(k.mac, lockMac, 6) == 0) {
      lockIdx = i;
      targetHeard = true;
      targetRssi = k.rssi;
      lockLastHeard = now;
      lockAvg = lockAvgValid ? (0.7f * lockAvg + 0.3f * k.rssi) : k.rssi;
      lockAvgValid = true;
    }
  }

  // Send the target's signal to the R4 up to 10 times a second.
  if (locked && targetHeard && now - lastT >= 100) {
    lastT = now;
    Serial1.printf("T,%d,%d,%u\n", targetRssi, (int)lockAvg, currentCh);
    static uint32_t lastUsb = 0;
    if (now - lastUsb >= 500) {
      lastUsb = now;
      Serial.printf("TARGET now %4d dBm  avg %4d dBm  ch %u\n", targetRssi, (int)lockAvg, currentCh);
    }
  }

  // Channel control: hop while scanning; stay put on a locked target unless it goes quiet.
  bool searching = !locked || (now - lockLastHeard > LOST_MS);
  if (searching) {
    if (now - lastHop >= HOP_MS) {
      lastHop = now;
      hopNext();
    }
  } else if (lockIdx >= 0 && devs[lockIdx].ch && devs[lockIdx].ch != currentCh) {
    setChannel(devs[lockIdx].ch);
  }

  // Once a second: status line to the R4 and a "lost" note.
  static uint32_t lastS = 0;
  if (now - lastS >= 1000) {
    lastS = now;
    int total = 0, cams = 0, iot = 0, hidden = 0;
    for (int i = 0; i < MAX_DEVS; i++) {
      if (!devs[i].used || now - devs[i].lastSeen > LIST_AGE_MS) continue;
      total++;
      if (devs[i].cat == 1) cams++;
      if (devs[i].cat == 2) iot++;
      if (devs[i].flags & F_HIDDEN) hidden++;
    }
    Serial1.printf("S,%d,%d,%d,%d,%d\n", total, cams, iot, hidden, locked ? 1 : 0);
    if (locked && now - lockLastHeard > LOST_MS) Serial.println("TARGET not heard - searching all channels...");
  }

  handleUsb();
  handleButton();
  updateLed(now, locked, !searching);
  delay(2);
}
