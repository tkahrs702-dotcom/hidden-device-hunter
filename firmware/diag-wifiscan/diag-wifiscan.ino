// Diagnostic: how many networks does the R4's Wi-Fi module report on a plain scan?
// No tasks, no buzzer. Serial Monitor at 115200; repeats every ~5 s.
#include <WiFiS3.h>

void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}
  Serial.print(F("Wi-Fi module firmware: "));
  Serial.println(WiFi.firmwareVersion());
}

void loop() {
  Serial.print(F("fw ")); Serial.print(WiFi.firmwareVersion()); Serial.print(F("  "));
  int n = WiFi.scanNetworks();
  Serial.print(F("scan found "));
  Serial.println(n);
  for (int i = 0; i < n; i++) {
    uint8_t b[6];
    WiFi.BSSID(i, b);
    char line[90];
    snprintf(line, sizeof(line), "  %-32s %02X:%02X:%02X:%02X:%02X:%02X ch%-2d %4d dBm",
             WiFi.SSID(i), b[0], b[1], b[2], b[3], b[4], b[5], WiFi.channel(i), WiFi.RSSI(i));
    Serial.println(line);
  }
  delay(3000);
}
