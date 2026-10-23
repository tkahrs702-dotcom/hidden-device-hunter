# Hidden Device Hunter (RF Finder) handoff
Last updated 2026-10-20. Owner Taelor. Source: Claude Android app docs, copied here by Claude Code on HaL.

## State
The active project scope is **V2: Phase 1 + Phase 2 only**. See [`docs/v2/README.md`](docs/v2/README.md) and [`docs/v2/HANDOFF.md`](docs/v2/HANDOFF.md) for the current entry point and detailed handoff. Phase 3 and Phase 4 manuals remain preserved as parked reference, not current build instructions; expansion requires a separate evidence-based review. Existing manuals have not passed a V2 audit.

Phase 2 hardware is the **Seeed Studio XIAO ESP32-C5**. Its firmware was flashed on 2026-10-18 and received a 5 GHz device on channel 36 and a 2.4 GHz access point on channel 2 (see merged PR #4 worklog). The Rokland 2.4/5 GHz directional antenna is ordered but not delivered. The XIAO-to-R4 UART, BOOT button behavior, and lock persistence remain untested. OV5647, BNO085, and LiPo remain unverified/not built.
Firmware (imported 2026-10-04 from a cloud-session bundle, see `docs/worklog/2026-10-04-firmware-import.md`): Phase 1 `firmware/phase1-r4-finder`,
Phase 2 shared sniffer `firmware/phase2-xiao-sniffer` + C5 build entry `firmware/phase2-c5-sniffer` + `firmware/phase2-r4-display`. The Seeed XIAO C5 uses D6 TX (GPIO11) and D7 RX (GPIO12) through the level shifter; generic C5 builds use RX1/TX1 GPIO4/GPIO5. The current local profile scans selected US non-DFS channels in both 2.4 and 5 GHz bands. Phase 1 compiled and was uploaded to the connected UNO R4 WiFi on 2026-10-19; serial confirmed Modulino detection, test toggle commands, and AUTO scan readings; audible output and field-finding performance remain unverified. Phase 3 (P4 camera): no firmware, notes only.
Known limits: Phase 1 R4 Wi-Fi scan max 10 networks and skips hidden SSIDs; Phase 1 is 2.4 GHz only; Phase 2 C5 scans selected channels and excludes DFS/6 GHz; random MACs carry no maker info.

## Decisions made on copy (2026-10-03)
- Level shifting: Build Plan said 1k+2k divider; Setup Guide (later, detailed) says Adafruit 757 shifter. Used the shifter everywhere (also needed for bidirectional UART).
- Cost: Setup Guide's $95-145 (checked prices) over Build Plan's $95-160.
- Toolchain: HaLjr already has ESP-IDF v5.5.5 and arduino-cli (see `~/Projects/sentinel/HANDOFF.md` sec. 4); guide's generic install steps only needed for Arduino IDE boards if the IDE is wanted.

## Open items
1. Claude app doc "Hidden Device Hunter" now holds a status summary (see Claude app copy below); the old 7 empty placeholder tabs were in the deleted original.
2. Setup Guide Sources list still has stale links (7-DSI-TOUCH-A, RPi Camera B); dropped in this copy.
3. P4 header pins, R4 tone() on D8, shifter and IMU details all verified 2026-10-03 (see `docs/manual/00-overview-and-safety.md`). Left: two bench multimeter checks (header 5V on LiPo, LiPo plug polarity) once the hardware is in hand.
4. Overlap with existing work: `firmware/ttgo-node` (undeployed anti-spy) and the R4 Wi-Fi/BLE scanner in `~/Projects/sentinel/docs/worklog/2026-09-29-r4-wifi-scanner.md`. Decide whether Phase 1/2 reuse that hub.
5. Phase 1 bench check (2026-10-19): see `docs/worklog/2026-10-19-stage1-audit-fixes.md` for upload and live serial results. Field test remains.
6. Phase 2 diagram and local firmware pin profile were corrected for the verified Seeed XIAO ESP32-C5. Power the boards separately over USB during UART tests; do not tie their 5 V rails. The level-shifted UART link still needs a bench test.

## Claude app copy
Claude app Project "Hidden Device Hunter" created 2026-10-04 in the Projects tab (https://claude.ai/code/project/chan_01D93rrNRNTZUUnDzP1evSvQ; goal, status summary in Project instructions, and the GitHub repo attached; this Project type has no file upload, so refresh the instructions from this file when it changes). Status doc: https://claude.ai/artifact/JRZNRaVzCsuKZQ8Rz8ohqj (status mirrored from this file; repo is the source of truth, refresh the doc when this changes). Duplicate cloud docs deleted 2026-10-04. Companion docs (renamed 2026-10-04 from "Sentinel RF Finder"): Build Plan https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e and Setup Guide https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1. Their in-body titles still say "Sentinel RF Finder"; the gallery list may lag the rename.

## Manual
Full manual written 2026-10-03 in `docs/manual/` (pin-to-pin tables, ASCII diagrams, assembly, software, tests). Pin data checked against the Waveshare schematic and docs, Seeed, Arduino core source and Adafruit board files.

## Next step
Bench-test the XIAO C5-to-R4 UART through the level shifter with each board powered separately by USB. When the ordered Rokland directional antenna arrives, confirm its connector/cable before use. Keep Phase 3/4 parked; only begin expansion after a separate evidence-based review. Do not treat RSSI changes as precise distance measurements.
