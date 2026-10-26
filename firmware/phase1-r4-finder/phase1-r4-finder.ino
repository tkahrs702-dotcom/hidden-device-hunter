// Hidden Device Hunter - Phase 1: Wi-Fi finder with buzzer
// Board: Arduino Uno R4 WiFi        Buzzer: Modulino Buzzer on the Qwiic socket
//
// What it does
//   1. Scans the Wi-Fi networks around you every few seconds.
//   2. With no network picked it follows the strongest one (AUTO). To follow one network,
//      type its number in the Serial Monitor and press Enter.
//   3. The buzzer beeps faster and the LED matrix bars rise as you walk toward it.
//   The pick is saved, so it keeps working on a power bank away from the computer.
//
// Serial Monitor: 115200 baud, line ending "Newline".
//   <number>  lock onto that network from the last list
//   l         show the list again
//   c         clear the target
//   m         mute / unmute the buzzer
//   h         help
//
// Limits of the R4's Wi-Fi chip firmware (checked in the WiFiS3 library source):
//   - It reports at most 10 networks per scan (the strongest ones).
//   - It skips networks with a hidden (blank) name.
//   Phase 2 (XIAO ESP32-S3) has neither limit and also sees devices that are not
//   access points, such as cameras joined to your router.

#include <WiFiS3.h>
#include <EEPROM.h>
#include "beeper.h"
#include "meter.h"

// ---------- settings you can change ----------
// Buzzer: Modulino Buzzer on the Qwiic socket (see beeper.h). No pin to set.
const int RSSI_FAR = -90;       // dBm at or below this = 0 % strength
const int RSSI_NEAR = -35;      // dBm at or above this = 100 % strength
const int MISSES_BEFORE_LOST = 3;
// ---------------------------------------------

const uint32_t SAVE_MAGIC = 0x48444831;  // "HDH1"

struct Saved {
  uint32_t magic;
  uint8_t bssid[6];
  char ssid[33];
};

struct Net {
  char ssid[33];
  uint8_t bssid[6];
  int rssi;
  uint8_t ch;
  uint8_t enc;
};

Saved target;
bool hasTarget = false;
Net lastList[WIFI_MAX_SSID_COUNT];
int lastCount = 0;
bool printList = true;
float avgRssi = 0;
bool avgValid = false;
int misses = 0;
uint8_t waitStep = 0;
bool muted = false;
bool buzzerTestMode = false;
bool scanZeroReported = false;

