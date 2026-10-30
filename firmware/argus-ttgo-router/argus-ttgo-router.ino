// Hidden Device Hunter - Argus: router guard.
// Board: TTGO LoRa32 (T3 V1.6.1), Arduino board "TTGO LoRa32-OLED", revision "TTGO LoRa32 V2.1 (1.6.1)".
// Sits by the router on its own USB power. No wires to any other board.
//
// What it does:
//  1. At boot it finds the router: the strongest network it hears (Argus sits next to it). The router's
//     BSSID is saved, so later boots look for that one. Hold BOOT for 3 s to learn a new router.
//  2. It listens on the router's channel for deauth and disassoc frames (the frames a "kick everyone off
//     Wi-Fi" attack sends). The router changes channel by itself (auto channel), so Argus re-checks the
//     channel every 10 minutes and whenever the router goes quiet.
//  3. When more than ATTACK_FRAMES such frames touch the router within WINDOW_MS, it sends an alert by
//     LoRa to Iolaus, which passes it to Merlin's screen. It also sends a heartbeat every 30 s.
// The router's name is never stored in this repo: Argus learns it on the spot.
//
// LoRa packets (text):  HDH1,H,<seq>,<ch>,<uptime min>
//                       HDH1,A,<seq>,<deauth>,<disassoc>,<ch>,<from MAC>,<to MAC>,<dBm>

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "esp_wifi.h"

// ---------- settings you can change ----------
// LoRa radio: must match Iolaus exactly.
static const long LORA_FREQ = 915E6;
static const int LORA_SF = 9;
static const long LORA_BW = 125E3;
static const int LORA_SYNC = 0x5A;  // this project's own sync word
static const uint32_t WINDOW_MS = 10000;   // count frames over this long
static const int ATTACK_FRAMES = 8;        // more than this many in one window = attack
static const uint32_t HEARTBEAT_MS = 30000;
static const uint32_t RECHECK_MS = 600000; // look for the router's channel again every 10 min
static const uint32_t QUIET_MS = 20000;    // no beacon from the router this long -> look again now
// TTGO T3 V1.6.1 pins
static const int LORA_SCK = 5, LORA_MISO = 19, LORA_MOSI = 27, LORA_SS = 18, LORA_RST = 23, LORA_DIO0 = 26;
static const int OLED_SDA = 21, OLED_SCL = 22, BUTTON = 0;
// ---------------------------------------------

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool haveOled;
Preferences prefs;

uint8_t router[6];
bool haveRouter;
int routerCh;
char routerSsid[33];
uint32_t seq;

// Counters written by the Wi-Fi callback (another task), read and reset by loop().
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile int deauthCount, disassocCount, lastRssi;
volatile uint32_t lastBeaconMs;
uint8_t lastFrom[6], lastTo[6];

static void macStr(const uint8_t *m, char *out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

static void show(const char *a, const char *b = "", const char *c = "", const char *d = "") {
  Serial.printf("%s | %s | %s | %s\n", a, b, c, d);
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("ARGUS router guard");
  oled.println(a);
  oled.println(b);
  oled.println(c);
  oled.println(d);
  oled.display();
}

// Called for every management frame heard on the channel.
static void IRAM_ATTR onFrame(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;
  const wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t *)buf;
  if (p->rx_ctrl.sig_len < 24) return;  // too short to hold the addresses
  const uint8_t *f = p->payload;
  uint8_t subtype = f[0] & 0xF0;
  const uint8_t *to = f + 4, *from = f + 10, *bssid = f + 16;
  bool touchesRouter = !memcmp(bssid, router, 6) || !memcmp(from, router, 6) || !memcmp(to, router, 6);
  if (!touchesRouter) return;
  portENTER_CRITICAL_ISR(&mux);
  if (subtype == 0x80 && !memcmp(from, router, 6)) {
    lastBeaconMs = millis();  // beacon from the router: it is still on this channel
  } else if (subtype == 0xC0 || subtype == 0xA0) {
    if (subtype == 0xC0) deauthCount++;
    else disassocCount++;
    memcpy(lastFrom, from, 6);
    memcpy(lastTo, to, 6);
    lastRssi = p->rx_ctrl.rssi;
  }
  portEXIT_CRITICAL_ISR(&mux);
}

