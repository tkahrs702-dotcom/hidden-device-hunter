# Merlin build guide: flash and wire all four boards

Written 10-30-2026. Do the steps in order, one board at a time. Nothing here has run on the hardware yet, so each step ends with a check: stop at the first one that fails and report what you saw.

| Name | Board | Job | Firmware folder |
|---|---|---|---|
| Merlin | Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3 | main screen and speaker | `firmware/merlin-p4-screen` (ESP-IDF) |
| Nimue | XIAO ESP32-C5 | Wi-Fi/Bluetooth sniffer | `firmware/phase2-xiao-sniffer` (Arduino) |
| Iolaus | TTGO LoRa32 (T3 V1.6.1), in the handheld | hears Argus, tells Merlin | `firmware/iolaus-ttgo-handheld` (Arduino) |
| Argus | TTGO LoRa32 (T3 V1.6.1), by the router | watches for attacks on the router | `firmware/argus-ttgo-router` (Arduino) |

Wiring diagram: [`manual/diagrams/merlin-four-board-wiring.svg`](manual/diagrams/merlin-four-board-wiring.svg).

**Safety:** every signal wire is 3.3 V. Only one wire carries 5 V (W1, Merlin 5V to Nimue's 5V power pin). Iolaus is powered from Merlin's 3V3, never 5 V. Flash every board **before** wiring it, with nothing else connected.

## 0. Get the code (HaLjr)

```
cd ~/Projects/hidden-device-hunter
git checkout main && git pull
```
(Until the pull request is merged, use `git fetch origin && git checkout claude/project-thread-3lhkkx` instead.)

## 1. Argus (router TTGO)

1. Check the sticker or silkscreen on the TTGO says T3 V1.6.1 (or "LoRa32 V2.1"). If it says something else, stop and tell Claude: the LoRa pins differ.
2. Screw the 915 MHz antenna on **before** powering. A LoRa radio sending with no antenna can damage itself.
3. Plug only Argus into HaLjr by USB, then:
   ```
   arduino-cli lib install "LoRa@0.8.0" "Adafruit SSD1306@2.5.13"
   PORT=$(ls /dev/serial/by-id/* | head -1); echo $PORT
   arduino-cli compile --upload -p "$PORT" --fqbn esp32:esp32:ttgo-lora32:Revision=TTGO_LoRa32_v21new firmware/argus-ttgo-router
   ```
4. Put Argus next to the router on any USB charger.

**Check:** the OLED says "Looking for the router...", then "Watching:" with your router's name and channel. If it shows a neighbour's network instead, move Argus right next to the router and hold its BOOT button for 3 seconds to learn again.

## 2. Iolaus (handheld TTGO)

1. Antenna on first, as above.
2. Plug only Iolaus in by USB, then:
   ```
   PORT=$(ls /dev/serial/by-id/* | head -1); echo $PORT
   arduino-cli compile --upload -p "$PORT" --fqbn esp32:esp32:ttgo-lora32:Revision=TTGO_LoRa32_v21new firmware/iolaus-ttgo-handheld
   ```
3. Leave it on USB for the check, with Argus powered by the router.

**Check:** within 30 seconds the OLED says "Argus OK" with a dBm number, and the packet count goes up every 30 s. Then unplug Iolaus's USB.

## 3. Nimue (XIAO ESP32-C5)

This firmware now sends on D7 and listens on D6 (`LINK_CROSSED = true`), to match the Merlin wiring, and its Bluetooth name is "Ananizapta".

```
bash tools/flash.sh c5
```
(Hold BOOT while plugging it in, as the script says.) **Check:** the script ends with "upload done" or similar and no red error. Unplug Nimue.

## 4. Merlin (ESP32-P4 screen)

HaLjr already has ESP-IDF 5.5.5 at `~/esp/esp-idf-v5.5.5` (`. ~/esp/esp-idf-v5.5.5/export.sh`). If it is missing, install it first:
```
mkdir -p ~/esp && cd ~/esp
git clone -b v5.5.5 --recursive https://github.com/espressif/esp-idf.git esp-idf-v5.5.5
cd esp-idf-v5.5.5 && ./install.sh esp32p4
```
**Use this project's own P4, not the SENTINEL one.** Flashing replaces everything on the board.

Plug a USB-C cable into the port marked **USB TO UART**, nothing else connected. Its name changes (`/dev/ttyACM0`, `/dev/ttyACM1`, ...), so find it each time:
```
. ~/esp/esp-idf-v5.5.5/export.sh
ls /dev/ttyACM*
```
The chip is known to be revision **v1.3** (Taelor, 10-30-2026: this is the same physical board that ran the earlier working firmware, which now needs updating), and the firmware is set up for that, so the revision check is skipped (Taelor, 10-30-2026). Optional: `python -m esptool -p /dev/ttyACM0 chip_id` shows the revision; if it ever says v3.x, stop and tell Claude, because `sdkconfig.defaults` would have to change.

**Backup (optional):** Taelor says the board's earlier firmware is already backed up (10-30-2026), so this step can be skipped. If you want a fresh copy anyway (about 2 minutes, keep the file):
```
mkdir -p ~/backups/merlin
python -m esptool -p /dev/ttyACM0 -b 921600 read_flash 0 0x2000000 ~/backups/merlin/merlin-original.bin
```
To restore it later: `python -m esptool -p /dev/ttyACM0 -b 921600 write_flash 0 ~/backups/merlin/merlin-original.bin` (esptool 4 spells it `read_flash`/`write_flash`, with underscores).

Build and flash:
```
cd ~/Projects/hidden-device-hunter/firmware/merlin-p4-screen
idf.py set-target esp32p4
idf.py -p /dev/ttyACM0 build flash monitor
```
(Leave the monitor with Ctrl+].)

