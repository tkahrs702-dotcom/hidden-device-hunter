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

## Rethink (2026-10-27): where should the dashboard live?
Hardware lessons from today: the R4's Wi-Fi chip is slow (page took long to load, serial lines lost while it sends), the R4 has little RAM (60-device list overflowed it), the R4 to C5 return wire is not guaranteed, and wiring mistakes cost the most time. Building more features on the R4 page keeps hitting those limits.

Options
- A. Keep the R4 Wi-Fi page and add volume and Track on it. Needs the R4 D1 to C5 D7 return wire and still has the RAM and speed limits.
- B. Move the dashboard to the C5 over Bluetooth LE. The C5 already has all the data and 5 GHz hearing, plenty of RAM, and BLE works alongside sniffing. The phone talks only to the C5 (Web Bluetooth in Android Chrome, page hosted on https); the phone can pick any device to track, ask for the detail of any device, and set volume. The C5 forwards the volume to the R4 over the existing one-way link (C5 TX to R4 RX), so no return wire is needed. The R4 goes back to being a simple beeper and bars box (and can keep its Wi-Fi page as an optional extra). Limit: Web Bluetooth does not work on iPhone.
- C. Both: B as the main dashboard, A kept as a fallback.

Recommendation: B (or C). It removes the weakest parts (R4 Wi-Fi, return wire) and gives full control from the phone. Needs Taelor's phone type (Android or iPhone) before building.

## Decision and build status (2026-10-27)
Taelor chose the C5 Bluetooth dashboard (Android). Built: BLE service on the C5 (Nordic UART style UUIDs, device name HDH-Finder), commands K/U/V, 40-device list with detail fields, buzzer volume levels on the R4, and `tools/finder/finder.html`. The return wire is not needed. Open: the page must be opened in a secure context (Web Bluetooth needs https or a local file); try the file from the phone's Downloads in Chrome first, host it on a private https page only if that fails.
