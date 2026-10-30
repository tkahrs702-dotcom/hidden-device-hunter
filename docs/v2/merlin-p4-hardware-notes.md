# Merlin (Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3): verified hardware facts

Written 10-30-2026. Sources are Waveshare's own repositories on GitHub (Apache-2.0), read on 10-30-2026:
- Product repo: `waveshareteam/ESP32-P4-WIFI6-Touch-LCD-4.3` (README, `docs/HARDWARE.md`, `schematic/ESP32-P4-WIFI6-Touch-LCD-4.3-schematic.pdf`, PCB label Rev1.2).
- BSP: `waveshareteam/Waveshare-ESP32-components`, `bsp/esp32_p4_wifi6_touch_lcd_4_3` (the registry package `waveshare/esp32_p4_wifi6_touch_lcd_4_3` 1.0.1 that Merlin builds against).
- Taelor's own board photos (10-30-2026): the silkscreen labels along the header, the speaker socket, the C6-UART pins.

## 40-pin header (connector J3 on the schematic)
Pin 1 is the 5V end. Odd pins are the row with the 5V labels (the lower row in the 10-30 photo, left column in the diagram). The schematic and the silkscreen photo agree on every pin. The board prints GPIO numbers only, not pin numbers.

| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | 5V | 2 | 3V3 |
| 3 | 5V | 4 | GPIO7 (I2C SDA, shared with touch and audio codec) |
| 5 | GND | 6 | GPIO8 (I2C SCL, shared) |
| 7 | GPIO37 | 8 | GPIO2 |
| 9 | GPIO38 | 10 | GND |
| 11 | GPIO5 | 12 | GPIO3 |
| 13 | GND | 14 | GPIO4 |
| 15 | GPIO21 | 16 | GPIO28 |
| 17 | GPIO22 | 18 | 3V3 |
| 19 | GND | 20 | GPIO29 |
| 21 | GPIO24 (USB1 D-) | 22 | GPIO30 |
| 23 | GPIO25 (USB1 D+) | 24 | GPIO31 |
| 25 | USB D- | 26 | GND |
| 27 | USB D+ | 28 | GPIO34 |
| 29 | GND | 30 | GPIO35 |
| 31 | GPIO32 | 32 | GPIO49 |
| 33 | GND | 34 | GPIO50 |
| 35 | GPIO46 | 36 | GPIO51 |
| 37 | GPIO47 | 38 | GPIO52 |
| 39 | GPIO48 | 40 | GND |

Pins 1 and 3 are the board's 5V rail (with a protection diode); pins 2 and 18 are 3V3. GPIO24/25 share the USB1 data lines.

## Pins Merlin uses for itself (do not use on the header)
| Function | GPIOs |
|---|---|
| I2C (touch GT911, audio codec) | 7 (SDA), 8 (SCL), also on header pins 4 and 6 |
| Audio I2S (ES8311) | 9 DOUT, 10 LRCK, 11 DIN, 12 SCLK, 13 MCLK; power-amp enable 53 |
| Display | backlight 26, reset 27 (the MIPI-DSI data lanes have their own dedicated pins) |
| microSD | 39 to 44 (D0 to D3 = 39 to 42, CLK 43, CMD 44) |
| Camera | MIPI-CSI ribbon socket, no GPIOs |
| ESP32-C6 (Wi-Fi/Bluetooth) | SDIO to the C6, not on the header |

## Choices made for the project, and why
- **GPIO5 (pin 11) RX and GPIO4 (pin 14) TX** for Nimue; **GPIO21 (pin 15) RX and GPIO22 (pin 17) TX** for Iolaus. None of these appear in the BSP's used-pin list or in the schematic's other blocks; GPIO4/5/21/22 are ordinary GPIOs. The ESP32-P4 can route any UART to any free GPIO.
- **Avoid** GPIO7/8 (shared I2C), GPIO37/38 (very likely the USB-to-UART port's UART0, plus strapping), GPIO34 to 38 (strapping pins on the P4, from the chip datasheet, not re-checked here), and GPIO24/25 (USB).
- **Sound:** speaker through the ES8311 codec and NS4150B amplifier, confirmed in the schematic notes (`docs/HARDWARE.md`) and BSP (`bsp_audio_codec_speaker_init`, amp enable GPIO53). The speaker is the white plate on a red/black wire, plugged into the 2-pin socket above USB TO UART, by MIC1.
- **Silicon:** Waveshare's default profile is chip revision 3.x (`CONFIG_ESP32P4_REV_MIN_300`, 250 MHz PSRAM); a Rev1.3 compatibility profile exists. Merlin's `sdkconfig.defaults` uses the Rev1.3 profile because the 10-28 draft noted the board is v1.3. If Merlin boot-loops or reports a chip revision error, try the Rev3.x profile (see `docs/HARDWARE.md` in the product repo for the exact settings).

## Not confirmed
- Nothing has been run on the physical board.
- The 5V and 3V3 rows are confirmed from the schematic; meter check 1 in `docs/merlin-build-guide.md` still verifies them on the real board.
- The ESP-Hosted SDIO pins to the C6 (believed to be GPIO14 to 19 plus a reset line, as on Espressif's P4 boards) were not read from the schematic.

## Notes for future work (not researched yet)
- **Camera (arrives Saturday, OV5647):** MIPI-CSI 15-pin 2-lane socket; Waveshare's example `09_video_lcd_display` shows a camera preview on this screen. Phase 3.
- **Android app:** reads Nimue's Bluetooth service ("Ananizapta"); keep the line protocol and GATT stable.
- **LoRa (Iolaus/Argus):** both TTGO T3 V1.6.1 boards run 915 MHz, spreading factor 9, bandwidth 125 kHz, sync word 0x5A. Same settings on both or they will not hear each other.
- **RD-03DE radar, Home Assistant:** not read. The private repos `rd03d-lora-presence` and `home-assistant` are the starting point if Taelor wants this project to link up with them; `sentinel` stays untouched (10-26 rule).
