# Hidden Device Hunter (RF Finder) handoff
Last updated 2026-10-03. Owner Taelor. Source: Claude Android app docs, copied here by Claude Code on HaL.

## State
Planning only. No parts ordered (XIAO, antenna, pigtail, shifter, OV5647, BNO085, LiPo all still "Buy"). No firmware written
for any phase (finder, XIAO sniffer, P4 camera/heat map). Cloud progress table: all phases Not started.

## Decisions made on copy (2026-10-03)
- Level shifting: Build Plan said 1k+2k divider; Setup Guide (later, detailed) says Adafruit 757 shifter. Used the shifter everywhere (also needed for bidirectional UART).
- Cost: Setup Guide's $95-145 (checked prices) over Build Plan's $95-160.
- Toolchain: HaLjr already has ESP-IDF v5.5.5 and arduino-cli (see `~/Projects/sentinel/HANDOFF.md` sec. 4); guide's generic install steps only needed for Arduino IDE boards if the IDE is wanted.

## Open items
1. Cloud doc "Hidden Device Hunter": only the Overview tab has content. The other 7 tabs (Parts & shopping, Software setup, Phases 1-4, Troubleshooting) are empty "pending" placeholders, checked 2026-10-03. The Setup Guide doc holds all real detail; nothing to copy from them. Overview's claim of "full firmware" per tab is not yet true.
2. Setup Guide Sources list still has stale links (7-DSI-TOUCH-A, RPi Camera B); dropped in this copy.
3. P4 header pins, R4 tone() on D8, shifter and IMU details all verified 2026-10-03 (see `docs/manual/00-overview-and-safety.md`). Left: two bench multimeter checks (header 5V on LiPo, LiPo plug polarity) once the hardware is in hand.
4. Overlap with existing work: `firmware/ttgo-node` (undeployed anti-spy) and the R4 Wi-Fi/BLE scanner in `~/Projects/sentinel/docs/worklog/2026-09-29-r4-wifi-scanner.md`. Decide whether Phase 1/2 reuse that hub.
5. Needs Taelor: order parts; confirm go on Phase 1 firmware.

## Manual
Full manual written 2026-10-03 in `docs/manual/` (pin-to-pin tables, ASCII diagrams, assembly, software, tests). Pin data checked against the Waveshare schematic and docs, Seeed, Arduino core source and Adafruit board files.

## Next step
Write Phase 1 finder firmware for the R4 (needs only buzzer + resistor, both cheap), test with phone hotspot.
