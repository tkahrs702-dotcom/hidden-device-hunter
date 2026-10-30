# 2026-10-30 session: telemetry, one-command flash, quieter Bluetooth, attack roadmap

(No Obsidian step exists in this repo; Taelor's rules live in claude-hub `config/CLAUDE.md`, outside this project's access. This is the session note in the repo's usual place. If claude-hub defines an Obsidian export, paste this note into it.)

**What happened**
- Read Gemini's telemetry research (114-page PDF export on HaLjr). Kept the useful ideas, rejected its code patches (they would undo working code) and one wiring diagram that put 5 V on the shifter's 3.3 V side. Gemini's phone-screen code was never in the PDF.
- Telemetry built and merged (PR #18): packets/s, idle baseline, hand-wave spike flag and signal wobble on the locked target, shown on the R4 page and the Bluetooth page; Save log button (CSV). A separate reviewer found 4 bugs (spike flag could latch, misleading 0 packets/s, log cap, CSV quotes); all fixed. A plug never spikes: only a streaming camera does.
- C5 flashed with the telemetry. Earlier failures were no `/dev/serial/by-id` Espressif device (cable or no BOOT) and an old checkout not on `main`.
- Merged PR #19: `tools/flash.sh` (one command, prompts per board, port found by pattern, fallbacks), `docs/FLASH-AND-TEST.md` (whole sequence in one page), C5 low-profile Bluetooth (plain name, announces only 2 min after power-on/BOOT press, 10 min after a dropped link, never while connected; page finds the C5 by service UUID), and `docs/v2/plan-attack-tracking.md` (roadmap to detect attacks on home devices and point at the source; second listening point on a TTGO LoRa32 Taelor owns).
- Taelor's direction: Bluetooth is the main phone link, fully portable; drop the R4 Wi-Fi/web page eventually. Decision card on how (home-screen page, ready-made terminal app, own Android app) was never answered; recommendation was the installable home-screen page. The QR sheet was built then removed.

**Pending (not done)**
- Rename the C5 Bluetooth from "Mini Speaker" to the honest "Ananizapta" (decided in the P4 thread, 2026-10-30): `BLE_NAME` in `firmware/phase2-xiao-sniffer/phase2-xiao-sniffer.ino`, the chooser text in `tools/finder/finder.html`, and `docs/FLASH-AND-TEST.md` line 77. Taelor has not said "go".
- Reflash the C5 after that: `cd ~/Projects/hidden-device-hunter && git checkout main && git pull && bash tools/flash.sh c5`. The C5 on the bench still has the older Bluetooth name/code; the R4 has the telemetry build.
- Telemetry has not been seen working on the phone page with real boards yet (needs a locked device heard; wave test needs a Wi-Fi camera).
- Nothing deleted yet from the web side (R4 Wi-Fi server, finder.html).

**Lessons**
- Give whole sequences in a saved file, not one command per message (Taelor, session timeouts).
- Android never lists a BLE gadget in Bluetooth settings; it only connects through a page/app.
- Google share links and several hosts are blocked in cloud sessions; ask Taelor to paste text.
- Squash-merged branches must be restarted from `main` before new work.

Related: [FLASH-AND-TEST](../FLASH-AND-TEST.md), [attack roadmap](../v2/plan-attack-tracking.md), [README](../../README.md)