**Check 2:** the screen lights up upright (tall, not sideways) with dark green-black panels: "Nimue sniffer: NOT HEARD", "Iolaus LoRa: NOT HEARD", "Router guard: Argus not heard yet". If the screen stays black or the board restarts over and over, stop and tell Claude with what the monitor prints. If the monitor says "running silent", the speaker is not set up: tell Claude.

## 5. Meter check 1 (Merlin alone, on USB)

Meter on DC volts, black probe on a Merlin GND pin (5, 13 or 19). Pin 1 is the end of the header marked 5V on the board; odd pins are the row with the 5V labels (the lower row in your photo).
- Pins 1 and 3 read about 5.0 V.
- Pins 2 and 18 read about 3.3 V.
- Pins 11, 14, 15 and 17 (GPIO5, 4, 21, 22) read about 0 V.

Pin numbers come from Waveshare's schematic (connector J3), see `docs/v2/merlin-p4-hardware-notes.md`. If any reading is off, stop and tell Claude. Now unplug Merlin's USB.

## 6. Wire it (everything unplugged)

Follow the diagram, W1 to W8, one wire at a time:

| Wire | From | To |
|---|---|---|
| W1 | Nimue 5V | Merlin 5V (pin 1) |
| W2 | Nimue GND | Merlin GND (pin 5) |
| W3 | Nimue D7 (TX) | Merlin GPIO5 (pin 11) |
| W4 (optional) | Merlin GPIO4 (pin 14) | Nimue D6 (RX): lets the screen's mode buttons work |
| W5 | Iolaus GND | Merlin GND (pin 13) |
| W6 | Iolaus GPIO13 (TX) | Merlin GPIO21 (pin 15) |
| W7 | Merlin GPIO22 (pin 17) | Iolaus GPIO34 (RX) |
| W8 | Iolaus 3V3 | Merlin 3V3 (pin 18) |

## 7. Meter check 2 (still unplugged, meter on beep)

- Each wire beeps end to end.
- Merlin 5V to Merlin GPIO5, GPIO21, GPIO22 and 3V3: **no** beep. If any beeps, stop and find the short.

## 8. First power-up

Nimue's and Iolaus's own USB cables and Iolaus's battery stay **unplugged**: Merlin powers both. Plug Merlin's USB-C in.

**Check, within 10 seconds:**
- "Nimue sniffer: connected" in green, and the device list fills.
- "Iolaus LoRa: connected", then within 30 s "Router guard OK. Argus on ch N".
- Press BOOT on Nimue: the screen shows "Locked ...", the big dBm number and the bar, and the speaker beeps faster as you walk toward the device.
- The Sound button cycles off, low, mid, high.

## 9. Router guard test (optional)

Argus counts deauth and disassoc frames on the router's channel. Normal home traffic gives a few now and then, below the alarm level (more than 8 in 10 s). A real attack sends hundreds. If the screen ever shows a red "ROUTER ATTACK" box and a siren, write down the "from" MAC it shows. Only test against your own router and devices.

## If something fails

Write down the step number and exactly what the screen, OLED or terminal says, and send it to Claude. Do not change wiring while anything is powered.
