# Hidden Device Hunter (RF Finder)

Handheld finder for hidden Wi-Fi and Bluetooth devices in Taelor's own home. Started in the Claude Android app 2026-10-03.
Status: **Phase 1 uploaded and bench-checked on the UNO R4 WiFi. The Seeed Studio XIAO ESP32-C5 was flashed and received 2.4 GHz and 5 GHz Wi-Fi traffic; its UART link to the R4 is still untested. The Rokland 2.4/5 GHz directional antenna is ordered but not delivered.** Serial confirmed Phase 1 Modulino detection, buzzer-test command handling, and live AUTO scan readings; audible output and field-finding remain unconfirmed.

## Where we left off

- The current project version is **V2: Phase 1 + Phase 2**. Start with [`docs/v2/README.md`](docs/v2/README.md); the current handoff is [`docs/v2/HANDOFF.md`](docs/v2/HANDOFF.md).
- Phase 2 uses a Seeed Studio XIAO ESP32-C5; its flashed firmware received a 5 GHz device on channel 36 and a 2.4 GHz access point on channel 2 (see the 2026-10-18 worklog).
- The Phase 2 drawing uses XIAO C5 D6/D7 (GPIO11/GPIO12), with UART through the level shifter. The current local firmware selects these board-specific pins; generic C5 carrier builds remain separate.
- Next: bench-test the level-shifted XIAO-to-R4 UART with each board powered separately by USB; when delivered, check the Rokland antenna connector and cable before use.
- Open: the UART link, BOOT button behavior, and lock persistence have not been tested with the R4 receiver.

## V2 build docs: Phase 1 + Phase 2

[`docs/v2/README.md`](docs/v2/README.md) is the V2 entry point and links to the existing phase manuals and firmware; it does not replace them.

| File | What |
|---|---|
| [`00-overview-and-safety.md`](docs/manual/00-overview-and-safety.md) | Shared system context and safety rules |
| [`01-phase1-finder.md`](docs/manual/01-phase1-finder.md) | UNO R4 WiFi + Qwiic Modulino Buzzer |
| [`02-phase2-xiao-antenna.md`](docs/manual/02-phase2-xiao-antenna.md) | Seeed XIAO ESP32-C5 dual-band sniffer and R4 UART |
| [`05-reference.md`](docs/manual/05-reference.md) | Existing reference and open verification items |

## Preserved, parked manuals (not V2 build instructions)

Phase 3 and Phase 4 are deferred. Their manuals are preserved as reference material, not current build instructions; any expansion requires a separate evidence-based review.

| File | What |
|---|---|
| [`03-phase3-p4-rf-camera.md`](docs/manual/03-phase3-p4-rf-camera.md) | Parked Phase 3 planning/reference |
| [`04-phase4-options.md`](docs/manual/04-phase4-options.md) | Parked Phase 4 options/reference |

Other docs: [`HANDOFF.md`](HANDOFF.md) (state, decisions, next step), [`docs/build-plan.md`](docs/build-plan.md) and [`docs/setup-guide.md`](docs/setup-guide.md) (the original Android-app docs, kept as source), [`docs/worklog/`](docs/worklog/).

## Cloud originals (live Claude Docs, private to Taelor)
Hidden Device Hunter (only Overview tab has content): https://claude.ai/artifact/6Hofsp6NwQrS2nxhyiZN5e. Build Plan: https://claude.ai/artifact/PuYyKuxAEt5E7tMc9CCF2e. Setup Guide: https://claude.ai/artifact/QAQKwawgR31SS5VzCug1D1.

## Phases
| Phase | Build | Test |
|---|---|---|
| 1 | Uno R4 WiFi + Modulino Buzzer on Qwiic; scans Wi-Fi and maps RSSI to beep rate and LED bars | Uploaded; serial bench check passed, field test pending |
| 2 | Seeed XIAO ESP32-C5 dual-band (2.4/5 GHz) monitor + compatible antenna; UART to R4 through a level shifter | C5 firmware received 2.4/5 GHz traffic; XIAO-to-R4 UART bench test remains |
| 3 | Deferred; see parked manual | Not part of V2 |
| 4 | Deferred; see parked manual | Not part of V2 |

Limits: finds only devices that transmit; Phase 1 is 2.4 GHz only. Phase 2 C5 firmware scans selected 2.4/5 GHz channels. Own home and network only.
Wiring rule: R4 is 5 V logic, every ESP32 is 3.3 V. Never put 5 V on an ESP32 pin.
Relation to SENTINEL: separate project; reuses the P4 and T3 boards and may reuse `~/Projects/sentinel/tools/r4-wifi-scanner`.
