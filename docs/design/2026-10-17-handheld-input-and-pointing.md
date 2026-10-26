# Handheld input and pointing: how to pick a target and find it with no computer

Date: 2026-10-17. Design only. No firmware written, nothing flashed.

> **Superseded 2026-10-26.** Taelor decided this project stays fully separate from SENTINEL, so option C (borrowing SENTINEL's receiver TTGO) and the build order below are dropped. A handheld screen and button need a board that belongs to this project. The rest is kept as the record of what was considered.

## The short answer

Right now there is no way to pick a target on the finder itself. The only input is typing a number into a computer. The fix is to give it one button and a small screen. You already own a board with both: a TTGO LoRa32. The R4's own Wi-Fi radio is too weak to be a good finder anyway.

You don't really "point" this finder. One normal antenna hears every direction the same, so it can only tell you "warmer" or "colder" as you walk. You can get a rough direction for free by using your body as a shield (see "Direction" below). A real pointer needs a directional antenna, which is a later purchase.

## Recommendation

Make a TTGO LoRa32 the handheld finder.
- Its Wi-Fi heard 18 networks where the R4 heard 1-2. Your router read -37 dBm on the TTGO and -68 dBm on the R4 in the same spot (measured 2026-10-17).
- It has a 0.96" OLED screen and a BOOT button on GPIO0 (SENTINEL pin notes: OLED on 21/22, button on 0).
- Use the SENTINEL receiver TTGO. Its notes say it already drives a buzzer on GPIO4, so no wiring is needed.
- Trade-off: both TTGOs run the SENTINEL radar link. While one is a finder, the radar alarm is off. Back up its firmware first (SENTINEL rule) and flash it back afterward. If that is not acceptable, buy a third board instead (see "Buy only if needed").

**Smallest next step:** write one TTGO finder sketch (one button, OLED, beeps), back up the receiver TTGO, flash it, and walk toward your phone hotspot. Success is: pick the hotspot with the button, then hear the beeps speed up as you get close.

## 1. Choosing what to hunt, with no computer

One button does everything:
- **Short press:** go to the next network in the list. The screen shows its name, signal and channel.
- **Long press (hold about 1 second):** lock onto the network on screen. Two quick beeps confirm. The pick is saved, so it survives unplugging.
- **Long press while locked:** unlock and go back to AUTO.
- **AUTO** (nothing locked): follow the strongest network heard. This is the default when it powers up with nothing saved.

The list is sorted strongest first and refreshed each scan, so the network you are standing nearest is usually first.

## 2. Showing warmer, colder and "right here"

- **Beeps:** faster as the signal gets stronger, a steady tone when very close. This is what Phase 1 already does.
- **Screen:** a big number (0-100 %), a bar, and a "best so far" mark. Walk until the number stops going up.
- **New best:** a short high chirp each time you beat your best reading. This tells you "warmer" without having to judge beep speed.
- **"Right here":** steady tone plus "HERE" on screen at the top of the range.
- **Lost:** one quiet click every 2 seconds and "LOST" on screen.
- **Faster updates once locked:** scan only the target's channel. A full scan takes a few seconds; one channel is much quicker. (The ESP32 Arduino scan call has a channel option. Check this before building.)
- **Per-board range:** the "near" and "far" levels must suit each board. The current -35 dBm "near" level is out of reach for the weak R4.

## 3. Direction, by hardware option

- **Any board with its built-in antenna (R4, TTGO):** no direction by itself. Two free tricks:
  1. *Walk and compare.* Take 5-10 steps, see if the number rose or fell, turn and try again.
  2. *Body shield.* Hold the finder flat against your chest and turn slowly in a full circle. The reading drops when your body is between the finder and the source. The device is roughly behind you at the lowest reading. Turn slowly, since each reading takes a moment.
- **XIAO ESP32-S3 with the Alfa panel antenna (Phase 2):** real pointing. The panel hears best straight ahead. Sweep it slowly and the strongest reading points at the source.
- **Close in (last few feet):** take the antenna off or shield it, so only very close sources show. This is a later option, not part of the first build.

## Comparison

| Option | Pick a target without a computer | Hears far enough? | Direction | New parts | Downside |
|---|---|---|---|---|---|
| A. R4 + AUTO mode (branch `modulino-buzzer`, not flashed) | No. Follows the strongest network only | Poor. Only networks within a few feet to a room | Hot/cold and body shield | None | Can't choose. Often follows your router, not the target. Max 10 networks, skips hidden names |
| B. R4 + Modulino Buttons or Knob on Qwiic | Yes, but the only screen is the 12x8 LED grid, so names would scroll | Poor (same weak radio) | Hot/cold and body shield | One Modulino (you don't own one; not priced) | Spends money on the weak radio |
| C. TTGO LoRa32 (OLED + BOOT button) **(recommended)** | Yes. One button, names on the OLED | Good. About 30 dB better than the R4 | Hot/cold and body shield | None | Radar link is off while the TTGO is borrowed |
| D. XIAO ESP32-S3 + external or directional antenna | Yes, once it has a button and a screen | Best, with the external antenna | Real pointing with the panel | Shopping list, $27.40 | Not bought. Needs a screen and button plan (or an R4 display link, per Phase 2) |

About option A: it would have fixed tonight's "no beeps on the power bank". The R4 did hear your hotspot at -40 to -49 dBm up close. But it still can't choose a target. You asked to stop before it was flashed, so it stays your call. It is a fair fallback for using the R4 tonight with no new code.

## Build order (for the implementer)

1. Back up the SENTINEL receiver TTGO's flash. Record how to put it back.
2. Write a TTGO finder sketch, reusing the Phase 1 logic: scan, smoothing, strength %, beep timing, saved target.
3. Add the screens (list, locked, LOST, HERE) on the OLED. Add button handling: short press, long press, debounce.
4. Add the "new best" chirp and the single-channel scan when locked.
5. Bench test with the phone hotspot, then walk test. Record results in `docs/worklog/`.
6. Flash SENTINEL back to the TTGO. Update HANDOFF.

Use the TTGO board profile `TTGO LoRa32-OLED, V2.1 (1.6.1)`. Keep off the pins on the SENTINEL "never use" list (6-8, 11, 12, 16, 17).

## Buy only if needed

- **You want the radar left running:** a third ESP32 board with a screen and button. Choose one later; not priced here.
- **You want real pointing:** the Phase 2 list (XIAO ESP32-S3, Alfa APA-M04 panel, U.FL to RP-SMA pigtail, level shifter), $27.40 checked 2026-10-04. Skip the level shifter if the XIAO runs on its own with its own screen.
- **Don't buy Modulino Buttons or a Knob** for the R4. They only add input to a radio that can't hear far enough.

## Sources

Project files (measured or checked earlier):
- `docs/worklog/2026-10-17-wifi-scan-troubleshooting.md`: R4 receive weakness; hotspot at -40 to -49 dBm on the R4.
- Session measurement 2026-10-17 on HaLjr, given in the task: R4 1-2 networks / router -68 dBm; TTGO 18 / -37 dBm; PC 12.
- `firmware/phase1-r4-finder/phase1-r4-finder.ino`: Serial-only input, beep timing, AUTO code, 10-network and hidden-name limits.
- `~/Projects/sentinel/docs/hardware/ttgo-t3-v1.6.1.md`: TTGO pins (OLED 21/22, button 0, buzzer 4 on the receiver, never-use pins).
- `~/Projects/sentinel/firmware/arduino-rd03d/README.md`: board profile V2.1 (1.6.1), Adafruit SSD1306 OLED library.
- `docs/shopping-list.txt`: Phase 2 parts and prices.

Vendor pages. These could not be opened this session (no web access), so treat the details as unverified:
- Modulino Buttons (3 buttons with LEDs, Qwiic): https://docs.arduino.cc/hardware/modulino-buttons/
- Modulino Knob (rotary encoder with push, Qwiic): https://docs.arduino.cc/hardware/modulino-knob/
- Modulino Pixels (8 RGB LEDs, Qwiic): https://docs.arduino.cc/hardware/modulino-pixels/
- LilyGO TTGO LoRa32 boards: https://github.com/Xinyuan-LilyGO/TTGO-LoRa-Series
- ESP32 Arduino Wi-Fi scan (channel option): https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html
