# Flash and test: the whole sequence in one place

Everything needed to put the latest code on both boards and test it, on HaLjr (Linux).
Copy each command whole; nothing in them needs editing.

## Quick way (normally all you need)
1. Flash both boards with one command, and follow its prompts:
```
cd ~/Projects/hidden-device-hunter && git checkout main && git pull && bash tools/flash.sh
```
   (`... flash.sh c5` or `... flash.sh r4` does just one board.)
2. Power both boards from the 2-port power bank.
3. The C5 locks the Tuya plug, and the R4 shows bars and beeps.

The sections below are the same steps by hand, for when something goes wrong.

## 0. Before you start
- Computer on your home Wi-Fi (not `HDH-Finder`), so `git pull` works.
- Only one board plugged into the computer at a time.
- Use a data USB cable and push it in firmly (a loose cable caused one failed flash).

## 1. Get the latest code (once per session)
```
cd ~/Projects/hidden-device-hunter && git checkout main && git pull
```
Expected: `Already up to date.` or a list of changed files.
- If it says `error: Your local changes ... would be overwritten`: run
  `cd ~/Projects/hidden-device-hunter && git stash && git checkout main && git pull`
  (your edits are kept in the stash, not deleted).

## 2. Flash the C5 (XIAO ESP32-C5 sniffer)
1. Unplug the R4 from the computer.
2. Hold the C5's BOOT button, plug the C5's USB cable in, then let go of BOOT.
3. Run:
```
cd ~/Projects/hidden-device-hunter && arduino-cli compile --upload -p $(ls /dev/serial/by-id/*Espressif* | head -1) --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer
```
Expected: it compiles for about a minute, then `Writing at ...` lines and `Hard resetting via RTS pin...`.
4. Unplug the C5 and plug it back in (no BOOT) so it starts the new code.

If it fails:
- `ls: cannot access '/dev/serial/by-id/*Espressif*'` followed by `Error: accepts at most 1 arg(s), received 2`:
  the computer cannot see the C5. Unplug it, check the cable, and repeat step 2 (hold BOOT while plugging in).
  The second error only follows from the first; the command itself is fine.
- `Failed to start stub flasher`: put the C5 in download mode (hold BOOT, tap RESET, let go of BOOT), then run:
```
cd ~/Projects/hidden-device-hunter && arduino-cli compile --output-dir /tmp/c5 --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer && ~/.arduino15/packages/esp32/tools/esptool_py/*/esptool --chip esp32c5 -p $(ls /dev/serial/by-id/*Espressif* | head -1) --no-stub write-flash 0x0 /tmp/c5/phase2-xiao-sniffer.ino.merged.bin
```
- The new code did not seem to arrive: you were not on `main`. Run step 1 again, then step 2.

## 3. Flash the R4 (UNO R4 WiFi display)
1. Unplug the C5 from the computer. Plug the R4 in alone.
2. Run:
```
cd ~/Projects/hidden-device-hunter && arduino-cli compile --upload -p $(arduino-cli board list | grep unor4wifi | awk '{print $1}' | head -1) --fqbn arduino:renesas_uno:unor4wifi firmware/phase2-r4-display
```
Expected: compiles in under a minute, then upload progress and `Done`.

If it fails:
- `No device found` or an empty port: double-tap the R4's RESET button (the LED pulses), then run the same command again.
- Still no device (Linux permissions, needed only once per computer):
```
sudo ~/.arduino15/packages/arduino/hardware/renesas_uno/*/post_install.sh
```
  then unplug and replug the R4 and run the flash command again.

## 4. Put it together
1. Wire stays as it is (C5 D6/D7 to the level shifter to R4 D0/D1, grounds joined). Never connect 5 V to a C5 pin.
2. Power both boards from one 2-port power bank (the small Nano bank shuts off with the C5 alone).
3. On boot the C5 locks your Tuya plug (10:5A:17:6C:02:4D) and the R4 bars and beeps start.
   C5 BOOT button: short press = next suspect, hold 1-3 s = unlock, hold 3 s+ = switch Wi-Fi / Bluetooth / both.

## 5. Open the phone page (pick one)
- R4 Wi-Fi page (works with any phone browser): join Wi-Fi `HDH-Finder`, password `hunter2026`,
  open `http://192.168.4.1`.
- Bluetooth page (Chrome on Android, not Brave): open `https://raw.githack.com/tkahrs702-dotcom/hidden-device-hunter/main/tools/finder/finder.html`,
  tap Connect to the finder and pick `Ananizapta` (the C5's Bluetooth name). It is only visible for 2 minutes after power-on, or any BOOT press, and 10 minutes after the phone loses the link. (Android's Bluetooth settings never list it; that is normal.)
  Tap Demo first if you want to see it work without the boards.

## 6. Telemetry tests
While a device is locked and heard, the page shows a line like `5 packets/s (idle 4)   wobble 2 dB: steady`.
- Hand-wave test: hold still about 1-2 ft from a suspect for 5 seconds (so it learns the idle rate),
  then wave your hand in front of it. A camera that sees motion sends more data: the page shows a red
  `WAVE SPIKE` line for 2 seconds. A plug or sensor will not spike.
- Wobble: steady (0-2 dB) = clean signal, usually close; wobbly (3-6 dB); bouncing (over 6 dB) = reflections, move.
- No packets/s line: the target is Bluetooth, the C5 is still searching channels, or the C5 still has old code.
- Save log (Bluetooth page): tap Save log at the end of a sweep; it downloads `hdh-log-<date>.csv`
  with every reading and the time. It keeps about the last hour.
