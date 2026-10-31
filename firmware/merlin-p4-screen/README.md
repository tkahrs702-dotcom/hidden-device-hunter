# Merlin: the finder's main screen

Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3, ESP-IDF 5.5 + Waveshare BSP + LVGL 9. Shows what Nimue (the C5 sniffer) and Iolaus (the handheld LoRa board) send it: the locked device with a big dBm number and a segmented bar, the device list, and a router-guard box that turns red when Argus reports a deauth/disassoc attack. Beeps through the onboard speaker, faster as you get closer; a siren on an attack.

Status: written 10-30-2026, not yet run on the hardware. CI compiles it on every push.

Wiring, flashing and checks: [`docs/merlin-build-guide.md`](../../docs/merlin-build-guide.md). Diagram: [`docs/manual/diagrams/merlin-four-board-wiring.svg`](../../docs/manual/diagrams/merlin-four-board-wiring.svg).

Settings at the top of `main/main.c`: the link pins and the signal scale. The panel is natively portrait (480 x 800), so nothing is rotated.
