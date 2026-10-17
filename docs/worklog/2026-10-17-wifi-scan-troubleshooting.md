# 2026-10-17 Wi-Fi scan troubleshooting (HaLjr)
Taelor: buzzer works but the finder "doesn't pick up the wifi".
- Finder (Modulino build, branch `modulino-buzzer`) lists only CenturyLink8864 at about -69 dBm.
- HaLjr's own Wi-Fi at the same time hears 6 named 2.4 GHz networks (CenturyLink8864 at 100%, Farm House x2, Godspeed, CenturyLink5785-2G).
- `firmware/diag-wifiscan` (plain WiFiS3 scan, no tasks or buzzer) gives the same single network, so it's not the finder code.
- Wi-Fi module firmware 0.6.0 (latest for renesas_uno 1.6.0).
- Likely causes: the radio is stuck (needs a full power cycle) or the antenna is blocked or weak. Next: unplug for 10 s and rescan, then move the board clear of metal or the PC and compare.
- Board is currently running `diag-wifiscan`; reflash `phase1-r4-finder` when done.
