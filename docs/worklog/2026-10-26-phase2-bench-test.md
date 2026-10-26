# 2026-10-26 Phase 2 bench test on HaLjr

- R4 now runs `phase2-r4-display` (3 startup chirps). Uploads first failed with "No device found on ttyACM0": fixed by running Arduino's udev script once, `sudo ~/.arduino15/packages/arduino/hardware/renesas_uno/*/post_install.sh`, then replugging. Phase 1 and Phase 2 both show a scrolling dot on the bottom row, so the dot alone does not prove which sketch is running; the 3 chirps do.
- ttyACM numbers swap between the R4 and the C5 on every replug. Check `ls -l /dev/serial/by-id/` before each command.
- C5 sniffer works: bare board on HaLjr USB listed 41 devices on 2.4 and 5 GHz (ch 36 seen), with IoT (Tuya, Espressif) and CAM (Wyze, Ring) tags.
- Bug found: `arduino-cli monitor` sends `\r` for Enter and the sniffer waited for `\n`, so typed commands never ran. Fixed in PR #9; until the C5 is reflashed, press Ctrl+J instead of Enter.
- The C5 kept a saved lock from an earlier test; it sends `L` to the R4 only once at boot, so an R4 that starts later keeps showing the dot until a new lock or `T` line arrives.
- Power: a small "nano" 5 V power bank switches off under the C5's low current draw. Use the 2-port bank for both boards, or the computer USB on the bench.
- The XIAO's small light is the charge LED; it goes dark with no battery while the board runs. The sniffer does not drive an LED.
- Link confirmed after flashing PR #9 on both boards: with the shifter wires on, tapping B on the C5 makes the R4 show the X (locked, target not heard), so D6 -> shifter -> R4 D0 works. B locks cameras first, and the nearest camera tags were weak, so tap B again to step to a device in range.
