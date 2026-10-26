# 2026-10-26: C5 sniffer and R4 display review

- Reviewed `phase2-xiao-sniffer` and `phase2-r4-display`. Pins (D6=GPIO11, D7=GPIO12, BOOT=GPIO28) are not the C5's USB pins (GPIO13/14), so the firmware cannot switch USB off. The default Tuya lock (10:5A:17:6C:02:4D, ver 1) is applied on the first boot; the R4's X confirms it was.
- Fixed: a locked target that went quiet for 4 s sent the C5 back to sweeping all 36 channels (8+ s per pass), so a plug that talks every few seconds was almost never caught and the R4 sat on X. The C5 now waits on the target's last channel for 30 s before sweeping.
- Fixed: USB text could stall the loop up to 100 ms per line when a computer was attached but nothing read the port; it is now dropped instead.
- Flash failures ("Failed to start stub flasher", "No serial data received") come from the USB reset into download mode, not the sketch. Hold BOOT while plugging the C5 in, then upload.
- Not run on hardware in this session; CI compiles it.
- PRs #9 and #10 merged. Added a peak marker on the R4: a dot on the top row over the best bar reached since the lock, so the closest approach can be read after walking away. Resets on a new lock.
- Phone dashboard (JS syntax-checked, C++ compiled by CI only, not run on hardware): the R4 makes Wi-Fi "HDH-Finder" and serves http://192.168.4.1 with the monitored target, live dBm, closest approach, graph, warmer/colder recommendation, sniffer counts and the 6 nearby suspects (`webview.h`; the C5 now sends B/D lines every 2 s).
