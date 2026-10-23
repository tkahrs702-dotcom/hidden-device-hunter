# V2 handoff

Last updated 2026-10-20.

## Decisions

- V2 covers Phase 1 and Phase 2 only. Use [`README.md`](README.md) as its documentation entry point.
- Existing manuals and firmware are the sources of truth; this handoff links to them rather than copying their instructions.
- Phase 3 and Phase 4 stay preserved but parked. Any future expansion starts with a separate evidence-based review. Their current manuals are not V2 build instructions.
- No claim is made that the manuals have passed an audit.

## State

- **Phase 1:** UNO R4 WiFi firmware was uploaded and serial bench-checked. Serial confirmed Modulino detection, buzzer-test command handling, and live AUTO scan readings. Audible output and field-finding remain unverified; Phase 1 is 2.4 GHz only. See [`2026-10-19-stage1-audit-fixes.md`](../worklog/2026-10-19-stage1-audit-fixes.md).
- **Phase 2:** Seeed Studio XIAO ESP32-C5 firmware was flashed and received 5 GHz traffic on channel 36 and 2.4 GHz traffic on channel 2. See [`2026-10-19-stage2-c5-audit.md`](../worklog/2026-10-19-stage2-c5-audit.md).
- The Rokland 2.4/5 GHz directional antenna is ordered but not delivered.
- The level-shifted XIAO-to-R4 UART, BOOT behavior, and lock persistence remain untested.

## Next steps

1. Bench-test the level-shifted XIAO-to-R4 UART with the two boards powered separately over USB. Do not connect the R4's 5 V logic to an ESP32 pin.
2. When the antenna arrives, check its connector and cable before use.
3. Keep audible/field performance as unverified until directly tested.
4. Consider Phase 3/4 only through a separate, evidence-based research review; no expansion is currently authorized by this V2 scope.
