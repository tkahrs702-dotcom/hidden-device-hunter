// Hidden Device Hunter - Phase 2: R4 beeper + LED meter fed by the ESP32 sniffer
// Board: Arduino Uno R4 WiFi
// Wiring: R4 D0 (RX) <- level shifter <- ESP32 UART TX

#include "beeper.h"
#include "meter.h"
#include "webview.h"

const int RSSI_FAR = -90;
const int RSSI_NEAR = -30;
const uint32_t LOST_AFTER_MS = 3000; 

bool locked = false;
uint32_t lastT = 0;
uint32_t camUntil = 0;
uint8_t waitStep = 0;
uint32_t lastAnim = 0;
uint32_t lastS = 0; 

char lockMac[18] = "";
char lockMaker[24] = "";
int peakBars = 0; 

int strengthPct(float rssi) {
  float p = (rssi - RSSI_FAR) * 100.0f / (RSSI_NEAR - RSSI_FAR);
  if (p < 0) p = 0;
  if (p > 100) p = 100;
  return (int)(p + 0.5f);
}

void onLine(char *line) {
  Serial.println(line); 
  char *f[8] = {0};
  int n = 0;
  for (char *tok = strtok(line, ","); tok && n < 8; tok = strtok(nullptr, ",")) f[n++] = tok;
  if (n == 0) return;

  if (f[0][0] == "L" && n >= 3) {
    locked = true;
    lastT = millis();
    peakBars = 0;
    Web::locked(f[1], n >= 4 ? f[3] : "");
    strncpy(lockMac, f[1], sizeof(lockMac) - 1);
    strncpy(lockMaker, n >= 4 ? f[3] : "", sizeof(lockMaker) - 1);
    if (atoi(f[2]) == 1) {
      camUntil = millis() + 1500;
      Meter::camera();
      Beeper::set(120, 60); 
    } else {
      Beeper::set(800, 80);
    }
  } else if (f[0][0] == "U") {
    locked = false;
    Web::unlocked();
    Beeper::set(0, 0);
  } else if (f[0][0] == "T" && n >= 3) {
    lastT = millis();
    locked = true;
    if (millis() < camUntil) return; 
    int avg = atoi(f[2]);
    Web::signal(atoi(f[1]), avg, n >= 4 ? atoi(f[3]) : 0);
    int pct = strengthPct(avg);
    if (pct >= 97) Beeper::continuous();
    else Beeper::set(1200 - (uint32_t)pct * 11, 40);
    int lit = (pct * 12 + 50) / 100;
    if (lit > peakBars) peakBars = lit;
    Meter::bars(lit, peakBars);
  } else if (f[0][0] == "S" && n >= 6) {
    lastS = millis();
    Web::stats(atoi(f[1]), atoi(f[2]), atoi(f[3]), atoi(f[4]));
    bool snifferLocked = atoi(f[5]) == 1;
    if (snifferLocked && !locked) {
      locked = true;
      lastT = millis();
      Meter::bars(0);
    } else if (!snifferLocked && locked) {
      locked = false;
      Beeper::set(0, 0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200); 
  Meter::begin();
  Beeper::begin();
  Serial.println(F("\n=== Hidden Device Hunter - Phase 2 display (Uno R4 WiFi) ==="));
}

void loop() {
  static char buf[128];
  static int len = 0;
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == "\r") continue;
    if (c == "\n") {
      buf[len] = "\0";
      if (len > 0) onLine(buf);
      len = 0;
    } else if (len < (int)sizeof(buf) - 1) {
      buf[len++] = c;
    }
  }

  Web::poll();

  if (locked && (millis() - lastT > LOST_AFTER_MS)) {
    locked = false;
    Meter::lost();
    Beeper::set(200, 200);
    delay(200);
    Beeper::set(0, 0);
  }

  bool linkAlive = (millis() - lastS < 2500);
  Meter::setHeartbeat(linkAlive);
}
