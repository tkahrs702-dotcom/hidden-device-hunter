# V2 handoff

Last updated 2026-10-20.

## Decisions

- The active scope is Phase 1 and Phase 2. Start with [`README.md`](README.md).
- Phase 3 and Phase 4 remain parked reference material. Expansion requires a separate evidence-based research review.
- The existing manuals have not passed a complete V2 audit.

## State

- **Phase 1:** UNO R4 WiFi firmware was uploaded. Serial checks confirmed Modulino detection, buzzer-test command handling, and live AUTO scan output. Audible output and field-finding remain unverified.
- **Phase 2:** Seeed Studio XIAO ESP32-C5 firmware received 5 GHz traffic on channel 36 and 2.4 GHz traffic on channel 2. The UART link to the R4, BOOT behavior, and lock persistence remain untested.
- The Rokland 2.4/5 GHz directional antenna is ordered but has not arrived; verify connector and cable fit before use.
- The Phase 2 USB CDC build option is required for the C5 USB serial console. Follow the Phase 2 guide for the current compile command and wiring.

## Next steps

1. Bench-test the level-shifted XIAO-to-R4 UART with both boards independently USB-powered and their grounds shared. Do not connect the 5 V rails or apply R4 5 V logic to an ESP32 pin.
2. Verify the XIAO BOOT-button lock/unlock behavior and saved-lock recovery.
3. When the ordered antenna arrives, check its connector and cable, then verify reception on both bands.
4. Keep Phase 3/4 parked until a separate evidence-based research review is requested and completed.

No new firmware should be flashed as part of this documentation handoff.
