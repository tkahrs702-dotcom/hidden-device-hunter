#!/usr/bin/env python3
"""Compare what the R4 (running firmware/diag-wifiscan) hears with HaLjr's own Wi-Fi.
RED = the R4 misses named 2.4 GHz networks that HaLjr hears. Usage: scan-compare.py [port] [seconds]"""
import re, subprocess, sys, time
import serial

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyACM0"
secs = float(sys.argv[2]) if len(sys.argv) > 2 else 12

s = serial.Serial(port, 115200, timeout=0.3)
buf = b""
t = time.time()
while time.time() - t < secs:
    buf += s.read(4000)
s.close()
text = buf.decode(errors="replace")
r4 = {}
for m in re.finditer(r"^\s+(.+?)\s+([0-9A-F:]{17}) ch(\d+)\s+(-?\d+) dBm", text, re.M):
    r4[m.group(2)] = (m.group(1), int(m.group(4)))

out = subprocess.run(["nmcli", "-t", "-e", "no", "-f", "SSID,BSSID,CHAN,SIGNAL", "dev", "wifi", "list", "--rescan", "yes"],
                     capture_output=True, text=True).stdout
host = {}
for line in out.splitlines():
    p = line.rsplit(":", 2)          # SSID:BSSID(with colons):CHAN:SIGNAL
    ssid_bssid, chan, sig = p[0], int(p[1]), int(p[2])
    ssid, bssid = ssid_bssid[:-18], ssid_bssid[-17:]
    if ssid and chan <= 14:
        host[bssid] = (ssid, sig)

print(f"HaLjr hears {len(host)} named 2.4 GHz APs, R4 hears {len(r4)}")
for b, (ssid, sig) in sorted(host.items(), key=lambda x: -x[1][1]):
    seen = f"R4 {r4[b][1]} dBm" if b in r4 else "R4 MISSING"
    print(f"  {ssid:22} {b}  HaLjr {sig:3}%  {seen}")
missing = [b for b in host if b not in r4]
print("RED" if missing else "GREEN")
sys.exit(1 if missing else 0)