// ---------- helpers ----------
void macToStr(const uint8_t *m, char *out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

const char *encName(uint8_t e) {
  switch (e) {
    case ENC_TYPE_NONE: return "OPEN";
    case ENC_TYPE_WEP: return "WEP";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "WPA/2";
    default: return "WPA3?";
  }
}

int strengthPct(float rssi) {
  float p = (rssi - RSSI_FAR) * 100.0f / (RSSI_NEAR - RSSI_FAR);
  if (p < 0) p = 0;
  if (p > 100) p = 100;
  return (int)(p + 0.5f);
}

// Stronger signal -> shorter gap between beeps. 0 % = one beep every 1.2 s, 100 % = steady tone.
uint32_t beepIntervalMs(int pct) {
  if (pct >= 97) return 0;  // 0 here means "steady tone"
  return 1200 - (uint32_t)pct * 11;
}

void saveTarget() {
  target.magic = SAVE_MAGIC;
  EEPROM.put(0, target);
}

void loadTarget() {
  EEPROM.get(0, target);
  hasTarget = (target.magic == SAVE_MAGIC);
  target.ssid[32] = 0;
}

void printHelp() {
  Serial.println(F("\nCommands: <number> = lock target   l = list   c = clear target   m = mute   t = toggle buzzer test   h = help"));
}

void printTable() {
  char mac[18];
  Serial.println(F("\n #  Network name                     MAC                ch  signal  security"));
  Serial.println(F("--  -------------------------------  -----------------  --  ------  --------"));
  for (int i = 0; i < lastCount; i++) {
    macToStr(lastList[i].bssid, mac);
    char line[110];
    snprintf(line, sizeof(line), "%2d  %-31.31s  %s  %2u  %4d dBm  %s%s", i + 1, lastList[i].ssid, mac,
             lastList[i].ch, lastList[i].rssi, encName(lastList[i].enc),
             lastList[i].enc == ENC_TYPE_NONE ? "  <- open network" : "");
    Serial.println(line);
  }
  if (lastCount == WIFI_MAX_SSID_COUNT)
    Serial.println(F("(The R4 lists only the 10 strongest networks. Move closer if yours is missing.)"));
  if (!hasTarget) Serial.println(F("AUTO mode: beeping for the strongest network. Type a number and press Enter to lock onto one."));
}

void handleSerial() {
  static char buf[24];
  static uint8_t len = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c != '\n') {
      if (len < sizeof(buf) - 1) buf[len++] = c;
      continue;
    }
    buf[len] = 0;
    len = 0;
    if (buf[0] == 0) continue;
    if (buf[0] == 'h' || buf[0] == 'H' || buf[0] == '?') {
      printHelp();
    } else if (buf[0] == 'l' || buf[0] == 'L') {
      printTable();
    } else if (buf[0] == 'c' || buf[0] == 'C') {
      hasTarget = false;
      target.magic = 0;
      EEPROM.put(0, target);
      if (!buzzerTestMode) Beeper::set(0, 0);
      Meter::question();
      Serial.println(F("Target cleared."));
      printList = true;
    } else if (buf[0] == 't' || buf[0] == 'T') {
      buzzerTestMode = !buzzerTestMode;
      if (buzzerTestMode) {
        Serial.println(Beeper::found ? F("Buzzer test on; type t again to stop.")
                                     : F("Buzzer not found yet; checking the Qwiic port. Type t again to stop."));
        Beeper::set(500, 100);
      } else {
        Serial.println(F("Buzzer test off."));
        Beeper::set(0, 0);
      }
    } else if (buf[0] == 'm' || buf[0] == 'M') {
      muted = !muted;
      Beeper::mute(muted);
      Serial.println(muted ? F("Buzzer muted.") : F("Buzzer on."));
    } else {
      int n = atoi(buf);
      if (n >= 1 && n <= lastCount) {
        memcpy(target.bssid, lastList[n - 1].bssid, 6);
        strncpy(target.ssid, lastList[n - 1].ssid, 32);
        target.ssid[32] = 0;
        hasTarget = true;
        avgValid = false;
        misses = 0;
        saveTarget();
        char mac[18];
        macToStr(target.bssid, mac);
        Serial.print(F("Locked onto \""));
        Serial.print(target.ssid);
        Serial.print(F("\" "));
        Serial.println(mac);
        Serial.println(F("Saved. It stays locked after unplugging. Type c to clear."));
      } else {
        Serial.println(F("Unknown command. Type h for help."));
      }
    }
  }
}

// ---------- main ----------
void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {}

  Meter::begin();
  Meter::question();
  if (!Beeper::begin()) Serial.println(F("WARNING: Modulino Buzzer not found. Check the Qwiic cable."));

  Serial.println(F("\n=== Hidden Device Hunter - Phase 1 (Uno R4 WiFi) ==="));
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("ERROR: Wi-Fi module not responding. Unplug and replug the board."));
    Meter::lost();
    while (true) delay(1000);
  }
  String fv = WiFi.firmwareVersion();
  Serial.print(F("Wi-Fi module firmware: "));
  Serial.println(fv);
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println(F("  (Older than this library expects. Update it with the Arduino IDE's Firmware Updater.)"));

  // The beeper plays a 3-beep startup chirp once the tasks start.

  loadTarget();
  if (hasTarget) {
    char mac[18];
    macToStr(target.bssid, mac);
    Serial.print(F("Saved target: \""));
    Serial.print(target.ssid);
    Serial.print(F("\" "));
    Serial.println(mac);
  }
  printHelp();
  Serial.println(F("Scanning..."));
  Beeper::runLoopAsTask();  // loop() runs as a task from here on
}

