# 04. Phase 4: optional add-ons (any order)

Nothing here is bought or built. Each item says what it needs and where it connects. Keep to 3.3 V on every ESP32/P4 pin.
| Option | What it does | Needs | Where it connects |
|---|---|---|---|
| BLE tracker mode | finds AirTags, Tiles and BLE bugs | nothing new; XIAO BLE scan | XIAO firmware; same UART to P4/R4 |
| Lens finder | red LED ring around the camera shows camera lenses by glint | red LEDs + resistors | P4 free GPIO (e.g. pin 12 GPIO3) through a transistor; sizing TBD |
| Wi-Fi CSI motion | detects motion in a room from channel state | ESP32-S3 CSI firmware | XIAO |
| AD8317 broadband RF detector | detects non-Wi-Fi transmitters (analog bugs) | AD8317 module | analog output to a P4 or XIAO ADC pin; verify the pin is ADC-capable before wiring |
| Whole-house heat map | logs signal vs position, draws a floor plan | microSD (P4 slot) | P4 SD slot (SDIO, dedicated) |
| T3 LoRa remote nodes | wireless sensors that report to the finder | LILYGO T3 | LoRa; never transmit without the antenna attached |
| Phone alerts | notify on new unknown devices | HA or phone push | via Home Assistant |
Add a full pin table to this file when an option is chosen, following the pattern of Phases 1-3.
