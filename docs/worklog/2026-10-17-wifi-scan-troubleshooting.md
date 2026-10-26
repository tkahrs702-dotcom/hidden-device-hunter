# 2026-10-17 Wi-Fi scan troubleshooting (HaLjr)
Taelor: buzzer works but the finder "doesn't pick up the wifi".
- Finder (Modulino build, branch `modulino-buzzer`) lists only CenturyLink8864 at about -69 dBm.
- HaLjr's own Wi-Fi at the same time hears 6 named 2.4 GHz networks (CenturyLink8864 at 100%, Farm House x2, Godspeed, CenturyLink5785-2G).
- `firmware/diag-wifiscan` (plain WiFiS3 scan, no tasks or buzzer) gives the same single network, so it's not the finder code.
- Wi-Fi module firmware 0.6.0 (latest for renesas_uno 1.6.0).
- Likely causes: the radio is stuck (needs a full power cycle) or the antenna is blocked or weak. Next: unplug for 10 s and rescan, then move the board clear of metal or the PC and compare.
- Board is currently running `diag-wifiscan`; reflash `phase1-r4-finder` when done.

## Results (same day, later)
Loop: `tools/scan-compare.py` (R4 running diag-wifiscan vs HaLjr nmcli). RED every run.
Ruled out, one at a time:
- Finder code: plain diag-wifiscan gives the same result.
- Stuck radio: unplugged 7 s at 15:56, no change.
- USB port: computer port 1-1 and keyboard hub 3-1.4 give the same result.
- Wi-Fi chip firmware: reflashed ESP32-S3 with official 0.6.0 (arduino-fwuploader 2.4.1), no change.
- Bridge scan code: plain `WiFi.scanNetworks()` with no RSSI cutoff (arduino/uno-r4-wifi-usb-bridge cmds_wifi_station.h).
- Placement: Taelor walked the whole house earlier with the same result.
Evidence: R4 hears strong signals fine (phone hotspot "Man In The Middle" at -40 to -49 dBm), but reads the router at -68 dBm while HaLjr reads -48 to -57 dBm. It misses every network HaLjr rates under ~70%. Sept 29 hub log shows the same (router -68, Farm House rarely).
Conclusion: this R4's receive path (onboard PCB antenna) is about 10-20 dB weaker than HaLjr's, so it only hears nearby networks. Not fixable in software.
Options: use it as a close-range finder; confirm with a second ESP32 (TTGO) side by side; full scanning on the XIAO ESP32-S3 + external antenna (Phase 2).
Board restored to phase1-r4-finder (Modulino build).
