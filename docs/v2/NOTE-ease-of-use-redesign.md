# Note: the current setup is too fiddly, rethink for ease of use (Taelor, 2026-10-27)

Taelor's verdict after a long session: this is not the way to make it work. Getting it running takes too many steps
(terminal flash lines, BOOT-hold timing, crossed wires, power-bank limits, a slow R4 Wi-Fi page, a Bluetooth page that
has to be downloaded and opened in the right browser). Taelor does not want to tear it apart and move wires, but is open
to a redesign that makes it work out of the box. The message was cut off after "you almost have to put in...".

## What went wrong in practice
- Flashing: terminal commands, BOOT held while plugging in, port names that swap. Taelor has no serial monitor.
- Wiring: R4 to C5 link wires were crossed (worked around with `LINK_CROSSED` in the C5 sketch, not rewired).
- Power: one shared bank sags; the R4 Wi-Fi page is flaky and slow. The Nano 5 V bank shuts off for the C5 alone.
- Phone page: R4 Wi-Fi page loads slowly; the Bluetooth page needs Chrome (Brave blocks Web Bluetooth) and a way to open
  the file. The repo is private, so raw links do not work. A claude.ai artifact copy exists but Bluetooth may be blocked in it.
- Unverified on hardware: BLE phone link, Bluetooth scan, mode switch, volume, Track.

## Questions the next session must answer first
1. Can the C5 alone be the whole device (its own screen or buzzer) so the R4, level shifter and link wires go away?
2. Can the phone page be reached with zero setup (one tap, no download, no browser rules)? Options: host over https
   (GitHub Pages needs the repo public or a paid plan, or another host with Taelor's consent), or a native Android app.
3. Can flashing be done once and never again (OTA update over Wi-Fi, or a one-click web flasher)?

## Rules to keep
Replies of 2-3 plain lines, take Taelor's reports as true, never ask for serial-monitor output, one command at a time,
no placeholders, design first and write it in the repo before building, no merging until Taelor says "merge".
See `HANDOFF-ble-scan-and-mode-toggle.md` for the current technical state.

## Update 2026-10-27 14:01
The claude.ai artifact copy of the finder page loads and Demo works, but Connect fails (Bluetooth is blocked inside that kind of page). Not tried yet: the saved file opened from Downloads in Chrome. The R4 Wi-Fi page (http://192.168.4.1) is the only phone route confirmed to open so far.

## Update 2026-10-27 14:20
The Scan for buttons (Wi-Fi / Bluetooth / Both) are now also on the R4 Wi-Fi page. The R4 sends `M,<n>` to the C5 over the D1 wire (R4 TX to C5 RX), so that direction of the link must be wired. Bluetooth devices appear in the page lists with "BLE" in the ch column.
