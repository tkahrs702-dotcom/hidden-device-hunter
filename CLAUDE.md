# Hidden Device Hunter (RF Finder)
Handheld finder for hidden Wi-Fi/Bluetooth devices in Taelor's own home. Start with the README's "Where we left off", then `docs/v2/README.md`.
- Fully separate from SENTINEL (Taelor, 2026-10-26): never use its boards, code, docs or tools. This project uses only its own hardware.
- ESP32 screens use the `lvgl-ui` skill. Log sessions in `docs/worklog/`; keep the README's "Where we left off" current.
- R4 is 5 V logic, ESP32s are 3.3 V: never put 5 V on an ESP32 pin. Own home and network only.
