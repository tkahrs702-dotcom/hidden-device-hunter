# Session note: 10-30-2026, Merlin wiring diagram and firmware for all four boards

## Done
- Pin-to-pin diagram rebuilt from Taelor's Merlin board photo: `docs/manual/diagrams/merlin-four-board-wiring.svg` (generator `tools/diagrams/merlin_wiring.py`). Header labels copied from the silkscreen, a photo strip sits beside the drawing, no crossing wires, onboard speaker instead of a piezo. 5 V vs 3.3 V made obvious after Taelor flagged that the first draft read as "all 5 V".
- Pin numbers: an earlier draft of the diagram dropped them, wrongly guessing the header follows Raspberry Pi order. Waveshare's schematic (connector J3) shows pin 1 = 5V, pin 2 = 3V3, so the old numbers (pin 11 = GPIO5, 14 = GPIO4, 15 = GPIO21, 17 = GPIO22, 18 = 3V3) were right. The diagram and guide use them again; see `docs/v2/merlin-p4-hardware-notes.md`.
- Firmware: `firmware/merlin-p4-screen` (ESP-IDF, screen + speaker + two UART links), `firmware/argus-ttgo-router` (deauth/disassoc watcher, learns the router as the strongest network, LoRa alerts), `firmware/iolaus-ttgo-handheld` (LoRa to Merlin serial). Nimue: `LINK_CROSSED = true` (TX on D7) and BLE name "Ananizapta".
- Step-by-step guide: `docs/merlin-build-guide.md`. CI compiles both TTGO sketches and Merlin (esp-idf-ci-action).
- Separate reviewer: pins, protocol and 5 V safety OK. Fixed: draw buffers in PSRAM, volume tug-of-war, device-list cap, promiscuous before set_channel. Rejected: pulsing GPIO16 for the OLED (T3 V1.6.1 uses 16/17 for PSRAM).

## Open
- Nothing run on hardware yet. Waveshare BSP name/version `waveshare/esp32_p4_wifi6_touch_lcd_4_3 ==1.0.1` unverified: CI will say.
- PR #20 (stale 10-28 draft) to be closed or replaced on Taelor's yes.
