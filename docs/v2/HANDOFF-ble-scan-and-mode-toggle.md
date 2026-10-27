# Handoff: finish the phone dashboard, Bluetooth scanning and the Wi-Fi/Bluetooth mode switch

Written 2026-10-27 so any engineer or AI assistant (Taelor plans to use Gemini) can continue without the chat history. Read `CLAUDE.md`, the README "Where we left off", then this file. Rules from CLAUDE.md that still apply: own home and own network only; the R4 is 5 V logic and the ESP32s are 3.3 V (never put 5 V on an ESP32 pin); this project is fully separate from SENTINEL; log sessions in `docs/worklog/`.

## What the system is
- Seeed XIAO ESP32-C5 (`firmware/phase2-xiao-sniffer`): receive-only Wi-Fi sniffer on 2.4 and 5 GHz, keeps a table of up to 160 devices (MAC, maker from a 771-entry table, signal, channel, packets), locks one target and streams its signal.
- Arduino UNO R4 WiFi (`firmware/phase2-r4-display`): LED-matrix bars, Modulino Buzzer beeps (faster when closer), optional Wi-Fi page (`webview.h`, network `HDH-Finder`, password `hunter2026`, http://192.168.4.1).
- Link C5 -> R4: UART 115200 through an Adafruit 757 level shifter. Taelor's wires to the C5 are crossed, so the code has `LINK_CROSSED = true` (C5 transmits on D7, listens on D6). The R4 side is D0 (RX) / D1 (TX). If the wires are ever put back (D6 = TX), set `LINK_CROSSED` to false.
- Phone link (new, on branch `claude/project-thread-j89159`, PR #16): the C5 is a Bluetooth LE peripheral named `HDH-Finder` (Nordic UART UUIDs `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`; notify `...0003...` C5 to phone; write `...0002...` phone to C5). Everything sent to the R4 is also streamed to the phone as text lines, in 20-byte chunks. The phone page is `tools/finder/finder.html` (Web Bluetooth, Chrome on Android; works in a "Demo" mode with fake data).

## Line protocol (C5 to R4 and phone)
`L,<MAC>,<cat>,<maker>` locked | `U` unlocked | `T,<now>,<avg>,<ch>` target signal, ~10/s | `S,<total>,<cams>,<iot>,<hidden>,<locked>` once a second | phone only: `B,<count>` then `D,<n>,<MAC>,<cat>,<avg>,<ch>,<maker>,<name>,<packets>,<seconds since heard>,<flags>` every 4 s (flags: 1 AP, 2 client, 4 hidden, 8 random MAC; "-" means empty).
Phone to C5 (lines): `K,<MAC>` track | `U` unlock | `V,<0-3>` buzzer volume (C5 forwards `V,n` to the R4).
`cat`: 0 none, 1 camera maker, 2 IoT chip.

## Status
Merged to main: PRs #9 to #15 (sniffer fixes, peak dot, R4 Wi-Fi page, hidden-network ranking, session note).
Open: PR #16 (branch `claude/project-thread-j89159`): delayed Wi-Fi start on the R4, serial pump while serving the page, link-status line on the R4 page, LINK_CROSSED, Bluetooth LE phone link on the C5, buzzer volume levels on the R4, `tools/finder/finder.html`, plans. Check CI before relying on it: the C5 Bluetooth code had not been confirmed to compile when this was written.
Hardware-verified (before today's changes): C5 locks the Tuya plug (10:5A:17:6C:02:4D) on boot; R4 beeps and bars follow. NOT hardware-verified: everything in PR #16 (Bluetooth link, phone page, volume, crossed-wire setting).

## Still to do (in this order)
1. Confirm PR #16 builds in CI (all four checks) and fix any Bluetooth compile errors. API facts (arduino-esp32 3.3.x BLE library): callbacks are `BLEServerCallbacks::onConnect(BLEServer*)`, `BLECharacteristicCallbacks::onWrite(BLECharacteristic*)` with `auto v = c->getValue()`; `BLEDevice::startAdvertising()`. The C5 uses the NimBLE backend, so no `BLE2902` descriptor is needed.
2. Hardware test with Taelor: flash the C5 (hold BOOT while plugging in), open `tools/finder/finder.html` in Chrome on Android (from Downloads as a local file, or host it on https; Web Bluetooth needs a secure context), press Connect, pick HDH-Finder. If the file URL is refused, host the page on a private https site.
3. Bluetooth device scanning (not built). In `phase2-xiao-sniffer.ino`: `BLEScan *s = BLEDevice::getScan(); s->setAdvertisedDeviceCallbacks(new AdvCb(), true /*duplicates for RSSI*/); s->setActiveScan(false /*receive-only*/); s->setInterval(100); s->setWindow(90); s->start(0, nullptr, false);` with `class AdvCb : public BLEAdvertisedDeviceCallbacks { void onResult(BLEAdvertisedDevice d) override {...} }`. In the callback only fill a `Pkt` (add kind `K_BLE`, `uint16_t company`) and `xQueueSend` it; read address with `parseMac(d.getAddress().toString().c_str(), ...)`, `d.getRSSI()`, `d.haveName() ? d.getName()`, company ID from `d.getManufacturerData()` (first two bytes, little endian), `d.getAddressType()` (0 public, else random). Add flag `F_BLE = 16` to `Dev`; make `findDev/addDev` key on MAC plus that flag (a phone can have the same MAC on Wi-Fi and Bluetooth). Public addresses can use the existing OUI lookup; random ones cannot. Clear scan results every ~20 s (`s->clearResults()`, done from `loop()`) so the library does not grow forever. Show BLE devices in the list and on the phone page (type "Bluetooth LE", channel column "BLE"), lock them by MAC like Wi-Fi targets.
4. Wi-Fi / Bluetooth mode switch (Taelor asked for a toggle): one radio does Wi-Fi sniffing, Bluetooth scanning and the phone link, so scanning both at once is slow. Add a mode: `WIFI`, `BLE`, `BOTH` (default BOTH). Phone page gets a three-way switch sending `M,<0|1|2>`; the C5 stops/starts the channel hopping (`hopNext`) or the BLE scan (`s->start/stop`) accordingly and reports `M,<mode>` in its status line so the page shows it. Also make a long BOOT press (3 s) cycle the mode for use without a phone, and show the mode on the R4 matrix for 1 s.
5. Phone-page features still to wire: volume buttons (done on the page, forwarded by the C5, handled in the R4 `V` line), Track button (done, `K,<MAC>`); verify both on hardware.
6. Optional: hosting the page on https so Taelor can open it without file access; saving the lock and mode across power cycles (Preferences, already used for the lock).

## Known limits and gotchas
- The R4 has little RAM. A 60-entry device list overflowed it once; keep R4 buffers small. Compile and look for `.heap` / `region RAM overflowed` errors.
- The R4 Wi-Fi chip is slow; never let it block reading the serial line (the code pumps the serial input between Wi-Fi writes).
- Hidden devices that only probe use random MACs and cannot be tracked by MAC.
- The Modulino Buzzer has no volume control; volume is faked with lower pitches (4000/2500/1500 Hz).
- The C5 must have `CDCOnBoot=cdc` in its FQBN to get the USB serial console.

## Build, flash, test
- CI compiles every sketch on every PR. Local compile: `arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer` and `arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi firmware/phase2-r4-display`.
- Flash the C5 holding BOOT while plugging it in, using the by-id port: `arduino-cli compile --upload -p /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_10:BD:A3:C7:F8:60-if00 --fqbn esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc firmware/phase2-xiao-sniffer`.
- Flash the R4 alone: `arduino-cli compile --upload -p "$(arduino-cli board list | grep -i unor4wifi | head -1 | cut -d' ' -f1)" --fqbn arduino:renesas_uno:unor4wifi firmware/phase2-r4-display`.
- Power both boards from a 2-port power bank (a small bank shuts off under the C5's load).
- Test the page without hardware: open `tools/finder/finder.html` in a browser and press Demo.

## How to work with Taelor
Taelor has no serial-monitor workflow, is low on credits, and dislikes long command chains. Keep replies to two or three plain lines, give one command at a time, take their reports as true, do not ask for monitor output, and think the design through before building.
