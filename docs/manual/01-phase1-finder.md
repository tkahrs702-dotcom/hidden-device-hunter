# 01. Phase 1: Wi-Fi finder with Modulino Buzzer (UNO R4 WiFi)

Phase 1 scans nearby Wi-Fi access points, lets you lock onto one by its scan-list number, and maps its reported RSSI to buzzer timing and LED-matrix bars. On 2026-10-19, the updated firmware was uploaded to the connected UNO R4 WiFi. Serial output confirmed Modulino detection, buzzer-test on/off command handling, and AUTO scan readings. This bench check does not independently confirm audible sound or field-finding performance.

## Hardware and wiring

| Part | Connection |
|---|---|
| Arduino UNO R4 WiFi | USB-C data cable for programming/serial, or USB power bank |
| Arduino Modulino Buzzer (ABX00108) | Connect to the board's Qwiic socket |

The firmware uses the Modulino Buzzer over the R4's Qwiic I2C bus (`Wire1`). It does **not** use a passive buzzer on D8, an external resistor, or the D8 `tone()` test sketch. Do not wire the Qwiic buzzer to D8 or 5 V; follow the Modulino's connector and board documentation.

## Firmware behavior

- At startup, probes the Modulino Buzzer and reports if it is not detected. The beeper task retries detection.
- Scans Wi-Fi and lists up to 10 named networks reported by the UNO R4 WiFi's WiFiS3 library.
- With no saved target, follows the strongest network in each scan. Enter a displayed number followed by newline to lock to that network by BSSID; the selection is saved in EEPROM.
- A stronger RSSI increases the beep rate and the number of lit matrix bars. A locked target that disappears is silenced on the first missed scan, shows a waiting indicator during the confirmation window, then changes to a slow lost-target click after three misses.
- The board's scan results are not a measure of exact distance. Wi-Fi multipath, obstructions, and antenna orientation affect RSSI.

## Standalone field use (no computer or keyboard)

Power the UNO R4 WiFi from a USB power bank. The firmware runs on the board; USB serial is not required:

- If a target was saved earlier, the finder starts locked to that network.
- If there is no saved target, it starts in AUTO mode and follows the strongest named network it detects.
- Use the buzzer and LED matrix as the field feedback. The `t` buzzer-test command is only an optional serial bench test and is not needed to operate the finder.

With the current hardware there are no buttons or phone-control interface, so choosing or clearing a target manually requires a USB serial connection before field use. The saved target remains in EEPROM after unplugging.

## Serial commands

These optional setup and diagnostic commands require a computer connected over USB. Open the serial monitor at 115200 baud with newline line endings.

| Command | Action |
|---|---|
| `<number>` | Lock onto that network from the latest list |
| `l` | Print the latest network list |
| `c` | Clear the saved target; does not stop an active buzzer test |
| `m` | Mute or unmute the buzzer |
| `t` | Toggle the 100 ms beep / 500 ms interval buzzer test on or off |
| `h` or `?` | Show help |

The serial port may reset the board when opened. The `t` test reports whether the buzzer was detected when the command was received; a test tone by itself is not proof that Wi-Fi scanning works.

## Known scan limits and diagnostics

- The installed WiFiS3 library returns at most 10 named access points per scan and skips blank/hidden SSIDs.
- A scan returning zero results does not prove there are no access points: WiFiS3 can also return zero when its lower-level scan command fails. The library API used here does not expose enough information to distinguish those cases.
- The firmware reports a negative scan result as an error and reports the zero-result ambiguity once until a non-empty scan occurs.
- Startup checks whether the Wi-Fi module responds and prints the module firmware version. A detected module and successful firmware compile do not verify scan performance.

## Build and verification

Compile for **Arduino UNO R4 WiFi**:

```sh
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all firmware/phase1-r4-finder
```

The source compiled successfully with warnings enabled and was uploaded to `/dev/ttyACM0`. Live serial output showed the updated `t` on/off responses and AUTO readings for detected networks. Audible sound and finding a selected hotspot while walking remain unverified.
