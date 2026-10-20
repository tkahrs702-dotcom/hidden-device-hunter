# V2 Phase 1: UNO R4 WiFi + Qwiic Modulino Buzzer

Phase 1 scans nearby Wi-Fi access points. With no saved target it follows the strongest network; a serial command can lock onto a listed network. RSSI drives the R4 LED matrix and the Qwiic Modulino Buzzer.

## Hardware

- Arduino UNO R4 WiFi
- Arduino Modulino Buzzer (ABX00108), connected to the R4's Qwiic socket
- USB-C data cable for programming and serial, or a USB power bank for standalone operation

The current firmware uses the Modulino over the Qwiic I2C bus. It does not use a passive buzzer, D8, a series resistor, or a breadboard. Do not wire the Qwiic buzzer to D8 or directly to 5 V.

## Build and upload

Install the UNO R4 board core and Modulino library as described in the [shared setup and safety guide](../manual/00-overview-and-safety.md). Connect the R4 over USB-C, identify its port with `arduino-cli board list`, then compile and upload:

```sh
cd ~/Projects/hidden-device-hunter
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all firmware/phase1-r4-finder
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi -p /dev/ttyACM0 firmware/phase1-r4-finder
```

Replace `/dev/ttyACM0` with the port identified for this R4. These instructions do not upload firmware unless you explicitly run the upload command.

## Serial use

Open the USB serial monitor at 115200 baud with newline line endings.

| Input | Action |
|---|---|
| `<number>` | Lock to that network from the latest scan list |
| `l` | Print the latest network list |
| `c` | Clear the saved target |
| `m` | Mute or unmute the buzzer |
| `t` | Toggle the optional buzzer test |
| `h` or `?` | Show help |

After programming, AUTO mode and saved-target operation do not require a computer. If a target is saved, the finder starts locked to it; otherwise, it follows the strongest network heard.

## Limits and verification

The R4 scan is 2.4 GHz only, returns up to 10 named networks, and can omit hidden SSIDs. RSSI varies with obstructions, reflections, and antenna orientation; it is not a distance measurement.

The firmware was uploaded and serial output confirmed Modulino detection, buzzer-test command handling, and live AUTO scan readings. Audible output and field-finding performance remain unverified. Compile or serial checks do not prove the buzzer is audible or that the finder works in the field.

The older passive-buzzer wiring guide is preserved but archived: [`docs/manual/01-phase1-finder.md`](../manual/01-phase1-finder.md).
