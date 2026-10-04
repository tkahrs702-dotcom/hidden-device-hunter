# Hidden Device Hunter (RF Finder) handoff
Last updated 2026-10-04. Owner Taelor. Source: Claude Android app docs, copied here by Claude Code on HaL.

## State
No parts ordered (XIAO, antenna, pigtail, shifter, OV5647, BNO085, LiPo all still "Buy").
Firmware (imported 2026-10-04 from a cloud-session bundle, see `docs/worklog/2026-10-04-firmware-import.md`): Phase 1 `firmware/phase1-r4-finder`,
Phase 2 `firmware/phase2-xiao-sniffer` + `firmware/phase2-r4-display`, `tools/make_oui_table.py` (771-entry maker table). All compile
(arduino-cli 1.3.1, renesas_uno 1.6.0, esp32 3.3.12) but are NOT run on hardware. Phase 3 (P4 camera): no firmware, notes only.
Known limits: R4 Wi-Fi scan max 10 networks and skips hidden SSIDs; 2.4 GHz only; random MACs carry no maker info.

## Decisions made on copy (2026-10-03)
- Level shifting: Build Plan said 1k+2k divider; Setup Guide (later, detailed) says Adafruit 757 shifter. Used the shifter everywhere (also needed for bidirectional UART).
- Cost: Setup Guide's $95-145 (checked prices) over Build Plan's $95-160.
- Toolchain: HaLjr already has ESP-IDF v5.5.5 and arduino-cli (see `~/Projects/sentinel/HANDOFF.md` sec. 4); guide's generic install steps only needed for Arduino IDE boards if the IDE is wanted.

## Open items
1. Claude app doc "Hidden Device Hunter" now holds a status summary (see Claude app copy below); the old 7 empty placeholder tabs were in the deleted original.
2. Setup Guide Sources list still has stale links (7-DSI-TOUCH-A, RPi Camera B); dropped in this copy.
3. P4 header pins, R4 tone() on D8, shifter and IMU details all verified 2026-10-03 (see `docs/manual/00-overview-and-safety.md`). Left: two bench multimeter checks (header 5V on LiPo, LiPo plug polarity) once the hardware is in hand.
4. Overlap with existing work: `firmware/ttgo-node` (undeployed anti-spy) and the R4 Wi-Fi/BLE scanner in `~/Projects/sentinel/docs/worklog/2026-09-29-r4-wifi-scanner.md`. Decide whether Phase 1/2 reuse that hub.
5. Needs Taelor: order parts; confirm go on Phase 1 firmware.

## Claude app copy
Claude Project doc "Hidden Device Hunter": https://claude.ai/artifact/JRZNRaVzCsuKZQ8Rz8ohqj (status mirrored from this file; repo is the source of truth, refresh the doc when this changes). Duplicate cloud docs deleted 2026-10-04. Companion docs (renamed 2026-10-04 from "Sentinel RF Finder"): Build Plan https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e and Setup Guide https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1. Their in-body titles still say "Sentinel RF Finder"; the gallery list may lag the rename.

## Manual
Full manual written 2026-10-03 in `docs/manual/` (pin-to-pin tables, ASCII diagrams, assembly, software, tests). Pin data checked against the Waveshare schematic and docs, Seeed, Arduino core source and Adafruit board files.

## Next step
Phase 1 firmware exists; flash it to the R4 (buzzer + resistor) and test with a phone hotspot. Find ports with `arduino-cli board list` (serial number, not ttyACM order).
