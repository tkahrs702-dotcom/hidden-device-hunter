# Plan: volume control and per-device check (asked 2026-10-27)

Taelor asked for (1) volume control on the Arduino and (2) a way to check each device the C5 found. Written to the repo first so it survives if a session runs out of tokens. Status: planned, not built. Build after PR #16 (phone page link fix) is confirmed working on hardware.

## 1. Volume control
- The Modulino Buzzer only takes a frequency and a length (`tone(freq, ms)`); it has no volume setting. Its piezo is loudest near 4 kHz, so volume is faked by lowering the frequency.
- Levels: 3 = loud (4000 Hz), 2 = medium (2500 Hz), 1 = quiet (1500 Hz), 0 = mute.
- Where: buttons on the phone page (`/v?l=0..3`); the R4 keeps the level in `Beeper` (`Beeper::setVolume`) and shows it on the page. No new wiring. Default 3. Resets on power-up (EEPROM later if wanted).
- Files: `firmware/phase2-r4-display/beeper.h` (frequency by level), `webview.h` (route and buttons), `phase2-r4-display.ino` (nothing else).

## 2. Check each device found
- The phone page already lists up to 60 devices. Add a tap on a row to open a detail card: MAC, maker, network name, type (access point, client, hidden), channel, average and latest dBm, packets, seconds since last heard, and whether it is a camera or IoT match.
- Needs two more fields from the C5 in each `D` line: packet count and seconds since heard (the line stays under 110 characters).
- "Track this" button on the card: the R4 sends `K,<MAC>` to the C5 over the return line (R4 D1 TX through the level shifter to C5 D7 RX); the C5 calls `lockOn()`. Without that wire the card still works; only the button does nothing. Taelor must confirm the return wire first.
- Files: `phase2-xiao-sniffer.ino` (extra D fields, read `K` lines from `Serial1`), `webview.h` (card and Track button), `phase2-r4-display.ino`.

## Limits and risks
- The R4 has little RAM (it already overflowed once at 60 devices); keep per-device data small.
- Hidden devices that only probe use random MACs and cannot be tracked by MAC.
- Not testable here; CI only compiles. Each step needs a hardware check by Taelor.

## Order
1. Confirm PR #16 works on hardware (dot top-right, page shows data).
2. Volume (small, R4 only).
3. Device detail card (read only).
4. Track button after the return wire is confirmed.
