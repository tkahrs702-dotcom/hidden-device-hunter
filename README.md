# Hidden Device Hunter (RF Finder)

Handheld finder for hidden Wi-Fi and Bluetooth devices in Taelor's own home. Started in the Claude Android app 2026-10-03.
Status: Phase 1 firmware is uploaded and bench-checked. The Seeed Studio XIAO ESP32-C5 Phase 2 firmware was flashed and received 2.4/5 GHz traffic; its UART link to the R4 is still untested. The Rokland 2.4/5 GHz directional antenna is ordered but not delivered.
**Current scope: V2 covers Phase 1 and Phase 2 only.** Start with [`docs/v2/README.md`](docs/v2/README.md) and its handoff. The older Phase 1 passive-buzzer instructions are archived; use the V2 Phase 1 guide for the Qwiic Modulino Buzzer.

Phase 3 (P4 camera) and Phase 4 (optional add-ons) are parked for later research. Their existing files are preserved, not current build instructions.

## Where we left off

Updated 2026-10-26. V2 (Phase 1 + Phase 2) is built and bench-tested; Phases 3 and 4 are parked. Session note: [`docs/sessions/2026-10-26-01YKZUS1.md`](docs/sessions/2026-10-26-01YKZUS1.md).
- State: the XIAO ESP32-C5 sniffer locks the default Tuya plug on boot and the UNO R4 WiFi shows bars, a peak dot (closest approach) and beeps. The R4 also serves a phone page (Wi-Fi `HDH-Finder`, `http://192.168.4.1`) with live dBm, a graph, a recommendation and up to 60 devices; hidden networks rank with the suspects and the page shows the BOOT presses that lock each. PRs #9 to #14 are merged and CI compiles every sketch. The phone page has not run on hardware yet. Details: [`docs/worklog/2026-10-26-c5-firmware-review.md`](docs/worklog/2026-10-26-c5-firmware-review.md).
- READ FIRST (2026-10-27): Taelor says the current setup is too fiddly to work with; rethink for ease of use before adding features, see [`docs/v2/NOTE-ease-of-use-redesign.md`](docs/v2/NOTE-ease-of-use-redesign.md). Bluetooth scan and Wi-Fi/Bluetooth switch are on PR #16 (green, untested on hardware, default Wi-Fi only).
- Planned: volume control and per-device check, see [`docs/v2/plan-volume-and-device-check.md`](docs/v2/plan-volume-and-device-check.md).
- Flash and test steps, all in one page: [`docs/FLASH-AND-TEST.md`](docs/FLASH-AND-TEST.md).
- Next: Taelor's walk test with the phone page, then the Phase 1 field test. Flash the C5 by holding BOOT while plugging it in, using its `/dev/serial/by-id` path; power both boards from one 2-port bank. Steps: [`docs/v2/HANDOFF.md`](docs/v2/HANDOFF.md).
- Open questions: the Rokland 2.4/5 GHz antenna has not arrived; does its connector and cable fit? Two meter checks (header 5 V on LiPo, LiPo plug polarity) wait for the hardware. Phase 3, if resumed, needs this project's own ESP32-P4.

## Quick start (HaL or HaLjr)
```
cd ~/Projects/hidden-device-hunter && git pull     # first time: see docs/computers.md to clone
tools/setup.sh                                     # toolchain check + compile all sketches + list boards
```
Then open the phase you are on: [V2 Phase 1](docs/v2/01-phase1.md) (start here), [Phase 2](docs/manual/02-phase2-xiao-antenna.md).
Phase 3 is parked. [`docs/computers.md`](docs/computers.md) has the folder layout, the per-session routine,
how to tell boards apart by serial number, and copy-paste flash commands.

## Repo map
| Path | What |
|---|---|
| [Where we left off](#where-we-left-off) | current state and next step: read first |
| [`docs/reference.md`](docs/reference.md) | early decisions, cloud copies, manual notes |
| [`docs/manual/`](docs/manual) | build manual with diagrams (table below) |
| [`docs/computers.md`](docs/computers.md) | HaL / HaLjr setup and routine |
| [`docs/shopping-list.txt`](docs/shopping-list.txt) | parts to buy, with prices and links |
| [`docs/worklog/`](docs/worklog) | one file per session |
| [`firmware/`](firmware) | `phase1-r4-finder`, `phase2-r4-display`, `phase2-xiao-sniffer`, `phase3-p4-rfcam` (placeholder) |
| [`tools/`](tools) | `setup.sh`, `make_oui_table.py` (camera-maker table), `diagrams/` (SVG generator) |

## Manual (`docs/manual/`, read in order)
| File | What |
|---|---|
| [`00-overview-and-safety.md`](docs/manual/00-overview-and-safety.md) | System diagram, how to read the diagrams, voltage rule, safety, tools, software install, verification log |
| [`01-phase1-finder.md`](docs/manual/01-phase1-finder.md) | Archived passive-buzzer build; do not use for current hardware |
| [`02-phase2-xiao-antenna.md`](docs/manual/02-phase2-xiao-antenna.md) | Seeed Studio XIAO ESP32-C5, updated wiring and connection schematic, pin-to-pin UART table, antenna and test status |
| [`03-phase3-p4-rf-camera.md`](docs/manual/03-phase3-p4-rf-camera.md) | ESP32-P4 camera screen + BNO085: verified header table, wiring diagram, schematic, wire list, calibration |
| [`04-phase4-options.md`](docs/manual/04-phase4-options.md) | Optional add-ons |
| [`05-reference.md`](docs/manual/05-reference.md) | Diagram index, wire colours, every wire by ID, signals, line protocol, power, pin cheats |
| [`diagrams/`](docs/manual/diagrams) | 8 SVG diagrams, generated by `tools/diagrams/make_diagrams.py` |

Other docs: [`docs/reference.md`](docs/reference.md) (early decisions and notes), [`docs/build-plan.md`](docs/build-plan.md) and [`docs/setup-guide.md`](docs/setup-guide.md) (the original Android-app docs, kept as source), [`docs/worklog/`](docs/worklog/).

## Cloud originals (live Claude Docs, private to Taelor)
Hidden Device Hunter (only Overview tab has content): https://claude.ai/artifact/6Hofsp6NwQrS2nxhyiZN5e. Build Plan: https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e. Setup Guide: https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1.

## Phases
| Phase | Build | Test |
|---|---|---|
| 1 | Uno R4 WiFi + Modulino Buzzer on Qwiic; scans Wi-Fi and maps RSSI to beeps and LED bars | Uploaded; serial bench check passed, field test pending |
| 2 | Seeed Studio XIAO ESP32-C5 dual-band sniffer; level-shifted UART to R4; Rokland directional antenna ordered | Firmware received 2.4/5 GHz traffic; UART and antenna tests pending |
| 3 | Parked: Waveshare ESP32-P4 camera-screen research | Not built; no field test |
| 4 | Parked: optional additions | Not built |

Limits: finds only devices that transmit; Phase 2 hops channels and does not receive both bands simultaneously. Own home and network only.
Wiring rule: R4 is 5 V logic, every ESP32 is 3.3 V. Never put 5 V on an ESP32 pin.
