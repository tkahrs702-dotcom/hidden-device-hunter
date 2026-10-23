# 00. Overview, safety, tools and software

Manual for the Hidden Device Hunter. Read this first, then do Phase 1, 2, 3 in order. Each phase file stands alone:
parts, pin-to-pin wiring, ASCII diagram, assembly, software, test, troubleshooting.
Status: Phase 1 is uploaded; serial confirmed Modulino detection and Wi-Fi scan output, but audible and field-finding behavior are not confirmed. The Seeed Studio XIAO ESP32-C5 has received 2.4 and 5 GHz traffic; its link to the R4 is not yet tested (see `../../HANDOFF.md`).

Legal: use only on your own home, network and devices. The finder passively listens; it does not break into or join anything.

## How the pieces fit
```
                      PHASE 1                    PHASE 2                       PHASE 3
                 +--------------+          +----------------+            +----------------------+
 Wi-Fi in air -->| Uno R4 WiFi  |          | Seeed XIAO     |  UART      | Waveshare ESP32-P4   |
 (R4 scans)      | scan + beep  |<--UART---| ESP32-C5       |----------->| camera + LVGL screen |
                 | LED matrix   | (shifter)| monitor mode   | (shifter)  | heat map + crosshair |
                 +--------------+          +----------------+   3.3 V)   |  + BNO085 IMU (I2C)  |
                       |                                                  +----------+-----------+
                     buzzer                                      R4 stays optional beeper (via shifter)
```
Phase 1 works standalone with its saved target or AUTO mode. Phase 2 uses the dual-band Seeed XIAO ESP32-C5 as a sharper "RF eye". Phase 3 moves the display and aiming to the P4.

## Voltage rule (the one that destroys boards)
| Board | Logic level |
|---|---|
| Uno R4 WiFi (RA4M1) | **5 V** |
| Seeed XIAO ESP32-C5, ESP32-P4, BNO085 data lines, T3 | **3.3 V** |
Never connect an R4 pin straight to an ESP32 pin. R4 <-> ESP32 goes through a suitable level shifter. ESP32 <-> P4 is 3.3 V both sides, direct.

## Safety rules
1. Wire only with everything unplugged. Check the pin table before applying power.
2. For the first C5-to-R4 UART test, power the R4 and XIAO C5 separately over USB. Do not connect their 5 V rails together; the Phase 2 link needs shared ground and level-shifted UART only.
3. Never transmit from the T3 LoRa radio without its antenna.
4. Do not hot-plug the u.FL antenna connector; click it on before power.
5. Check LiPo plug polarity against the board silkscreen before connecting (cheap cells are wired both ways).
6. Own network and devices only.

## Tools
Soldering iron and solder (XIAO headers, BNO085 headers), multimeter, flush cutters, heat-shrink, small Phillips screwdriver,
USB-C data cables (not charge-only), breadboard or Dupont jumpers (F-F pack), hot glue or standoffs for mounting.

## Software install (once, on HaL or HaLjr)
HaLjr already has ESP-IDF v5.5.5 (`~/esp/esp-idf-v5.5.5`) and `arduino-cli` (see `~/Projects/sentinel/HANDOFF.md` sec. 4).
1. Arduino IDE 2.x, or `arduino-cli`.
2. Install core **Arduino UNO R4 Boards** (`arduino:renesas_uno`): includes WiFiS3 and Arduino_LED_Matrix.
3. Add board URL `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, install **esp32 by Espressif Systems** v3.x. Phase 2 target: **XIAO_ESP32C5** (`esp32:esp32:XIAO_ESP32C5`); enable USB CDC on boot for the USB serial console.
4. Linux serial permission: `sudo usermod -aG dialout $USER`, then log out and in.
5. P4 uses ESP-IDF (`. ~/esp/esp-idf-v5.5.5/export.sh`). Silicon rev quirks and BSP are in `~/.claude/skills/lvgl-ui/board-p4.md`: read it before building anything for the P4.
CLI equivalents:
```
arduino-cli core install arduino:renesas_uno
arduino-cli core install esp32:esp32   # after adding the board URL to the config
arduino-cli board list                 # shows which /dev/ttyACM* is which board
```

## Verification log (checked 2026-10-03)
| Fact | Status | Source |
|---|---|---|
| P4 40-pin header table (pins 1-40, GPIO numbers) | Verified | docs.waveshare.com/ESP32-P4-WIFI6-Touch-LCD-4.3 |
| P4 GPIO2/3/4/5 are free header GPIOs; I2C on GPIO7/8 shared with touch/audio/camera | Verified (header table, board-p4.md) | same, `board-p4.md` |
| P4 USBD_N/USBD_P pins 25/27 not usable as GPIO | Verified | Waveshare docs |
| XIAO ESP32-S3: D6 = GPIO43 (TX), D7 = GPIO44 (RX), 3V3 pin 700 mA, 5V pin dead on battery | Verified | wiki.seeedstudio.com xiao_esp32s3_getting_started |
| Seeed Studio XIAO ESP32-C5: D6=GPIO11, D7=GPIO12, BOOT=GPIO28 | Verified from installed XIAO variant; BOOT behavior still needs bench test | Arduino-ESP32 3.3.12 XIAO_ESP32C5 variant and merged PR #4 worklog |
| R4 WiFi: D0 = RX, D1 = TX, Serial1; 5 V logic; 8 mA per pin, 60 mA total; 5 V pin ~4.7 V on USB | Verified | Arduino docs via lastminuteengineers |
| R4 `tone()` works on D8 (any digital pin; timer-interrupt toggling) | Verified | ArduinoCore-renesas `Tone.cpp` |
| Adafruit 757 pin layout: LV, A1-A4, GND on one side; HV, B1-B4, GND on the other; 10K pull-ups | Verified | adafruit/4-Channel-Level-Shifter-PCB board file |
| BNO085 default address 0x4A; alternate 0x4B when DI is pulled high (NOT an ADR jumper); VIN 3-5 V; 10K I2C pull-ups on board | Verified (manual corrected) | learn.adafruit.com BNO085 pinouts |
| P4 battery connector J2: pin 1 = BAT, pin 2 = GND | Verified in schematic (physical pin 1 position: confirm on the board) | Waveshare schematic PDF |
| P4 header 5V = VCC_5V rail, fed from USB or battery boost (Q7); not gated by the power button | Verified in schematic; **live-measure once** | Waveshare schematic PDF |
| P4 CSI: 15-pin 0.5 mm, MIPI 2-lane; OV5647 supported | Verified | Waveshare product page |
