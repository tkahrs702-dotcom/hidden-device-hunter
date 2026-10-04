# 00. Overview, safety, tools and software

Manual for the Hidden Device Hunter. Read this first, then do Phase 1, 2, 3 in order. Each phase file stands alone:
parts, pin-to-pin wiring, ASCII diagram, assembly, software, test, troubleshooting.
Status of the project: planning only; no parts ordered, no firmware written (see `../../HANDOFF.md`).

Legal: use only on your own home, network and devices. The finder passively listens; it does not break into or join anything.

## How the pieces fit
```
                      PHASE 1                    PHASE 2                       PHASE 3
                 +--------------+          +----------------+            +----------------------+
 Wi-Fi in air -->| Uno R4 WiFi  |          | XIAO ESP32-S3  |  UART      | Waveshare ESP32-P4   |
 (R4 scans)      | scan + beep  |<--UART---| monitor mode   |----------->| camera + LVGL screen |
                 | LED matrix   | (shifter)| panel antenna  |  (direct,  | heat map + crosshair |
                 +--------------+          +----------------+   3.3 V)   |  + BNO085 IMU (I2C)  |
                       |                                                  +----------+-----------+
                     buzzer                                      R4 stays optional beeper (via shifter)
```
Phase 1 works alone. Phase 2 adds the XIAO as a sharper "RF eye". Phase 3 moves the display and aiming to the P4.

## Voltage rule (the one that destroys boards)
| Board | Logic level |
|---|---|
| Uno R4 WiFi (RA4M1) | **5 V** |
| XIAO ESP32-S3, ESP32-P4, BNO085 data lines, T3 | **3.3 V** |
Never connect an R4 pin straight to an ESP32 pin. R4 <-> ESP32 goes through the Adafruit 757 level shifter. XIAO <-> P4 is 3.3 V both sides, direct.

## Safety rules
1. Wire only with everything unplugged. Check the pin table before applying power.
2. One power source per board at a time (see the XIAO USB + 5 V pin warning in Phase 2).
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
3. Add board URL `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, install **esp32 by Espressif Systems** v3.x. Boards: `XIAO_ESP32S3`.
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
| R4 WiFi: D0 = RX, D1 = TX, Serial1; 5 V logic; 8 mA per pin, 60 mA total; 5 V pin ~4.7 V on USB | Verified | Arduino docs via lastminuteengineers |
| R4 `tone()` works on D8 (any digital pin; timer-interrupt toggling) | Verified | ArduinoCore-renesas `Tone.cpp` |
| Adafruit 757 pin layout: LV, A1-A4, GND on one side; HV, B1-B4, GND on the other; 10K pull-ups | Verified | adafruit/4-Channel-Level-Shifter-PCB board file |
| BNO085 default address 0x4A; alternate 0x4B when DI is pulled high (NOT an ADR jumper); VIN 3-5 V; 10K I2C pull-ups on board | Verified (manual corrected) | learn.adafruit.com BNO085 pinouts |
| P4 battery connector J2: pin 1 = BAT, pin 2 = GND | Verified in schematic (physical pin 1 position: confirm on the board) | Waveshare schematic PDF |
| P4 header 5V = VCC_5V rail, fed from USB or battery boost (Q7); not gated by the power button | Verified in schematic; **live-measure once** | Waveshare schematic PDF |
| P4 CSI: 15-pin 0.5 mm, MIPI 2-lane; OV5647 supported | Verified | Waveshare product page |
