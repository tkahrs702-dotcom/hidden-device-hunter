// Hidden Device Hunter - Phase 2: R4 beeper + LED meter fed by the ESP32 sniffer
// Board: Arduino Uno R4 WiFi
// Wiring: R4 D0 (RX) <- level shifter <- ESP32 UART TX
//         Buzzer: Modulino Buzzer on the Qwiic socket, same as Phase 1.
//
// The ESP32 sniffer does all the listening. This sketch only turns its numbers into sound and bars:
//   locked target heard   -> beeps speed up with signal, bars rise, steady tone when right on it
//   target is a CAM maker -> camera picture flashes for 1.5 s when the lock starts
//   top-row dot           -> closest approach since the lock (best bar reached)
//   target gone quiet     -> X on the matrix, one quiet click every 2 s
//   nothing locked        -> moving dot, silent; a steady top-right dot means the sniffer is connected
// Everything received is also copied to the USB Serial Monitor (115200) for logging.
//
// Messages from the ESP32 sniffer (one per line):
//   L,<MAC>,<cat>,<maker>   locked (cat 0 = unknown, 1 = camera maker, 2 = IoT chip)
//   U                       unlocked
//   T,<now>,<avg>,<ch>      target signal in dBm, about 10 per second
//   S,<total>,<cams>,<iot>,<hidden>,<locked>   once a second

#include "beeper.h"
#include "meter.h"

// ---------- settings you can change ----------
// Buzzer: Modulino Buzzer on the Qwiic socket (see beeper.h). No pin to set.
// The panel antenna adds gain, so readings run higher than Phase 1's. Adjust if the bars
// sit at full too early (raise RSSI_NEAR) or never move (lower RSSI_FAR).
const int RSSI_FAR = -90;
const int RSSI_NEAR = -30;
const uint32_t LOST_AFTER_MS = 3000;
// ---------------------------------------------

bool locked = false;
uint32_t lastT = 0;
uint32_t camUntil = 0;
uint8_t waitStep = 0;
uint32_t lastAnim = 0;
uint32_t lastS = 0;  // last status line from the sniffer; proves the wire works
char lockMac[18] = "";
char lockMaker[24] = "";
int peakBars = 0;  // best bar count since the lock = closest approach

int strengthPct(float rssi) {
  float p = (rssi - RSSI_FAR) * 100.0f / (RSSI_NEAR - RSSI_FAR);
  if (p < 0) p = 0;
  if (p > 100) p = 100;
  return (int)(p + 0.5f);
}

void onLine(char *line) {
  Serial.println(line);  // log everything
  char *f[6] = {0};
  int n = 0;
  for (char *tok = strtok(line, ","); tok && n < 6; tok = strtok(nullptr, ",")) f[n++] = tok;
  if (n == 0) return;

  if (f[0][0] == 'L' && n >= 3) {
    locked = true;
    lastT = millis();
    peakBars = 0;
    strncpy(lockMac, f[1], sizeof(lockMac) - 1);
    strncpy(lockMaker, n >= 4 ? f[3] : "", sizeof(lockMaker) - 1);
    if (atoi(f[2]) == 1) {
      camUntil = millis() + 1500;
      Meter::camera();
      Beeper::set(120, 60);  // quick triple chirp: "camera maker"
    } else {
      Meter::bars(0);
      Beeper::set(0, 0);
    }
  } else if (f[0][0] == 'U') {
    locked = false;
    Beeper::set(0, 0);
  } else if (f[0][0] == 'T' && n >= 3) {
    lastT = millis();
    locked = true;
    if (millis() < camUntil) return;  // let the camera picture show first
    int avg = atoi(f[2]);
    int pct = strengthPct(avg);
    if (pct >= 97) Beeper::continuous();
    else Beeper::set(1200 - (uint32_t)pct * 11, 40);
    int lit = (pct * 12 + 50) / 100;
    if (lit > peakBars) peakBars = lit;
    Meter::bars(lit, peakBars);
  } else if (f[0][0] == 'S' && n >= 6) {
    // The once-a-second status also carries the lock state, so a lock (or unlock) whose
    // L/U line was missed, e.g. because the sniffer booted first, still shows up here.
    lastS = millis();
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
  Serial1.begin(115200);  // D0/D1 to the ESP32 sniffer through the level shifter
  Meter::begin();
  if (!Beeper::begin()) Serial.println(F("WARNING: Modulino Buzzer not found. Check the Qwiic cable."));
  Serial.println(F("\n=== Hidden Device Hunter - Phase 2 display (Uno R4 WiFi) ==="));
  Serial.println(F("Waiting for the ESP32 sniffer..."));
  Beeper::runLoopAsTask();  // loop() runs as a task from here on
}

void loop() {
  static char buf[96];
  static uint8_t len = 0;
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\r') continue;
    if (c == '\n') {
      buf[len] = 0;
      if (len) onLine(buf);
      len = 0;
    } else if (len < sizeof(buf) - 1) {
      buf[len++] = c;
    }
  }

  uint32_t now = millis();
  static bool showingLost = false;
  bool isLost = locked && now - lastT > LOST_AFTER_MS;
  if (isLost && !showingLost) {
    Meter::lost();
    Beeper::set(2000, 15);
  }
  showingLost = isLost;
  if (isLost) {
    // already showing the X
  } else if (!locked && now - lastAnim > 150) {
    lastAnim = now;
    Meter::waiting(waitStep++, lastS && now - lastS < 2500);
  } else if (locked && camUntil && now >= camUntil) {
    camUntil = 0;
    Beeper::set(0, 0);
    Meter::bars(0);
  }
}
