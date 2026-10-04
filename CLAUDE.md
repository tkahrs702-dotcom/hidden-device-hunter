# Hidden Device Hunter (RF Finder)
Handheld finder for hidden Wi-Fi/Bluetooth devices in Taelor's own home. Start with `HANDOFF.md`, then `README.md`.
- Separate project from SENTINEL; it only reuses the P4 and T3 boards and may reuse the R4 scanner in `~/Projects/sentinel/tools/r4-wifi-scanner`.
- ESP32 screens use the `lvgl-ui` skill. Log sessions in `docs/worklog/`; keep HANDOFF current.
- R4 is 5 V logic, ESP32s are 3.3 V: never put 5 V on an ESP32 pin. Own home and network only.
