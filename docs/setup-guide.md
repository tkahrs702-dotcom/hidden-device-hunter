# Sentinel RF Finder: Setup Guide
Copied 2026-10-03 from the Claude Docs original (https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1). Prices checked online on that date; they move.
Condensed: pin tables, steps, rules and troubleshooting kept; sources trimmed.

## Parts and prices (USD)
| Part | Phase | Pick | Price |
|---|---|---|---|
| Passive piezo buzzer | 1 | 2-pin passive piezo, or 3-pin passive module (Keyestudio) | 1-4 |
| 100 ohm resistor 1/4 W | 1 | | <1 |
| USB power bank | 1 | 5 V; one without low-current auto-off | 10-20 |
| XIAO ESP32-S3 | 2 | Seeed XIAO ESP32S3, u.FL jack, antenna in box | 7.49 |
| u.FL to RP-SMA female bulkhead pigtail | 2 | Superbat 15 cm 1.13 mm, or Proxicast 2-pack (RP-SMA) | 2.13-7.94 |
| 2.4 GHz directional panel antenna | 2 | Alfa APA-M04, 7 dBi, RP-SMA male | 9.97-13.99 |
| Logic level shifter, 4-ch | 2 | Adafruit 757 (BSS138) | 3.95-4.50 |
| Jumper wires F-F | 2 | pack of 20-40 | 3-6 |
| OV5647 camera, 15-pin **0.5 mm** ribbon | 3 | Waveshare OV5647 (the 4.3-C kit camera); standard RPi Camera (B) cables are 1.0 mm and do NOT fit | ~16-20 |
| IMU | 3 | Adafruit BNO085, STEMMA QT (adafruit.com/product/4754) | 24.95-29.50 |
| STEMMA QT to female header cable | 3 | Adafruit 4-pin | 0.95 |
| 3.7 V LiPo, MX1.25 plug | 3 | check polarity before connecting | 8-15 |
| Handle / mount | 3 | plywood, acrylic or 3D print; M2.5 standoffs | 5-15 |
| Screen | 3 | built into your P4 board (4.3 in, 480x800, touch) | 0 |

Total Phases 1-3: about $95-145 on top of R4, T3 and P4. XIAO stock antenna is weak; panel antenna replaces it in Phase 2.
Pigtail must be RP-SMA **female** to mate the Alfa's RP-SMA **male**.

