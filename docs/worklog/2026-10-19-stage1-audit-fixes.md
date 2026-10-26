# 2026-10-19 Stage 1 audit fixes

- Audited Phase 1 source against the installed UNO R4 WiFi, WiFiS3, Modulino, and FreeRTOS libraries.
- Confirmed `arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all firmware/phase1-r4-finder` succeeds.
- Changed `t` to toggle buzzer test mode, so it stops without clearing the saved target. The test mode now takes precedence over automatic signal/lost-target beeps.
- Zero Wi-Fi results are reported as ambiguous because the installed WiFiS3 scan API also returns zero when its lower-level scan command fails. Negative scan results are reported explicitly.
- A missed locked target is silenced on the first missed scan instead of continuing to signal stale RSSI; the waiting indicator is shown until the three-scan lost-target indication.
- Replaced obsolete passive-buzzer/D8 Phase 1 wiring and “firmware not written” documentation with the current Modulino Qwiic design and unverified hardware status.
- Uploaded the corrected firmware to the UNO R4 WiFi at `/dev/ttyACM0`; `arduino-cli upload` completed successfully.
- Before upload, serial returned the old firmware's `Buzzer found. Test beeps playing; type c to stop.` After upload, the new firmware returned `Buzzer test on; type t again to stop.` and `Buzzer test off.` The stop confirmation arrived after the blocking scan cycle.
- Post-upload serial also showed AUTO scan output for `CenturyLink8864` and `Man In The Middle`, including RSSI readings. This confirms the sketch's scan/report path produced results during the bench check, not that RSSI accurately measures distance or that field finding works.
- The test mode was toggled off. Serial/program output cannot independently establish that the buzzer was acoustically audible or that the target-finding field test passed.
- USB serial was used and the corrected firmware was flashed; no other source changes were made beyond the audit fixes.
