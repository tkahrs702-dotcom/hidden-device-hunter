// Diagnostic: is the Modulino Buzzer reachable on the R4's Qwiic port (Wire1)?
// Repeats every ~4 s so the Serial Monitor (115200) always catches it.
#include <Modulino.h>

ModulinoBuzzer buzzer;
bool found = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Modulino.begin();  // Wire1 = Qwiic on the UNO R4 WiFi
}

void loop() {
  Serial.print(F("Qwiic (Wire1) devices:"));
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire1.beginTransmission(a);
    if (Wire1.endTransmission() == 0) { Serial.print(F(" 0x")); Serial.print(a, HEX); n++; }
  }
  if (!n) Serial.print(F(" NONE"));
  Serial.println();

  if (!found) found = buzzer.begin();
  Serial.print(F("buzzer.begin(): "));
  Serial.println(found ? F("FOUND") : F("NOT FOUND"));
  if (found) {
    Serial.println(F("Playing 2500 Hz for 300 ms"));
    buzzer.tone(2500, 300);
    delay(700);
    Serial.println(F("Playing 4000 Hz for 300 ms"));
    buzzer.tone(4000, 300);
    delay(700);
  }
  Serial.println(F("---"));
  delay(2000);
}
