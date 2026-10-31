# Session note: 10-30-2026 (into 10-31), build and flash the handheld

## Done
- Flashed all four boards on HaLjr from the PR #23 branch (`claude/project-thread-3lhkkx`); each upload ended "verified" and "Hard resetting": Argus, Iolaus, Nimue (direct `arduino-cli`, not `tools/flash.sh`), Merlin (`idf.py build flash monitor`, ESP-IDF 5.5.5). Screen and OLED results were not reported.
- Meter beep checklist written: shared folder `hdh/meter-check-10-31.md`.
- The C5 (Nimue) died during wiring. A replacement arrives 10-31. Step-by-step for it: shared folder `hdh/new-c5-nimue-steps.md` (flash alone, solder by printed labels, beep test, power-up).
- Diagram fixed in the shared folder (`hdh/wiring-10-31-2026.png/.svg`): the C5 power pin is labelled **VBUS** as printed on the board (it said "5V"); added a line for Taelor's real wire colours. The repo copy on PR #23 still says "5V" and needs the same fix.

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
- Step 2's check says "Argus OK" with dBm; Iolaus's OLED really shows "IOLAUS LoRa ear" and a packets count.
- Wire list W1 says "Nimue 5V"; should be "Nimue VBUS".

## Next
New C5: follow `hdh/new-c5-nimue-steps.md`. Then first power-up of the whole handheld.
