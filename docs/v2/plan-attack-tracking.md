# Roadmap: find devices that attack your home devices (Taelor, 2026-10-30)

Goal: notice when something nearby attacks Taelor's own cameras and other home devices over Wi-Fi or
Bluetooth, and walk the finder to the attacker using signal strength. Own home and own network only.
The C5 only listens; it never sends anything back (no counter-attacks, no jamming).

Status: plan only. Nothing below is built.

## What the C5 can and cannot see
| Attack | What it looks like on air | Can the C5 see it? |
|---|---|---|
| Deauth / disassoc flood (kicks a camera off Wi-Fi) | many management frames, subtype 12 or 10, often "from" the router's MAC | Yes (already captures management frames) |
| Beacon flood (hundreds of fake networks) | beacons from many new random MACs at the same strength | Yes |
| Probe flood | probe requests from many random MACs, fast | Yes |
| Evil twin (fake copy of your network) | a second access point with your network name on another MAC/channel | Yes |
| Bluetooth spam (pop-up floods, fake trackers) | bursts of adverts from many random addresses | Yes, when Bluetooth scanning is on |
| Raw radio jamming (noise, not Wi-Fi) | no readable frames, just noise | No: needs a spectrum analyser |
| Attacks over the internet or wired network | nothing on the air near you | No |

Attackers fake their MAC address, so the address is not trusted. What gives them away is where the
frames come from: the strength they arrive at, and that it differs from the real device they pretend to be.

## Phases
### A. Detect (C5 firmware, no new parts)
1. Count suspicious frames per channel each second: deauth/disassoc, new-MAC beacons, probe requests,
   Bluetooth adverts from new random addresses.
2. Learn a quiet baseline for each count over the first minutes (same idea as the hand-wave baseline).
3. Raise an alert only when a count stays well above baseline for a few seconds (one stray deauth is normal
   when a device leaves). Record type, channel, the targeted MAC and the signal strength.
4. Watch-list: Taelor's own devices (cameras, Tuya plug, router) saved by MAC, so an attack aimed at them is
   flagged first. Router beacons give the "real" strength to compare a spoofed router against.
5. Send `A,<type>,<channel>,<target MAC>,<dBm>,<count>` lines to the R4 and the phone.

### B. Point at the attacker
1. "Track attack": the C5 parks on the attack's channel and follows only the attack frames, using the same
   bars, beeps, wobble and Save log as tracking a device.
2. If the "router" frames arrive at a different strength than the real router's beacons, show "spoofed:
   not your router".
3. Sweep with the directional antenna; the loudest direction is the attacker. Short attacks: the log keeps
   the last strength so you can resume when it starts again.

### C. Make it robust
1. Hop across 2.4 and 5 GHz while idle; stay put while tracking.
2. Rate-limit alerts and keep counts in fixed-size rings (no memory growth on the C5 or R4).
3. Test plan: on Taelor's own network only, with a second board of this project's own sending test deauth frames
   to Taelor's own test device; check the alert fires, nothing fires in a quiet hour, and the bars lead to the board.
4. Note for the router: turning on WPA3 or "protected management frames" stops deauth attacks working at all;
   the finder still sees them.

### D. Later (Phase 3/4)
- P4 screen: attack log and a direction view.
- Second listening point (the Phase 2.A telemetry idea) to tell which room the attacker is in.

## Open questions for Taelor
- Which home devices go on the watch-list (MACs)?
- Alert style: special beep pattern on the R4, phone pop-up, or both?
