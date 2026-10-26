# Hidden Device Hunter: Build Plan
Copied 2026-10-03 from the Claude Docs original (https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e).

Goal: handheld finder for hidden Wi-Fi and Bluetooth devices, four phases that each reuse the last.

## Roles
| Device | Role |
|---|---|
| Arduino Uno R4 WiFi | Phase 1 brain; later beeper, LED alarm, Wi-Fi scanner streaming over USB |
| LILYGO T3 (LoRa) | Dropped 2026-10-26: that board belongs to SENTINEL. Phase 4 LoRa nodes would need their own boards |
| XIAO ESP32-S3 (to buy) | RF eye: monitor mode, BLE, external directional antenna |
| Waveshare ESP32-P4 | Phase 3 RF camera: camera, screen, heat-map overlay |
| HaL / HaLjr | Flash firmware, logging, BT scanning over USB |

## Phase 1: Wi-Fi finder with buzzer
Parts: Uno R4 WiFi, passive piezo buzzer, 100 ohm resistor, USB power bank.
- [ ] Buzzer + 100 ohm between D8 and GND
- [ ] Finder firmware: scan, pick target, RSSI -> beep speed and LED matrix bars
- [ ] Target switching (button or Serial command)
- [ ] Test: hide phone hotspot in another room

## Phase 2: Hidden clients + directional antenna
Parts: XIAO ESP32-S3 (u.FL), u.FL to RP-SMA pigtail, 2.4 GHz panel antenna (8-14 dBi), level shifter.
- [ ] Monitor-mode firmware; lock onto one MAC
- [ ] Vendor lookup to flag camera brands
- [ ] XIAO <-> R4 UART (through level shifter)
- [ ] Test: point at a phone on your Wi-Fi through a wall

## Phase 3: RF camera on the P4
Parts: OV5647 camera (15-pin 0.5 mm ribbon), BNO085 IMU, 3.7 V LiPo (MX1.25), mount. Screen is built into the Touch-LCD-4.3.
- [ ] Antenna parallel to camera
- [ ] P4 camera feed on screen (LVGL)
- [ ] IMU pointing angle + RSSI from XIAO over UART
- [ ] Red/blue hot spots on image, crosshair readout
- [ ] R4 stays as proximity alarm
- [ ] Test: hotspot behind wall shows as hot spot

## Phase 4: options (any order)
BLE tracker mode (AirTag, Tile, BLE bugs); lens finder (red LED ring); Wi-Fi CSI motion; AD8317 broadband bug detector;
whole-house heat map to microSD; T3 LoRa remote nodes; phone alerts for new unknown devices.

## Parts (approx. USD; Phases 1-3 about $95-160 new, Setup Guide's checked prices give $95-145)
Have: R4 WiFi, Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3, LILYGO T3, HaL/HaLjr.
Buy: buzzer + 100 ohm (1-2), USB bank (0-20), XIAO ESP32-S3 u.FL (8-15), u.FL to RP-SMA pigtail (3-5), panel antenna (15-30),
level shifter, OV5647 (15-20), BNO085 (10-20), LiPo + mount (5-15). Full priced list with links: `setup-guide.md`.
