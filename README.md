# Sentinel RF Finder (a.k.a. Hidden Device Hunter)

Handheld finder for hidden Wi-Fi / Bluetooth devices in Taelor's own home. Written 2026-10-03 in the
Claude Android app; copied here so every computer has it. Status: **planning only, nothing built or bought yet**
(every phase "Not started" in the cloud doc).

## Docs here
| File | What |
|---|---|
| [`HANDOFF.md`](HANDOFF.md) | Start here: status, decisions, open items, next step |
| [`docs/build-plan.md`](docs/build-plan.md) | The four phases, roles, parts list |
| [`docs/setup-guide.md`](docs/setup-guide.md) | Priced parts with links, installs, wiring tables, assembly, tests, troubleshooting |

## Cloud originals (live Claude Docs, private to Taelor)
- Hidden Device Hunter (8 tabs: Overview, Parts & shopping, Software setup, Phases 1-4, Troubleshooting): https://claude.ai/artifact/6Hofsp6NwQrS2nxhyiZN5e. Only its Overview tab has content (summarised below); the other 7 tabs are empty placeholders.
- Build Plan: https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e
- Setup Guide: https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1

## Summary
| Phase | Build | Test |
|---|---|---|
| 1 | Uno R4 WiFi + passive buzzer on D8 (100 ohm in series). Scan, pick target, beep rate and LED-matrix bars follow RSSI | Hide phone hotspot in another room; beeps speed up on approach |
| 2 | XIAO ESP32-S3 (u.FL) + Alfa APA-M04 7 dBi panel antenna. Monitor mode, lock one MAC, vendor lookup, UART to R4 via level shifter | Phone on Wi-Fi behind a wall; tone peaks when panel points at it |
| 3 | Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3 + OV5647 camera + BNO085 IMU. Camera on built-in screen (LVGL), heat-map overlay and crosshair; XIAO moves to P4 40-pin header | Hotspot behind wall shows as hot spot, stays put when sweeping |
| 4 | Optional: BLE tracker mode, lens-glint LED ring, Wi-Fi CSI presence, AD8317 bug detector, floor-plan heat map on microSD, T3 LoRa remote nodes, phone alerts | n/a |

Limits: finds only devices that transmit; 2.4 GHz only (5 GHz-only devices invisible); own home and network only.
Wiring rule: R4 is 5 V logic, every ESP32 is 3.3 V. Never put 5 V on an ESP32 pin.

Relation to SENTINEL: reuses the P4 and T3. The older anti-spy node (`firmware/ttgo-node`) and R4 Wi-Fi/BLE scanner
(`docs/worklog/2026-09-29-r4-wifi-scanner.md`) cover the same goal at room scale; this is the directional, handheld version.
