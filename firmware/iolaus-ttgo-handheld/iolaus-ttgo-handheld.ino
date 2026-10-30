// Hidden Device Hunter - Iolaus: the handheld's LoRa ear.
// Board: TTGO LoRa32 (T3 V1.6.1), Arduino board "TTGO LoRa32-OLED", revision "TTGO LoRa32 V2.1 (1.6.1)".
// Lives in the handheld, wired to Merlin (all 3.3 V, powered from Merlin's 3V3 pin, its own USB and
// battery unplugged while wired):
//   Iolaus GPIO13 (TX) -> Merlin GPIO21     Merlin GPIO22 -> Iolaus GPIO34 (RX)     Iolaus GND -> Merlin GND
//   Iolaus 3V3 -> Merlin 3V3
// It hears Argus by LoRa and passes each packet to Merlin as one serial line (115200 baud):
//   I,<uptime s>                                      every 2 s, so Merlin knows Iolaus is alive
//   H,<seq>,<ch>,<uptime min>,<lora dBm>              Argus heartbeat
//   A,<seq>,<deauth>,<disassoc>,<ch>,<from>,<to>,<dBm>,<lora dBm>   attack on the router

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- settings you can change ----------
// LoRa radio: must match Argus exactly.
static const long LORA_FREQ = 915E6;
static const int LORA_SF = 9;
static const long LORA_BW = 125E3;
static const int LORA_SYNC = 0x5A;
// TTGO T3 V1.6.1 pins (PIN_ prefix: the board package already defines LORA_SCK etc. as macros)
static const int PIN_LORA_SCK = 5, PIN_LORA_MISO = 19, PIN_LORA_MOSI = 27, PIN_LORA_SS = 18, PIN_LORA_RST = 23, PIN_LORA_DIO0 = 26;
static const int PIN_OLED_SDA = 21, PIN_OLED_SCL = 22;
static const int MERLIN_TX = 13, MERLIN_RX = 34;  // 34 is input-only, fine for receiving
// ---------------------------------------------

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
bool haveOled;
uint32_t packets, lastPacketMs;
String lastLine = "waiting for Argus";

static void show() {
  if (!haveOled) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("IOLAUS LoRa ear");
  oled.printf("packets: %lu\n", (unsigned long)packets);
  if (lastPacketMs) oled.printf("last: %lus ago\n", (unsigned long)((millis() - lastPacketMs) / 1000));
  oled.println(lastLine.substring(0, 63));
  oled.display();
}

void setup() {
  Serial.begin(115200);  // USB, for checking on a computer
  Serial1.begin(115200, SERIAL_8N1, MERLIN_RX, MERLIN_TX);
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  haveOled = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_SS);
  LoRa.setPins(PIN_LORA_SS, PIN_LORA_RST, PIN_LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    lastLine = "LoRa radio not found!";
    Serial.println(lastLine);
    show();
    while (true) delay(1000);
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setSyncWord(LORA_SYNC);
  LoRa.enableCrc();
  show();
}

void loop() {
  static uint32_t lastAlive, lastShow;
  uint32_t now = millis();

  if (LoRa.parsePacket()) {
    String p;
    while (LoRa.available()) p += (char)LoRa.read();
    int dbm = LoRa.packetRssi();
    // Only this project's packets, and only the two kinds Argus sends.
    if ((p.startsWith("HDH1,H,") || p.startsWith("HDH1,A,")) && p.length() < 120 && p.indexOf('\n') < 0) {
      String out = p.substring(5) + "," + String(dbm);
      Serial1.println(out);
      Serial.println(out);
      packets++;
      lastPacketMs = now;
      lastLine = String(p[5] == 'A' ? "ATTACK! " : "Argus OK ") + String(dbm) + " dBm";
      show();
    }
  }

  if (now - lastAlive >= 2000) {
    lastAlive = now;
    Serial1.printf("I,%lu\n", (unsigned long)(now / 1000));
  }
  if (now - lastShow >= 1000) {
    lastShow = now;
    show();
  }
  // Merlin does not send anything yet; drain its line so nothing piles up.
  while (Serial1.available()) Serial1.read();
}