## Software install (once, on HaL or HaLjr)
Arduino IDE for R4 + XIAO, ESP-IDF for P4.
1. Arduino IDE 2.x (arduino.cc/en/software).
2. Boards Manager: **Arduino UNO R4 Boards** (includes WiFiS3, LED Matrix).
3. Preferences > Additional boards URL `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, install **esp32 by Espressif Systems** (v3.x).
4. Boards: **Arduino UNO R4 WiFi**; **XIAO_ESP32S3**.
5. Linux: `sudo usermod -aG dialout $USER`, log out/in.
XIAO first flash: hold BOOT, plug USB-C, release BOOT (only if port missing).

ESP-IDF (P4): Waveshare recommends ESP-IDF over Arduino for the P4. HaLjr already has ESP-IDF v5.5.5 at `~/esp/esp-idf-v5.5.5`
(see HANDOFF.md); use the version Waveshare's ESP-IDF guide names if it differs. Flash Waveshare's camera and display demos first to prove hardware.
Windows only: CH343 USB-serial driver if the P4 UART port is missing.
Bench: soldering iron (XIAO, BNO085 headers), multimeter, flush cutters, heat-shrink.

## Phase 1 wiring (all on R4's 5 V side, no shifter)
| From | To | Note |
|---|---|---|
| R4 D8 | 100 ohm, end 1 | PWM tone |
| 100 ohm, end 2 | Buzzer + | longer leg / marked + |
| Buzzer - | R4 GND | any GND |
| Power bank USB | R4 USB-C | mobile power |
3-pin module instead: S -> D8, + -> 5V, - -> GND, no resistor.
Assemble unplugged; confirm D8 goes through the resistor. Buzzer test (3 beeps):
```cpp
void setup() {
  for (int i = 0; i < 3; i++) { tone(8, 2000, 150); delay(300); }
}
void loop() {}
```
Finder firmware: NOT WRITTEN YET (scan, lock target, beep faster with RSSI, LED-matrix bars).
Field test: phone hotspot in another room, select as target, walk toward it. Each scan takes a few seconds; walk slowly.

## Phase 2 wiring (R4 5 V <-> XIAO 3.3 V through level shifter)
| From | To | Note |
|---|---|---|
| R4 5V | Shifter HV | high reference |
| R4 5V | XIAO 5V pin | powers XIAO |
| R4 GND | Shifter GND (HV) | common |
| XIAO 3V3 | Shifter LV | low reference |
| XIAO GND | Shifter GND (LV) | common |
| R4 D1 (TX) | Shifter B1 | R4 -> XIAO |
| Shifter A1 | XIAO D7 (RX, GPIO44) | |
| XIAO D6 (TX, GPIO43) | Shifter A2 | XIAO -> R4 |
| Shifter B2 | R4 D0 (RX) | |
| XIAO u.FL | pigtail u.FL end | press straight down until it clicks |
| Pigtail RP-SMA | Alfa APA-M04 | hand-tight |
Buzzer stays on D8. R4 uses `Serial1` (D0/D1). XIAO: `Serial1.begin(115200, SERIAL_8N1, D7, D6)`; write pins WITH the D prefix.
Keep baud <= 115200 (shifter is slow above that).
Assembly: solder headers; flash XIAO alone first; unplug all, wire; click pigtail on BEFORE power, remove stock antenna; power R4 from USB.
**Warning:** never plug XIAO USB-C while its 5V pin is fed from the R4; pull the 5V jumper to reflash.
XIAO firmware: NOT WRITTEN YET (monitor mode, MAC tracking, vendor lookup, readings to R4).
Field test: phone on Wi-Fi behind a wall, pick its MAC, sweep panel slowly; tone peaks when aimed. Keep body behind antenna.

## Phase 3 wiring (P4 40-pin header, all 3.3 V, no shifter)
| From | To (header pin, signal) | Note |
|---|---|---|
| OV5647 ribbon | CSI connector | 15-pin 0.5 mm |
| 3.7 V LiPo | MX1.25 battery header | red = +; board charges over USB-C |
| BNO085 VIN | pin 2, 3V3 | |
| BNO085 GND | pin 5, GND | |
| BNO085 SDA | pin 4, GPIO7 | shared I2C with touch/audio/camera; BNO085 addr 0x4A, no clash |
| BNO085 SCL | pin 6, GPIO8 | |
| XIAO 5V | pin 1, 5V | |
| XIAO GND | pin 10, GND | |
| XIAO D6 (TX) | pin 11, GPIO5 (P4 RX) | direct |
| XIAO D7 (RX) | pin 14, GPIO4 (P4 TX) | direct |
| XIAO u.FL | pigtail -> panel antenna | same as Phase 2 |
| Optional pin 8, GPIO2 (P4 TX) | Shifter A3 -> B3 -> R4 D0 | keeps R4 as beeper; shifter LV = pin 18 3V3, HV = R4 5V, GND = pin 13 + R4 GND |
Pin numbers come from Waveshare's table for this board, not Raspberry Pi numbering (pin 1 is 5V). Match each wire to silkscreen before power. Leave GPIO37/38 (pins 7, 9) free: likely USB-UART console.
The repo's verified P4 facts are in `.claude/skills/lvgl-ui/board-p4.md`; check these pins against it before wiring.
Assembly: flash Waveshare camera demo and confirm image; build handheld plate (antenna face-forward, lens centred on antenna face, screen toward you);
mount BNO085 flat and rigid >= 5 cm from speaker, magnets, LiPo; wire with battery and USB out; plug LiPo, short-press POWER, I2C scan finds 0x4A.
Calibrate: hotspot 3 m away in open air; sweep, note peak vs on-screen position, enter offset in firmware; figure-8 for 20 s for IMU.
Phase 3 firmware (ESP-IDF): NOT WRITTEN YET (camera via LVGL, BNO085 heading/tilt, XIAO UART, heat map, crosshair).
Field test: hotspot behind wall shows as hot spot; stays put on sweeps.

## Safety rules
- Wire only unpowered; check against pin table first.
- Never 5 V on a 3.3 V pin (XIAO, P4, BNO085 data, T3). R4-to-ESP32 signals go through the level shifter.
- One power source per board at a time.
- Never transmit from the T3 LoRa radio without its antenna.
- Scan and track only your own network and devices.

## Troubleshooting
| Symptom | Check first | Then |
|---|---|---|
| No beep | polarity, resistor seated, D8 | run test sketch; try active buzzer |
| Board not in port list | data-capable USB cable, other port | XIAO hold BOOT; Linux dialout; Windows CH343 for P4 |
| Garbled serial between boards | TX->RX crossed, grounds tied, same baud | drop to 57600; confirm shifter LV/HV powered |
| XIAO sees almost nothing | pigtail fully clicked | try stock antenna; u.FL fragile, press straight |
| Signal jumps | walls/bodies reflect 2.4 GHz | average readings, sweep slowly, antenna away from body |
| P4 screen black | board powered (short-press POWER) | Waveshare LCD demo; charge LiPo or USB-C |
| No camera image | ribbon orientation, lens cap | Waveshare camera demo alone |
| BNO085 missing on I2C | VIN on 3V3, SDA/SCL not swapped | STEMMA QT cable; addr 0x4A |
| Heading drifts | magnets/speaker/power bank near IMU | move away; redo figure-8 |
| Power bank shuts off | low-current auto-off | bank with always-on mode, or small load |
