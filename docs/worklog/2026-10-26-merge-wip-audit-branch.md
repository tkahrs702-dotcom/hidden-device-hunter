# 2026-10-26 Merge of wip-2026-10-19-audit-and-c5 into main

Taelor asked to merge the WIP branch. Every changed file conflicted with main, so each was resolved by hand:

- Kept from the WIP branch: the Phase 1 R4 finder audit fixes (buzzer test toggles with `t`, scan errors and zero results reported, a missed locked target goes quiet at once). This is the firmware uploaded to the R4 on 2026-10-19, so main now matches the board. Also kept: the R4 display change, both 2026-10-19 worklogs, and one Phase 3 note.
- Kept from main: README, the manual, `docs/v2/`, the Phase 2 wiring drawing (main's 2026-10-20 V2 pass and the 2026-10-26 README summary are newer), and the Phase 2 sniffer firmware, which is the version flashed to the XIAO ESP32-C5 that received 2.4 and 5 GHz traffic.
- Dropped: `firmware/phase2-c5-sniffer/` (a generic C5 carrier wrapper that needs the WIP sniffer's RX1/TX1 pins) and the old `HANDOFF.md` (folded into the README earlier today).
- Compile checks passed: phase1-r4-finder and phase2-r4-display (`arduino:renesas_uno:unor4wifi`), phase2-xiao-sniffer (`esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc`). Nothing was uploaded.

## WIP sniffer changes not merged (compile-checked on 2026-10-19, never run on hardware)

The branch stays on GitHub (`98c18ad`). To port into main's sniffer, one at a time, with a hardware check after each:
- Report channel-tune and promiscuous-mode setup failures by name.
- Check the packet queue allocation; count dropped frames atomically and report them.
- Generic ESP32-C5 carrier support (RX1/TX1, no BOOT button).
- Design choices to decide first: the WIP build skips the 5 GHz DFS channels (main listens on them, receive-only) and halts if the US regulatory domain or dual-band mode can't be set (main warns and carries on at 2.4 GHz).
