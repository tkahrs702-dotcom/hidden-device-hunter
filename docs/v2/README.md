# Hidden Device Hunter V2

## Scope

V2 is the Phase 1 + Phase 2 project: the UNO R4 WiFi with Qwiic Modulino Buzzer, and the Seeed XIAO ESP32-C5 dual-band sniffer connected to the R4 through a level shifter. This document is an entry point to existing material, not a replacement or an audit of the manuals. The manuals have **not** passed a V2 audit.

Phase 3 (P4 camera) and Phase 4 (optional additions) are outside V2. Their files remain preserved as parked reference, not current build instructions. Revisit them only after a separate evidence-based review.

## Current build references

| Topic | Existing source |
|---|---|
| Shared overview and electrical safety | [`docs/manual/00-overview-and-safety.md`](../manual/00-overview-and-safety.md) |
| Phase 1 wiring and use | [`docs/manual/01-phase1-finder.md`](../manual/01-phase1-finder.md) |
| Phase 1 firmware | [`firmware/phase1-r4-finder/`](../../firmware/phase1-r4-finder/) |
| Phase 2 XIAO C5 wiring and R4 UART | [`docs/manual/02-phase2-xiao-antenna.md`](../manual/02-phase2-xiao-antenna.md) |
| Phase 2 C5 build entry and shared sniffer | [`firmware/phase2-c5-sniffer/`](../../firmware/phase2-c5-sniffer/) and [`firmware/phase2-xiao-sniffer/`](../../firmware/phase2-xiao-sniffer/) |
| Phase 2 R4 receiver | [`firmware/phase2-r4-display/`](../../firmware/phase2-r4-display/) |
| Existing verification notes | [`docs/worklog/2026-10-19-stage2-c5-audit.md`](../worklog/2026-10-19-stage2-c5-audit.md) and [`docs/worklog/2026-10-19-stage1-audit-fixes.md`](../worklog/2026-10-19-stage1-audit-fixes.md) |
| Current detailed state and next step | [`docs/v2/HANDOFF.md`](HANDOFF.md) |
| Project-level handoff | [`HANDOFF.md`](../../HANDOFF.md) |

## Verified status and limits

- **Phase 1:** firmware was uploaded to the UNO R4 WiFi; serial bench checks confirmed Modulino detection, buzzer-test command handling, and live AUTO scan readings. Audible output and field-finding performance are unconfirmed. Phase 1 is 2.4 GHz only.
- **Phase 2:** the XIAO ESP32-C5 firmware received 2.4 GHz and 5 GHz Wi-Fi traffic. The XIAO-to-R4 UART, BOOT behavior, and lock persistence have not been tested. The ordered Rokland dual-band antenna has not been delivered.
- The current C5 profile scans selected channels; it does not cover all bands/channels or detect devices that are not transmitting. RSSI is not a precise distance measurement.
- Keep the R4's 5 V logic off ESP32 pins. Follow the existing Phase 2 manual's separate-USB-power guidance for UART bench tests.

These notes summarize reported bench results; they do not certify the manuals, audible performance, or field performance.
