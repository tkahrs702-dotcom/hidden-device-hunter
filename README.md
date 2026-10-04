# Hidden Device Hunter (RF Finder)

Handheld finder for hidden Wi-Fi and Bluetooth devices in Taelor's own home. Started in the Claude Android app 2026-10-03.
Status: **planning only**. Nothing built or bought, no firmware written. The manual below is complete for wiring, assembly, software setup and testing; the firmware itself is the next work.

## Manual (`docs/manual/`, read in order)
| File | What |
|---|---|
| [`00-overview-and-safety.md`](docs/manual/00-overview-and-safety.md) | System diagram, voltage rule, safety, tools, software install, verification log |
| [`01-phase1-finder.md`](docs/manual/01-phase1-finder.md) | R4 WiFi + buzzer: parts, pin-to-pin wiring, diagram, assembly, test sketches, troubleshooting |
| [`02-phase2-xiao-antenna.md`](docs/manual/02-phase2-xiao-antenna.md) | XIAO ESP32-S3 + panel antenna + level shifter: wiring, diagram, UART link test |
| [`03-phase3-p4-rf-camera.md`](docs/manual/03-phase3-p4-rf-camera.md) | ESP32-P4 camera screen + BNO085: full verified header table, wiring, diagram, calibration |
| [`04-phase4-options.md`](docs/manual/04-phase4-options.md) | Optional add-ons |
| [`05-reference.md`](docs/manual/05-reference.md) | Signals, power, pin cheats, open verification items |

Other docs: [`HANDOFF.md`](HANDOFF.md) (state, decisions, next step), [`docs/build-plan.md`](docs/build-plan.md) and [`docs/setup-guide.md`](docs/setup-guide.md) (the original Android-app docs, kept as source), [`docs/worklog/`](docs/worklog/).

## Cloud originals (live Claude Docs, private to Taelor)
Hidden Device Hunter (only Overview tab has content): https://claude.ai/artifact/6Hofsp6NwQrS2nxhyiZN5e. Build Plan: https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e. Setup Guide: https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1.

## Phases
| Phase | Build | Test |
|---|---|---|
| 1 | Uno R4 WiFi + passive buzzer on D8 (100 ohm in series); beep rate and LED bars follow RSSI | Hotspot hidden in another room |
| 2 | XIAO ESP32-S3 (u.FL) + Alfa APA-M04 panel antenna; monitor mode, lock one MAC, UART to R4 via shifter | Phone behind a wall; tone peaks when aimed |
| 3 | Waveshare ESP32-P4 4.3 in + OV5647 + BNO085; heat-map overlay and crosshair | Hotspot behind wall shows as hot spot |
| 4 | Optional extras | n/a |

Limits: finds only devices that transmit; 2.4 GHz only. Own home and network only.
Wiring rule: R4 is 5 V logic, every ESP32 is 3.3 V. Never put 5 V on an ESP32 pin.
Relation to SENTINEL: separate project; reuses the P4 and T3 boards and may reuse `~/Projects/sentinel/tools/r4-wifi-scanner`.
