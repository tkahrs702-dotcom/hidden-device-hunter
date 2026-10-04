# 05. Pin reference and signal list

## Signals between boards
| Link | Signal | Level | Direction | Wires |
|---|---|---|---|---|
| R4 <-> XIAO (Phase 2) | UART 115200 8N1 | 5 V R4 / 3.3 V XIAO via shifter | both | R4 D1->B1/A1->XIAO D7; XIAO D6->A2/B2->R4 D0; grounds tied |
| XIAO <-> P4 (Phase 3) | UART 115200 8N1 | 3.3 V | both | XIAO D6->P4 pin 11 (GPIO5); P4 pin 14 (GPIO4)->XIAO D7; grounds tied |
| P4 -> R4 (optional) | UART | 3.3 V to 5 V via shifter | one way | P4 pin 8 (GPIO2)->A3/B3->R4 D0 |
| BNO085 <-> P4 | I2C 0x4A | 3.3 V | both | SDA pin 4, SCL pin 6 |
| R4 -> buzzer | tone | 5 V | out | D8 via 100 ohm |

## Power
| Board | Powered from | Notes |
|---|---|---|
| R4 | USB-C (computer or power bank) | 5 V pin ~4.7 V on USB; max 8 mA per pin, 60 mA total |
| XIAO (Phase 2) | R4 5V pin | do not also plug its USB-C |
| XIAO (Phase 3) | P4 header pin 1 (5V) | do not also plug its USB-C; Seeed's "5V dead on battery" refers to the XIAO's own battery pads, not this feed; P4 header 5V comes from its boost on LiPo |
| P4 | USB-C or MX1.25 LiPo | short-press POWER |

## Board pin cheats
- **R4:** D0 = RX, D1 = TX (Serial1). Debug on USB `Serial`. 12x8 LED matrix on board.
- **XIAO ESP32-S3:** D0=GPIO1, D1=2, D2=3, D3=4, D4=5 (SDA), D5=6 (SCL), D6=43 (TX), D7=44 (RX), D8=7, D9=8, D10=9.
- **P4 header:** see the full table in `03-phase3-p4-rf-camera.md`.

## Verification status
All items from the original guide are now checked against sources (see the log in `00-overview-and-safety.md`).
Two bench confirmations remain, both one-minute multimeter checks done when you have the hardware:
1. Header 5V (pin 1 or 3) reads about 5 V when the P4 runs on LiPo alone (the schematic says yes).
2. LiPo plug: red wire lands on J2 pin 1 (BAT) and black on pin 2 (GND) before you connect it.
