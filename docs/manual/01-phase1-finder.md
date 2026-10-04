# 01. Phase 1: Wi-Fi finder with buzzer (Uno R4 WiFi)

Goal: R4 scans Wi-Fi, you pick a target network, and the buzzer beeps faster (and the LED matrix shows more bars) as the signal gets stronger. Handheld on a USB power bank.
Needs only a buzzer and a resistor. Everything on the R4's own 5 V side; no level shifter.

## Parts
| Part | Spec | Price |
|---|---|---|
| Uno R4 WiFi | have | 0 |
| Passive piezo buzzer | 2-pin passive piezo, or 3-pin passive module (Keyestudio) | 1-4 |
| Resistor | 100 ohm 1/4 W | <1 |
| USB power bank | 5 V, with no low-current auto-off | 10-20 |
| USB-C cable | data-capable | have |
"Passive" matters: a passive buzzer needs a tone signal (`tone()`); an active one just buzzes at one pitch when given 5 V and will not beep faster in tone.

## Pin-to-pin wiring
| # | From | To | Note |
|---|---|---|---|
| 1 | R4 D8 | resistor end 1 | tone signal |
| 2 | resistor end 2 | buzzer + (longer leg / marked +) | |
| 3 | buzzer - | R4 GND (any GND pin) | |
| 4 | power bank USB | R4 USB-C | mobile power |
3-pin module variant (no resistor): module S -> D8, module + -> R4 5V, module - -> R4 GND.
Current: R4 pins are rated 8 mA each (60 mA total). A passive piezo draws far less, so the 100 ohm resistor is safe. Do not drive a magnetic buzzer or speaker straight from a pin.

## Wiring diagram
```
        Uno R4 WiFi (top view, USB-C at left)
   +---------------------------------------------+
   |                                [D13 ... D8 ... D0]   digital header
   |  USB-C <== power bank          |       |
   +--------------------------------|-------|-----+
                                    |       |
                          GND header|       |D8
                                    |       +----[ 100 ohm ]----( + )
                                    |                             BUZZER
                                    +-----------------------------( - )
```
Same as a schematic:
```
 D8 ----/\/\/\---- (+) BUZZER (-) ---- GND
        100 ohm
```

## Assembly
1. Unplug the R4 from everything.
2. Push the resistor and buzzer legs into a small breadboard (or twist and solder). Put the resistor in series on the + leg.
3. Run jumper D8 -> resistor, buzzer - -> GND. Double-check D8 goes through the resistor and not straight to the buzzer.
4. Plug the USB-C cable into the HaL/HaLjr for programming first; move to the power bank later.

## Software
Install the toolchain from `00-overview-and-safety.md`. Select board **Arduino UNO R4 WiFi**.

### Test 1: buzzer (3 beeps)
```cpp
void setup() {
  for (int i = 0; i < 3; i++) { tone(8, 2000, 150); delay(300); }
}
void loop() {}
```
Expect three short beeps after upload. If silent: swap the buzzer legs (piezos mostly don't care), then try D9 in place of D8
(D8 is not a marked PWM pin; `tone()` on the R4 may or may not work there, unverified). Use whichever pin works and update the wiring table.

### Test 2: Wi-Fi scan with signal strength (serial monitor, 115200)
```cpp
#include <WiFiS3.h>
void setup() { Serial.begin(115200); while (!Serial) {} }
void loop() {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    byte b[6]; WiFi.BSSID(i, b);
    char mac[18]; snprintf(mac, sizeof mac, "%02X:%02X:%02X:%02X:%02X:%02X", b[5], b[4], b[3], b[2], b[1], b[0]);
    Serial.print(WiFi.SSID(i)); Serial.print("  "); Serial.print(mac);
    Serial.print("  RSSI "); Serial.print(WiFi.RSSI(i)); Serial.print(" dBm  ch "); Serial.println(WiFi.channel(i));
  }
  Serial.println("---"); delay(2000);
}
```
If the MAC bytes print reversed, flip the order in the `snprintf`. Each scan takes a few seconds.

### Finder firmware (not written yet)
Behavior to build, in order:
1. Scan, list networks on Serial with an index.
2. Serial command (or button on a spare pin, pulled to GND) picks the target by MAC.
3. Loop: scan, read the target's RSSI, map -90 dBm (slow beep, 1 bar) to -30 dBm (continuous beep, all bars).
4. Draw bars on the 12x8 LED matrix (`Arduino_LED_Matrix`).
Scan-based RSSI updates every few seconds, so walk slowly. Phase 2 removes that limit.

## Field test
1. Phone hotspot on, in another room, 2.4 GHz band.
2. Select it as target. Walk toward it; beeps should speed up near it.
3. Walk past and away; beeps slow down.

## Troubleshooting (Phase 1)
| Symptom | Check first | Then |
|---|---|---|
| No beep | polarity, resistor seated, pin D8 | run Test 1; try D9; try an active buzzer on a 3-pin module |
| Board not in port list | data-capable cable, other USB port | `arduino-cli board list`, dialout group |
| Scan shows nothing | R4 WiFi firmware on the ESP32-S3 module | update via Arduino IDE firmware updater |
| Power bank shuts off | low-current auto-off | bank with always-on mode, or add a small load |
| Beep rate jumps around | walls and bodies reflect 2.4 GHz | average the last few readings; hold the R4 away from your body |
