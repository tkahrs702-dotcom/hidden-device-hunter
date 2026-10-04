# 02. Phase 2: directional antenna + XIAO ESP32-S3 RF eye

Goal: a XIAO ESP32-S3 in monitor mode locks onto one MAC and streams its signal strength over UART to the R4, which beeps. A 7 dBi panel antenna makes the
reading directional: the tone peaks when the antenna points at the device. This also finds hidden clients that are not access points.
Requires Phase 1 working. R4 is 5 V, XIAO is 3.3 V, so a level shifter sits between them.

## Parts
| Part | Spec | Price |
|---|---|---|
| XIAO ESP32-S3 | Seeed, u.FL jack version, stock antenna in the box | 7.49 |
| u.FL to RP-SMA **female** bulkhead pigtail | 15 cm 1.13 mm (Superbat) or Proxicast 2-pack | 2.13-7.94 |
| 2.4 GHz panel antenna | Alfa APA-M04, 7 dBi, RP-SMA **male** | 9.97-13.99 |
| Logic level shifter, 4-channel | Adafruit 757 (BSS138), bidirectional | 3.95-4.50 |
| Jumper wires F-F | pack of 20-40 | 3-6 |
| Pin headers | for XIAO and shifter if not pre-soldered | have |
Pigtail female + Alfa male must mate. The XIAO's stock antenna is weak; the panel antenna replaces it.

## Adafruit 757 pin layout
```
   LOW side (3.3 V)                 HIGH side (5 V)
   LV  A1 A2 A3 A4 GND              HV  B1 B2 B3 B4 GND
```
Silkscreen read from Adafruit's board file (`adafruit/4-Channel-Level-Shifter-PCB`, 2026-10-03): left column top to bottom LV, A1, A2, A3, A4, GND; right column HV, B1, B2, B3, B4, GND.
A1<->B1 etc. Each channel works both ways. Low side 1.8 V up to HV; HV up to 10 V. Each line has a 10K pull-up on both sides, so idle UART lines sit high (correct for UART).

## Pin-to-pin wiring
| # | From | To | Note |
|---|---|---|---|
| 1 | R4 5V | Shifter HV | high-side reference |
| 2 | R4 5V | XIAO 5V pin | powers the XIAO (~4.7 V on USB power) |
| 3 | R4 GND | Shifter GND (HV side) | common ground |
| 4 | XIAO 3V3 | Shifter LV | low-side reference |
| 5 | XIAO GND | Shifter GND (LV side) | common ground |
| 6 | R4 D1 (TX) | Shifter B1 | R4 -> XIAO |
| 7 | Shifter A1 | XIAO D7 (RX, GPIO44) | |
| 8 | XIAO D6 (TX, GPIO43) | Shifter A2 | XIAO -> R4 |
| 9 | Shifter B2 | R4 D0 (RX) | |
| 10 | XIAO u.FL | pigtail u.FL end | press straight down until it clicks |
| 11 | pigtail RP-SMA female | Alfa APA-M04 RP-SMA male | hand-tight |
Buzzer stays on D8 with its 100 ohm resistor. TX always goes to RX of the other board; tie the grounds.

## Wiring diagram
```
  Uno R4 WiFi (5 V)             Adafruit 757                       XIAO ESP32-S3 (3.3 V)
 +----------------+   HIGH side         LOW side              +----------------------+
 | 5V  o----------+---> HV            LV <------------------- o 3V3                   |
 |       \        |                                            |                      |
 |        +-------+------------------------------------------> o 5V   (power in)       |
 | GND o----------+---> GND (HV)      GND (LV) <-------------- o GND                  |
 | D1 TX o--------+---> B1 ---- A1 ---------------------------> o D7  RX  (GPIO44)    |
 | D0 RX o<-------+---- B2 ---- A2 <--------------------------- o D6  TX  (GPIO43)    |
 | D8  o--[100R]--(+)BUZZER(-)--GND       |                    |  u.FL o--pigtail--> RP-SMA --> Alfa panel antenna
 | USB-C <== power bank                   |                    +----------------------+
 +----------------+
```

## Assembly
1. Solder pin headers to the XIAO (and shifter if needed). Check no solder bridges with the multimeter (continuity between neighbouring pins should be open).
2. **Flash the XIAO alone first** (USB to the computer, nothing else connected). Hold BOOT, plug USB-C, release BOOT only if the port does not appear.
3. Unplug everything. Wire rows 1-9 of the table.
4. Click the u.FL plug on **before power**: set one side of the connector into the socket, then press down on the other side straight. The connector is fragile. Remove the stock antenna.
5. Screw the pigtail to the panel antenna by hand.
6. Power the R4 from USB or the power bank.
**Warning:** never plug the XIAO's USB-C in while its 5V pin is fed from the R4. To reflash the XIAO, pull the 5V jumper (row 2) first.
Keep UART at 115200 or lower; the BSS138 shifter is slow above that.

## Software
- R4 side: `Serial1.begin(115200);` (D0/D1). Debug stays on `Serial` (USB).
- XIAO side: `Serial1.begin(115200, SERIAL_8N1, D7, D6);` (RX first, then TX). Write the pins WITH the D prefix.

### Link test (do this before any RF firmware)
XIAO sketch:
```cpp
void setup() { Serial1.begin(115200, SERIAL_8N1, D7, D6); }
void loop() { Serial1.println("ping from xiao"); delay(500); }
```
R4 sketch:
```cpp
void setup() { Serial.begin(115200); Serial1.begin(115200); }
void loop() {
  while (Serial1.available()) Serial.write(Serial1.read());
}
```
Serial monitor on the R4 should show "ping from xiao" twice a second. Then reverse the direction with a second test (R4 sends, XIAO prints to its USB `Serial`, with the 5V jumper pulled).

### RF firmware (not written yet)
XIAO: promiscuous (monitor) mode via `esp_wifi_set_promiscuous`, channel hop 1-13, parse 802.11 frame headers, keep a table of MACs with RSSI,
optional lock to one MAC, OUI vendor lookup (flag camera vendors). Send `MAC,RSSI` lines to the R4. R4 turns the line into beeps and matrix bars.

## Field test
Phone on your Wi-Fi in the next room. Pick its MAC. Hold the panel antenna in front of you, body behind it, and sweep slowly left to right; the tone should peak when aimed through the wall at the phone.

## Troubleshooting (Phase 2)
| Symptom | Check first | Then |
|---|---|---|
| Garbled or no serial between boards | TX to RX crossed? grounds tied? same baud? | drop to 57600; confirm shifter LV and HV both powered |
| XIAO resets or R4 browns out | R4 on a weak USB port | use the power bank; XIAO draws more while scanning |
| XIAO sees almost nothing | pigtail fully clicked? | try the stock antenna to confirm the XIAO works |
| Signal jumps | walls and bodies reflect 2.4 GHz | average readings, sweep slowly, antenna away from body |
| XIAO not in the port list | BOOT-hold procedure | different cable, dialout group |
| Antenna reads 5 GHz devices as missing | 2.4 GHz only | expected limit |
