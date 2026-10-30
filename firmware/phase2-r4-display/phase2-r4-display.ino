// Hidden Device Hunter - Phase 2: R4 beeper + LED meter fed by the ESP32 sniffer
// Board: Arduino Uno R4 WiFi
// Wiring: R4 D0 (RX) <- level shifter <- ESP32 UART TX
//         Buzzer: Modulino Buzzer on the Qwiic socket, same as Phase 1.
//
// The ESP32 sniffer does all the listening. This sketch only turns its numbers into sound and bars:
//   locked target heard   -> beeps speed up with signal, bars rise, steady tone when right on it
//   target is a CAM maker -> camera picture flashes for 1.5 s when the lock starts
//   phone page            -> join Wi-Fi HDH-Finder, open http://192.168.4.1 (see webview.h)
//   top-row dot           -> closest approach since the lock (best bar reached)
//   target gone quiet     -> X on the matrix, one quiet click every 2 s
//   nothing locked        -> moving dot, silent; a steady top-right dot means the sniffer is connected
// Everything received is also copied to the USB Serial Monitor (115200) for logging.
//
// Messages from the ESP32 sniffer (one per line):
//   L,<MAC>,<cat>,<maker>   locked (cat 0 = unknown, 1 = camera maker, 2 = IoT chip)
//   U                       unlocked
//   T,<now>,<avg>,<ch>,<pps>,<idle pps>,<wobble dB>,<wave 0/1>
//                           target signal in dBm, about 10 per second, plus telemetry for the phone page
//   S,<total>,<cams>,<iot>,<hidden>,<locked>   once a second

#include "beeper.h"
#include "meter.h"
#include "webview.h"

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
  char *f[8] = {0};
  int n = 0;
  for (char *tok = strtok(line, ","); tok && n < 8; tok = strtok(nullptr, ",")) f[n++] = tok;
  if (n == 0) return;

  if (f[0][0] == 'L' && n >= 3 && locked && strcmp(f[1], lockMac) == 0) {
    lastT = millis();  // the sniffer repeats L every 4 s: same target, nothing to redo
  } else if (f[0][0] == 'L' && n >= 3) {
    locked = true;
    lastT = millis();
    peakBars = 0;
    Web::locked(f[1], n >= 4 ? f[3] : "");
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
    Web::unlocked();
    Beeper::set(0, 0);
  } else if (f[0][0] == 'T' && n >= 3) {
    lastT = millis();
    locked = true;
    if (millis() < camUntil) return;  // let the camera picture show first
    int avg = atoi(f[2]);
    Web::signal(atoi(f[1]), avg, n >= 4 ? atoi(f[3]) : 0);
    if (n >= 8) Web::telemetry(atoi(f[4]), atoi(f[5]), atoi(f[6]), atoi(f[7]));
    int pct = strengthPct(avg);
    if (pct >= 97) Beeper::continuous();
    else Beeper::set(1200 - (uint32_t)pct * 11, 40);
    int lit = (pct * 12 + 50) / 100;
    if (lit > peakBars) peakBars = lit;
    Meter::bars(lit, peakBars);
  } else if (f[0][0] == 'V' && n >= 2) {
    Beeper::volume(atoi(f[1]));
  } else if (f[0][0] == 'M' && n >= 2) {
    Web::mode(atoi(f[1]));
  } else if (f[0][0] == 'B' && n >= 2) {
    Web::nearbyStart(atoi(f[1]));
  } else if (f[0][0] == 'D' && n >= 6) {
    Web::nearby(atoi(f[1]), f[2], atoi(f[3]), atoi(f[4]), atoi(f[5]), n >= 7 ? f[6] : "-", n >= 8 ? f[7] : "-");
  } else if (f[0][0] == 'S' && n >= 6) {
    // The once-a-second status also carries the lock state, so a lock (or unlock) whose
    // L/U line was missed, e.g. because the sniffer booted first, still shows up here.
    lastS = millis();
    Web::stats(atoi(f[1]), atoi(f[2]), atoi(f[3]), atoi(f[4]));
    bool snifferLocked = atoi(f[5]) == 1;
    if (snifferLocked && !locked) {
      locked = true;
      lastT = millis();
      Meter::bars(0);
    } else if (!snifferLocked && locked) {
      locked = false;
      Web::unlocked();
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

// Reads whatever the sniffer has sent. Also called while the phone page is being sent, because
// the Wi-Fi chip is slow and the R4's serial buffer would otherwise overflow and lose lines.
void pumpSerial() {
  static char buf[110];
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
}

void loop() {
  // The phone page starts 4 s after power-up, from the loop task, so the chirp, the dot and the
  // beeps never wait for the Wi-Fi module: if it is slow or stuck, only the page is affected.
  static bool webStarted = false;
  if (!webStarted && millis() > 4000) {
    webStarted = true;
    Web::setPump(pumpSerial);
    Web::begin();  // Wi-Fi "HDH-Finder", page at http://192.168.4.1
  }
  pumpSerial();
  Web::poll();

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