// Scans all channels. learn = true picks the strongest network; otherwise finds the saved router.
static bool findRouter(bool learn) {
  esp_wifi_set_promiscuous(false);
  show("Looking for the router...");
  int n = WiFi.scanNetworks(false, true);
  int best = -1;
  for (int i = 0; i < n; i++) {
    if (learn) {
      if (best < 0 || WiFi.RSSI(i) > WiFi.RSSI(best)) best = i;
    } else if (!memcmp(WiFi.BSSID(i), router, 6)) {
      best = i;
    }
  }
  bool ok = best >= 0;
  if (ok) {
    memcpy(router, WiFi.BSSID(best), 6);
    routerCh = WiFi.channel(best);
    strlcpy(routerSsid, WiFi.SSID(best).c_str(), sizeof(routerSsid));
    if (learn) prefs.putBytes("router", router, 6);
    haveRouter = true;
  }
  WiFi.scanDelete();
  if (haveRouter) {
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(routerCh, WIFI_SECOND_CHAN_NONE);
  }
  lastBeaconMs = millis();
  return ok;
}

static void sendLora(const char *msg) {
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  Serial.printf("LoRa sent: %s\n", msg);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLUP);
  Wire.begin(OLED_SDA, OLED_SCL);
  haveOled = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    show("LoRa radio not found!", "Check the board type.");
    while (true) delay(1000);
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setSyncWord(LORA_SYNC);
  LoRa.enableCrc();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_promiscuous_filter_t filt = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(onFrame);

  prefs.begin("argus", false);
  bool saved = prefs.getBytes("router", router, 6) == 6;
  if (!(saved && findRouter(false))) findRouter(true);
}

void loop() {
  static uint32_t windowStart, lastBeat, lastCheck, buttonDown;
  uint32_t now = millis();
  char a[24], b[24], line[120];

  // Hold BOOT 3 s: learn the strongest network as the router.
  if (digitalRead(BUTTON) == LOW) {
    if (!buttonDown) buttonDown = now;
    if (now - buttonDown > 3000) {
      findRouter(true);
      buttonDown = 0;
      while (digitalRead(BUTTON) == LOW) delay(10);
    }
  } else {
    buttonDown = 0;
  }

  if (!haveRouter) {
    if (now - lastCheck > 15000) {
      lastCheck = now;
      findRouter(true);
    }
    show("No network heard.", "Retrying...");
    delay(500);
    return;
  }

  if (now - lastCheck > RECHECK_MS || now - lastBeaconMs > QUIET_MS) {
    lastCheck = now;
    findRouter(false);
  }

  if (now - windowStart >= WINDOW_MS) {
    portENTER_CRITICAL(&mux);
    int de = deauthCount, di = disassocCount, rssi = lastRssi;
    uint8_t from[6], to[6];
    memcpy(from, lastFrom, 6);
    memcpy(to, lastTo, 6);
    deauthCount = disassocCount = 0;
    portEXIT_CRITICAL(&mux);
    windowStart = now;
    if (de + di > ATTACK_FRAMES) {
      macStr(from, a);
      macStr(to, b);
      snprintf(line, sizeof(line), "HDH1,A,%lu,%d,%d,%d,%s,%s,%d", (unsigned long)++seq, de, di, routerCh, a, b, rssi);
      sendLora(line);
      char l2[24];
      snprintf(l2, sizeof(l2), "%d deauth %d disassoc", de, di);
      show("!! ATTACK !!", l2, a);
    } else {
      char l1[40], l2[24];
      snprintf(l1, sizeof(l1), "%.20s", routerSsid);
      snprintf(l2, sizeof(l2), "ch %d  quiet", routerCh);
      show("Watching:", l1, l2, de + di ? "a few frames, normal" : "");
    }
  }

  if (now - lastBeat > HEARTBEAT_MS || !lastBeat) {
    lastBeat = now;
    snprintf(line, sizeof(line), "HDH1,H,%lu,%d,%lu", (unsigned long)++seq, routerCh, (unsigned long)(now / 60000));
    sendLora(line);
  }
  delay(20);
}
