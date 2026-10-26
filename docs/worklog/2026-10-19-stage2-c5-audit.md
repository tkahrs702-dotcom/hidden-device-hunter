# 2026-10-19 Phase 2 ESP32-C5 audit and wiring correction

- The connected board is the Seeed Studio XIAO ESP32-C5, not an unidentified generic C5 carrier. The merged PR #4 worklog records a 2026-10-18 flash and live reception on 5 GHz channel 36 and 2.4 GHz channel 2.
- Corrected the Phase 2 manual and drawing to use the XIAO C5's D6 TX (GPIO11) and D7 RX (GPIO12) pins, with the two UART directions routed through the Adafruit 757 level shifter.
- The drawing replaces the stale XIAO-S3 GPIO43/44 labels, removes the obsolete D8 passive-buzzer wiring, and calls for a 2.4/5 GHz antenna instead of the 2.4 GHz-only Alfa APA-M04.
- Corrected the local shared firmware profile to select D6/D7 and BOOT GPIO28 on the Seeed XIAO ESP32-C5 while retaining RX1/TX1 GPIO4/GPIO5 for generic C5 carrier builds.
- The current local C5 profile explicitly requests US regulatory settings and 2.4+5 GHz band mode, and hops 2.4 GHz channels 1-11 plus US non-DFS 5 GHz channels 36-48 and 149-165. DFS, 6 GHz, and simultaneous reception are not supported by this profile.
- Channel-tune and promiscuous-mode setup failures are reported; packet queue allocation is checked; dropped packet frames are counted atomically and reported.
- The current local changes have not been uploaded to the board. The earlier flash and RF reception are verified; the UART connection to the R4, BOOT-button operation, and lock persistence remain untested.
- Independent review found and corrected two related documentation issues: the general overview now names the Seeed XIAO ESP32-C5 (not the generic Dev Module), and the wiring diagram powers the R4 and XIAO separately over USB rather than connecting unverified 5 V rails.
- Taelor confirmed the 2.4/5 GHz directional antenna is a Rokland antenna that has been ordered but not delivered. Verify the exact cable/connector when it arrives.
- Independent follow-up review confirmed the XIAO C5 D6/GPIO11 TX and D7/GPIO12 RX paths match the installed XIAO board variant and local firmware. No new concrete issue was found; the UART wiring itself remains untested.
- Compile-only checks passed with `--warnings all` for `esp32:esp32:XIAO_ESP32C5:CDCOnBoot=cdc` and the generic `esp32:esp32:esp32c5` target. No firmware was uploaded.
