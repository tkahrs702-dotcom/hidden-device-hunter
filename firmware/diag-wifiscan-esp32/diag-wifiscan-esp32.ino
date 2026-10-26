// Diagnostic: plain Wi-Fi scan on any ESP32 board (e.g. TTGO LoRa32), same output format
// as diag-wifiscan so tools/scan-compare.py can compare it with the computer's scan.
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void loop() {
  int n = WiFi.scanNetworks();
  Serial.print(F("scan found "));
  Serial.println(n);
  for (int i = 0; i < n; i++) {
    uint8_t *b = WiFi.BSSID(i);
    char line[90];
    snprintf(line, sizeof(line), "  %-32s %02X:%02X:%02X:%02X:%02X:%02X ch%-2d %4d dBm",
             WiFi.SSID(i).c_str(), b[0], b[1], b[2], b[3], b[4], b[5], WiFi.channel(i), WiFi.RSSI(i));
    Serial.println(line);
  }
  WiFi.scanDelete();
  delay(3000);
}
