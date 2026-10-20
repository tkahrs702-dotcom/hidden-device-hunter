# 2026-10-20 Phase 2 XIAO C5 wiring update

- Confirmed GitHub `main` already includes the flashed Seeed Studio XIAO ESP32-C5 firmware and its 2026-10-18 2.4/5 GHz reception test (merged PR #4).
- Replaced the current Phase 2 manual's stale S3 wiring instructions with the Seeed XIAO C5 wiring: D6/GPIO11 TX to R4 D0/RX through shifter channel 2, and optional/reserved R4 D1/TX to XIAO D7/GPIO12 through channel 1.
- Added a wiring diagram and a functional connection schematic. They show separate USB power for the R4 and XIAO, shared ground, R4 5V only to shifter HV, and XIAO 3V3 to LV. The previous generated S3 diagrams are explicitly historical.
- Updated the reference tables, overview, README, and HANDOFF to show that the UART link still needs a bench test. Updated the antenna status: Rokland 2.4/5 GHz directional model ordered but not delivered; connector fit is pending delivery.
- Validated both new SVG files and regenerated the system overview from `tools/diagrams/make_diagrams.py`.
- Compile-only check passed for the GitHub C5 firmware target `esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc` with all warnings enabled. No firmware was uploaded; this PR makes no firmware changes.