void loop() {
  handleSerial();

  int n = WiFi.scanNetworks();  // blocks a few seconds; the beeper task keeps the beeps going
  handleSerial();
  if (n < 0) {
    Serial.print(F("ERROR: Wi-Fi scan returned "));
    Serial.print(n);
    Serial.println(F("."));
    n = 0;
  } else if (n == 0 && !scanZeroReported) {
    Serial.println(F("Wi-Fi scan returned 0 networks. WiFiS3 also reports some scan-command failures as 0, so this does not prove the air is empty."));
    scanZeroReported = true;
  } else if (n > 0) {
    scanZeroReported = false;
  }

  lastCount = 0;
  int found = -1;
  for (int i = 0; i < n && lastCount < WIFI_MAX_SSID_COUNT; i++) {
    Net &e = lastList[lastCount];
    strncpy(e.ssid, WiFi.SSID(i), 32);
    e.ssid[32] = 0;
    WiFi.BSSID(i, e.bssid);
    e.rssi = WiFi.RSSI(i);
    e.ch = WiFi.channel(i);
    e.enc = WiFi.encryptionType(i);
    if (hasTarget && memcmp(e.bssid, target.bssid, 6) == 0) found = lastCount;
    lastCount++;
  }

  // No locked target: follow the strongest network heard (hot/cold for the nearest
  // router or hotspot), so the finder works on a power bank with no computer.
  static uint8_t autoBssid[6];
  if (!hasTarget) {
    if (printList) {
      printTable();
      printList = false;
    }
    if (lastCount == 0) {
      if (!buzzerTestMode) Beeper::set(0, 0);
      avgValid = false;
      Meter::waiting(waitStep++);
      return;
    }
    found = 0;
    for (int i = 1; i < lastCount; i++)
      if (lastList[i].rssi > lastList[found].rssi) found = i;
    if (memcmp(autoBssid, lastList[found].bssid, 6) != 0) {  // new strongest: restart the average
      memcpy(autoBssid, lastList[found].bssid, 6);
      avgValid = false;
    }
  }
  const char *name = hasTarget ? target.ssid : lastList[found >= 0 ? found : 0].ssid;

  if (found >= 0) {
    misses = 0;
    int r = lastList[found].rssi;
    // Smooth out the jumps: 60 % old value, 40 % new reading.
    avgRssi = avgValid ? (0.6f * avgRssi + 0.4f * r) : r;
    avgValid = true;
    int pct = strengthPct(avgRssi);
    uint32_t gap = beepIntervalMs(pct);
    if (!buzzerTestMode) {
      if (gap == 0) Beeper::continuous();
      else Beeper::set(gap, 40);
    }
    Meter::bars((pct * 12 + 50) / 100);

    char line[120];
    snprintf(line, sizeof(line), "%s %-20.20s ch%-2u now %4d dBm  avg %4d dBm  strength %3d%%  %s",
             hasTarget ? "TARGET" : "AUTO  ", name, lastList[found].ch, r, (int)avgRssi, pct,
             gap == 0 ? "STEADY TONE" : "");
    Serial.print(line);
    if (gap) {
      Serial.print(F("beep every "));
      Serial.print(gap);
      Serial.print(F(" ms"));
    }
    Serial.println();
  } else {
    misses++;
    if (misses == 1) {
      avgValid = false;
      if (!buzzerTestMode) {
        Beeper::set(0, 0);
        Meter::waiting(waitStep++);
      }
    }
    if (misses >= MISSES_BEFORE_LOST) {
      if (!buzzerTestMode) Beeper::set(2000, 15);  // one quiet click every 2 s = "not heard"
      Meter::lost();
      avgValid = false;
      Serial.print(F("TARGET not heard for "));
      Serial.print(misses);
      Serial.println(F(" scans (out of range, switched off, or not in the 10 strongest)."));
    }
  }
}
