# Hidden Device Hunter V2

## Scope

V2 covers Phase 1 and Phase 2: the UNO R4 WiFi with its Qwiic Modulino Buzzer, and the Seeed Studio XIAO ESP32-C5 dual-band sniffer connected to the R4 through a level shifter.

Phase 3 (P4 camera) and Phase 4 (optional additions) are parked for later research. Their existing files remain preserved as reference, not current build instructions. Expansion requires a separate evidence-based review. The manuals have not passed a complete V2 audit.

## Build guides

| Phase | Guide | Firmware |
|---|---|---|
| 1 | [UNO R4 WiFi + Qwiic Modulino Buzzer](01-phase1.md) | [`firmware/phase1-r4-finder/`](../../firmware/phase1-r4-finder/) |
| 2 | [XIAO ESP32-C5 and level-shifted UART](../manual/02-phase2-xiao-antenna.md) | [`firmware/phase2-xiao-sniffer/`](../../firmware/phase2-xiao-sniffer/), plus [`firmware/phase2-r4-display/`](../../firmware/phase2-r4-display/) |

Shared electrical safety guidance: [`docs/manual/00-overview-and-safety.md`](../manual/00-overview-and-safety.md). The old passive-buzzer Phase 1 instructions are marked archived in [`docs/manual/01-phase1-finder.md`](../manual/01-phase1-finder.md).

## Verified status and limits

- **Phase 1:** firmware was uploaded to the UNO R4 WiFi. Serial checks confirmed Modulino detection, buzzer-test command handling, and live AUTO scan readings. Audible output and field-finding remain unverified. Phase 1 scans 2.4 GHz Wi-Fi.
- **Phase 2:** XIAO ESP32-C5 firmware received 2.4 GHz and 5 GHz Wi-Fi traffic. The XIAO-to-R4 UART, BOOT-button behavior, and lock persistence remain untested. The ordered Rokland dual-band antenna has not been delivered.
- Phase 2 scans selected channels; it does not cover every channel or band, and it can miss devices that are not transmitting. RSSI is not a precise distance measurement.
- For the UART bench test, power the R4 and XIAO separately over USB, connect common ground, and use the level shifter. Never apply R4 5 V logic directly to an ESP32 pin.

These are status notes, not a certification of audible performance, field performance, or the full manual.
