# Session note — 10-30-2026 — P4 "Merlin" main screen plan

Planning session (no firmware built). The P4 becomes the finder's main screen and the system grows to four named boards. All review was done by separate reviewer agents, never self-review.

## Decisions (confirmed by Taelor)
- **Board names:** Merlin = Waveshare ESP32-P4 screen, Nimue = XIAO ESP32-C5 sniffer, Iolaus = handheld TTGO LoRa32, Argus = router TTGO LoRa32.
- **Bluetooth broadcast name = "Ananizapta"** (a protective charm word), broadcast honestly — no "Mini Speaker" disguise. Still goes quiet after ~2 min or once a phone connects. Board nicknames stay internal to code/docs.
- **Two TTGO LoRa32 boards:** Iolaus on the handheld (UART to Merlin), Argus by the router (deauth/disassoc sentry, finds the router's channel itself at boot and re-checks — router uses auto channel; alerts to Iolaus over 915 MHz LoRa).
- Phone/web page direction dropped; the on-device screen is the way forward. Repo is **public** — no film names ("Necron 99/Wizards") in code; keep Wi-Fi name out of git (local config or learn strongest network).
- Future: an **Android app** will read the same BLE telemetry — keep the line protocol and GATT stable.
- This remains **Phase 2** (README Phase 3 stays the P4 camera). OV5647 camera + 100 Ω resistor arrive ~11-01; camera = Phase 3.

## Wiring (3.3 V everywhere; reviewer-confirmed pins)
Nimue→Merlin: C5 5V→P4 pin1 (5V); C5 GND→pin5; C5 **D7 (TX)**→pin11 (GPIO5 RX); P4 pin14 (GPIO4 TX)→C5 D6 (optional).
Iolaus→Merlin: TTGO 3V3→pin18; GND→pin19; TTGO TX→pin15 (GPIO21 RX); P4 pin17 (GPIO22 TX)→TTGO RX.
Argus: no wires to Merlin — links to Iolaus by 915 MHz radio only.
**Sound: use the board's ONBOARD speaker + audio codec (ES8311 + NS4150B amp), not an external piezo.** (Confirmed from Taelor's board photo: speaker plugged into the onboard 2-pin connector.) Earlier external-piezo idea on GPIO28/pin16 is dropped.
Never put 5 V on any ESP32 signal pin. Power Nimue from pin1 only with its USB unplugged.

## Status / open
- PR #20 (branch `claude/project-thread-faq9ao`) still holds the **stale 10-28 single-screen draft** — not the plan above. Needs replacing.
- The 4-board pin-to-pin diagram draft is in project scratch (`/mnt/project-files/hdh/four-board-wiring.png/.svg`); needs the redraw below before committing.

## Next thread's first tasks
1. Redraw the 4-board pin-to-pin diagram: remove the external piezo (onboard speaker instead), reroute wires so none cross the pin labels, note the onboard speaker and the unused C6-UART header (TX/RX/IO9/GND). Verify every used pin against the real board silkscreen. Separate reviewer, then show Taelor.
2. Build the Merlin screen: Scorched Earth palette (#0C0E0B bg, #D32F2F red, #9E9D24 olive, #4A5D4E border, #E0E2DC text), big 48pt dBm, real segmented bar, portrait 480x800 flex layout, only the fields the C5 sends (L/U/T/S/M/B/D). ESP-IDF 5.5.5, LVGL 9.5, BSP pinned ==1.0.1, fonts incl. 18. Keep P4 Wi-Fi off.
3. Argus + Iolaus firmware (fresh, never copy SENTINEL): router sentry (Wi-Fi start, deauth+disassoc, BSSID filter, time window, auto channel) → LoRa → handheld. Same LoRa settings both ends.
4. Separate reviewer on each piece, then replace PR #20's contents.
