# Session note: 10-30-2026 (into 10-31), build and flash the handheld

## Done
- Flashed all four boards on HaLjr from the PR #23 branch (`claude/project-thread-3lhkkx`); each upload ended "verified" and "Hard resetting": Argus, Iolaus, Nimue (direct `arduino-cli`, not `tools/flash.sh`), Merlin (`idf.py build flash monitor`, ESP-IDF 5.5.5).
- Meter beep checklist written: shared folder `hdh/meter-check-10-31.md`.
- The C5 (Nimue) died during wiring. A replacement arrives 10-31. Step-by-step for it: shared folder `hdh/new-c5-nimue-steps.md` (flash alone, solder by printed labels, beep test, power-up).
- Diagram fixed in the shared folder (`hdh/wiring-10-31-2026.png/.svg`): the C5 power pin is labelled **VBUS** as printed on the board (it said "5V"); added a line for Taelor's real wire colours. The repo copy on PR #23 still says "5V" and needs the same fix.

## Bench results (10-31, after the C5 died)
- Prototype secured. Merlin and Iolaus run without the C5; Nimue reads "NOT HEARD" as expected.
- LoRa tested by Taelor: Argus (by the router) reaches Iolaus.
- Screen glitching and hard resets: Merlin's USB boot log was clean (chip v1.3, PSRAM, screen, touch). Screen was steady with Iolaus on power wires only, so the cause was the loose Iolaus data-wire plugs; fixed with hot glue.
- Iolaus showed "NOT HEARD" on Merlin: Iolaus pin 13 had been wired to the wrong Merlin pin. Taelor moved it to Merlin "21" (pin 15) and Argus to Iolaus to Merlin now works.
- A 6 dBi antenna is on Iolaus. Taelor is to check its label says 915 MHz (or 868/915); a 2.4 GHz antenna must not be used on LoRa.
- Boot-log warning "ledc: GPIO 26 is not usable" (backlight control): harmless so far, a fix for later.
- The firmware's Iolaus screen check in the build guide is right: Iolaus shows "Argus OK N dBm" after a packet.

## Wiring facts learned
- C5 back silkscreen, power edge from the USB end: VBUS, GND, 3V3, D10, D9, D8, D7. VBUS is the 5 V input; Seeed lists 3V3 as output only, so Nimue is powered VBUS to Merlin 5V.
- Merlin pins 1 and 3 are both 5V (same rail); pins 2 and 18 are 3V3.
- Taelor's wire colours on Iolaus and Nimue: red = GND, teal/green = power.
- The speaker is magnetic: mind placement on the prototype.
- The 100 ohm resistor was for the dropped external piezo (old W9, GPIO28); not used.

## Failure log (Taelor asked for this to be recorded)
- The diagram labelled the C5's VBUS pin "5V", which did not match the board Taelor photographed.
- Taelor uploaded several reference photos (LoRa, C5, overview) for a pin-to-pin check. Claude did not identify the pins and parts from them, then asked questions the photos should have answered (VBUS vs 3V3, which Merlin pin) and mixed up which C5 pin the power wire was on.
- Flash commands were first split across blocks and used a `$PORT` variable that came out empty; Claude also described Taelor's terminal without seeing the output.
- Lesson: when photos arrive, state per wire exactly what can and cannot be read before asking; every label must match the silkscreen; one self-contained command block per step with the port written out.

## Guide errors still open (`docs/merlin-build-guide.md`, PR #23)
- Step 3 says `bash tools/flash.sh c5`, which checks out main and flashes the old Nimue code. Use the direct command.
- Wire list W1 says "Nimue 5V"; should be "Nimue VBUS".

## Next
New C5: follow `hdh/new-c5-nimue-steps.md`. Then first power-up of the whole handheld.
