#!/usr/bin/env python3
"""Four-board pin-to-pin wiring diagram (Merlin, Nimue, Iolaus, Argus).

Header labels are copied from the silkscreen in Taelor's Merlin board photo.
Usage: python3 tools/diagrams/merlin_wiring.py tools/diagrams/photos/merlin-header-strip.jpg \
       tools/diagrams/photos/merlin-speaker.jpg docs/manual/diagrams/merlin-four-board-wiring.svg
"""
import base64
import sys

strip_jpg, spk_jpg, out = sys.argv[1:4]

def b64(p):
    return base64.b64encode(open(p, "rb").read()).decode()

W, H = 1500, 1380
RED, BLK, GRN, PUR, GRY = "#d32f2f", "#111111", "#1b8a3a", "#7b2fa8", "#9a9a9a"
ORG = "#e07b00"  # 3.3 V power
s = []
add = s.append

def text(x, y, t, size=14, fill="#222", weight="normal", anchor="start", extra=""):
    add(f'<text x="{x}" y="{y}" font-size="{size}" fill="{fill}" font-weight="{weight}" '
        f'text-anchor="{anchor}" {extra}>{t}</text>')

def dot(x, y, fill, r=7):
    add(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="#fff" stroke-width="1.5"/>')

def wire(pts, color, dash=False, tag=None, tag_at=None):
    d = " ".join(f"{x},{y}" for x, y in pts)
    da = ' stroke-dasharray="10 7"' if dash else ""
    add(f'<polyline points="{d}" fill="none" stroke="{color}" stroke-width="4" '
        f'stroke-linejoin="round" stroke-linecap="round"{da}/>')
    if tag:
        tx, ty = tag_at
        add(f'<rect x="{tx-17}" y="{ty-11}" width="34" height="20" rx="10" fill="#fff" stroke="{color}" stroke-width="2"/>')
        text(tx, ty + 4, tag, 12, color, "bold", "middle")

add(f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" '
    f'width="{W}" height="{H}" viewBox="0 0 {W} {H}" font-family="Helvetica, Arial, sans-serif">')
add(f'<rect width="{W}" height="{H}" fill="#ffffff"/>')
text(W / 2, 46, "Hidden Device Hunter: pin-to-pin wiring", 28, "#111", "bold", "middle")
text(W / 2, 76, "Every signal wire is 3.3 V. Only ONE wire carries 5 V: W1, which powers Nimue. "
     "Iolaus (LoRa) is powered from 3.3 V (W8), never 5 V.", 16, "#111", "bold", "middle")
add(f'<line x1="430" y1="104" x2="460" y2="104" stroke="{RED}" stroke-width="5"/>')
text(468, 109, "5 V power only", 14, RED, "bold")
add(f'<line x1="620" y1="104" x2="650" y2="104" stroke="{ORG}" stroke-width="5"/>')
text(658, 109, "3.3 V power", 14, ORG, "bold")
add(f'<line x1="790" y1="104" x2="820" y2="104" stroke="{GRN}" stroke-width="5"/>')
add(f'<line x1="822" y1="104" x2="852" y2="104" stroke="{PUR}" stroke-width="5"/>')
text(860, 109, "3.3 V data signals", 14, "#333", "bold")

# ---- Merlin header (J3 on Waveshare's schematic: odd pins on the left, even on the right, pin 1 = the 5V end at the top) ----
Y0, P = 330, 34
AX, BX = 640, 850            # A = 5V row of the silkscreen, B = 3V3 row
A = ["5V", "5V", "GND", "37", "38", "5", "GND", "21", "22", "GND",
     "24", "25", "DM", "DP", "GND", "32", "GND", "46", "47", "48"]
B = ["3V3", "SDA", "SCL", "2", "GND", "3", "4", "28", "3V3", "29",
     "30", "31", "GND", "34", "35", "49", "50", "51", "52", "GND"]
used_a = {0: RED, 2: BLK, 5: GRN, 6: BLK, 7: GRN, 8: PUR}
used_b = {6: PUR, 8: ORG}
y = lambda i: Y0 + i * P

add(f'<rect x="{AX-25}" y="{Y0-58}" width="{BX-AX+50}" height="{20*P+76}" rx="12" fill="#eef3fb" stroke="#2456a4" stroke-width="3"/>')
text((AX + BX) / 2, Y0 - 98, "MERLIN", 24, "#2456a4", "bold", "middle")
text((AX + BX) / 2, Y0 - 76, "Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3, 40-pin header", 13, "#555", anchor="middle")
text(AX + 10, Y0 - 32, "odd pins", 12, "#2456a4", "bold")
text(BX - 10, Y0 - 32, "even pins", 12, "#2456a4", "bold", "end")
text((AX + BX) / 2, Y0 - 32, "pin nos.", 11, GRY, anchor="middle")
for i in range(20):
    ca, cb = used_a.get(i), used_b.get(i)
    la = ("GPIO" + A[i]) if A[i].isdigit() else A[i]
    lb = ("GPIO" + B[i]) if B[i].isdigit() else B[i]
    fa = RED if A[i] == "5V" else ("#111" if ca else GRY)
    fb = ORG if B[i] == "3V3" else ("#111" if cb else GRY)
    text(AX + 16, y(i) + 5, la, 14, fa, "bold" if ca or A[i] == "5V" else "normal")
    text(BX - 16, y(i) + 5, lb, 14, fb, "bold" if cb or B[i] == "3V3" else "normal", "end")
    text((AX + BX) / 2, y(i) + 4, f"{2*i+1} | {2*i+2}", 11, "#8a8a8a", anchor="middle")
    dot(AX, y(i), ca or "#c4c4c4", 7 if ca else 5)
    dot(BX, y(i), cb or "#c4c4c4", 7 if cb else 5)
text((AX + BX) / 2, y(19) + 40, "other end of the header", 12, GRY, anchor="middle")

# Photo strip, aligned row-for-row with the drawing
PX = 985
add(f'<image x="{PX}" y="{Y0-P/2}" width="78.5" height="{20*P}" xlink:href="data:image/jpeg;base64,{b64(strip_jpg)}"/>')
add(f'<rect x="{PX}" y="{Y0-P/2}" width="78.5" height="{20*P}" fill="none" stroke="#2456a4" stroke-width="2"/>')
text(PX + 39, Y0 - 44, "your photo,", 12, "#2456a4", "bold", "middle")
text(PX + 39, Y0 - 29, "same header", 12, "#2456a4", "bold", "middle")
text(PX + 92, y(0) + 5, "← 5V end", 13, "#2456a4", "bold")

# ---- Nimue (XIAO ESP32-C5), upper left ----
NX0, NX1 = 120, 340
NY = [360 + k * 28 for k in range(7)]
add(f'<rect x="{NX0}" y="300" width="{NX1-NX0}" height="262" rx="12" fill="#eaf7ee" stroke="#1b8a3a" stroke-width="3"/>')
text((NX0 + NX1) / 2, 286, "NIMUE", 22, "#1b8a3a", "bold", "middle")
text((NX0 + NX1) / 2, 323, "XIAO ESP32-C5 sniffer", 13, "#555", anchor="middle")
text((NX0 + NX1) / 2, 340, "USB-C end at top", 11, GRY, anchor="middle")
left = ["D0", "D1", "D2", "D3", "D4", "D5", "D6"]
right = ["5V", "GND", "3V3", "D10", "D9", "D8", "D7"]
for k in range(7):
    lu = k == 6
    ru = k in (0, 1, 6)
    text(NX0 + 14, NY[k] + 5, left[k] + (" (RX)" if lu else ""), 13, "#111" if lu else GRY, "bold" if lu else "normal")
    text(NX1 - 14, NY[k] + 5, ("(TX) " if k == 6 else "") + right[k], 13, "#111" if ru else GRY, "bold" if ru else "normal", "end")
    dot(NX0, NY[k], PUR if lu else "#c4c4c4", 7 if lu else 4)
    dot(NX1, NY[k], {0: RED, 1: BLK, 6: GRN}.get(k, "#c4c4c4"), 7 if ru else 4)

wire([(NX1, NY[0]), (560, NY[0]), (560, y(0)), (AX, y(0))], RED, tag="W1", tag_at=(385, NY[0]))
text((NX0 + NX1) / 2, 582, "5V pin = power in only.", 12, RED, "bold", "middle")
text((NX0 + NX1) / 2, 598, "Nimue makes its own 3.3 V inside.", 12, RED, "bold", "middle")
wire([(NX1, NY[1]), (530, NY[1]), (530, y(2)), (AX, y(2))], BLK, tag="W2", tag_at=(385, NY[1]))
wire([(NX1, NY[6]), (500, NY[6]), (500, y(5)), (AX, y(5))], GRN, tag="W3", tag_at=(385, NY[6]))
wire([(NX0, NY[6]), (80, NY[6]), (80, 150), (950, 150), (950, y(6)), (BX, y(6))], PUR, dash=True,
     tag="W4", tag_at=(515, 150))

# ---- Iolaus (handheld TTGO LoRa32), lower left ----
IY = [700, 735, 770, 805]
add(f'<rect x="{NX0}" y="640" width="{NX1-NX0}" height="225" rx="12" fill="#fdf1e4" stroke="#b5651d" stroke-width="3"/>')
text((NX0 + NX1) / 2, 626, "IOLAUS", 22, "#b5651d", "bold", "middle")
text((NX0 + NX1) / 2, 663, "handheld TTGO LoRa32", 13, "#555", anchor="middle")
text((NX0 + NX1) / 2, 680, "915 MHz LoRa listener", 11, GRY, anchor="middle")
for k, (lab, col) in enumerate([("GND", BLK), ("TX GPIO13", GRN), ("RX GPIO34", PUR), ("3V3", ORG)]):
    text(NX1 - 14, IY[k] + 5, lab, 14, "#111", "bold", "end")
    dot(NX1, IY[k], col)
text(NX0 + 14, 835, "T3 V1.6.1 pin labels", 11, GRY)
text(NX0 + 14, 848, "(TX/RX set in its firmware)", 11, GRY)
text((NX0 + NX1) / 2, 890, "((•)) talks to Argus by LoRa radio", 12, "#b5651d", "bold", "middle")
text((NX0 + NX1) / 2, 910, "3.3 V board: powered by W8 from", 12, ORG, "bold", "middle")
text((NX0 + NX1) / 2, 926, "Merlin 3V3. Never 5 V.", 12, ORG, "bold", "middle")

wire([(NX1, IY[0]), (440, IY[0]), (440, y(6)), (AX, y(6))], BLK, tag="W5", tag_at=(385, IY[0]))
wire([(NX1, IY[1]), (470, IY[1]), (470, y(7)), (AX, y(7))], GRN, tag="W6", tag_at=(385, IY[1]))
wire([(NX1, IY[2]), (500, IY[2]), (500, y(8)), (AX, y(8))], PUR, tag="W7", tag_at=(385, IY[2]))
wire([(NX1, IY[3]), (380, IY[3]), (380, 1035), (920, 1035), (920, y(8)), (BX, y(8))], ORG,
     tag="W8", tag_at=(650, 1035))

# ---- Right column: Argus, onboard speaker, unused C6-UART ----
RX0, RX1 = 1150, 1470
add(f'<rect x="{RX0}" y="250" width="{RX1-RX0}" height="150" rx="12" fill="#fdf1e4" stroke="#b5651d" stroke-width="3" stroke-dasharray="9 6"/>')
text(RX0 + 16, 280, "ARGUS", 20, "#b5651d", "bold")
text(RX0 + 16, 305, "router TTGO LoRa32, stays by the router", 13, "#333")
text(RX0 + 16, 327, "Power only (its own USB or LiPo).", 13, "#333")
text(RX0 + 16, 349, "No wires to Merlin.", 13, RED, "bold")
text(RX0 + 16, 375, "((•)) sends alerts to Iolaus by LoRa", 13, "#b5651d", "bold")

add(f'<rect x="{RX0}" y="425" width="{RX1-RX0}" height="300" rx="12" fill="#f6f6f6" stroke="#666" stroke-width="2"/>')
text(RX0 + 16, 453, "SOUND: Merlin's own speaker", 15, "#111", "bold")
add(f'<image x="{RX0+16}" y="466" width="130" height="190" xlink:href="data:image/jpeg;base64,{b64(spk_jpg)}"/>')
add(f'<rect x="{RX0+16}" y="466" width="130" height="190" fill="none" stroke="#666"/>')
for k, t in enumerate(["Your photo: the speaker", "(white plate) hangs on a", "red/black wire. The wire", "plugs into the white 2-pin", "socket above USB TO UART,", "by MIC1. Leave it plugged", "in."]):
    text(RX0 + 158, 484 + k * 19, t, 12, "#333")
for k, t in enumerate(["No header wires, no external piezo.",
                       "Driven by the onboard codec and amp,",
                       "set up in the Merlin firmware."]):
    text(RX0 + 16, 680 + k * 16, t, 12, "#333")

add(f'<rect x="{RX0}" y="745" width="{RX1-RX0}" height="92" rx="12" fill="#f6f6f6" stroke="#666" stroke-width="2"/>')
text(RX0 + 16, 772, "C6-UART pins (TX RX IO9 GND)", 15, "#111", "bold")
text(RX0 + 16, 796, "Bottom-right corner, by the C6 module.", 13, "#333")
text(RX0 + 16, 818, "Not used. Leave them empty.", 13, "#333")

# ---- Wire list ----
LY = 1075
add(f'<rect x="60" y="{LY}" width="1380" height="168" rx="12" fill="#fafafa" stroke="#ccc" stroke-width="2"/>')
text(80, LY + 28, "Wire list (Merlin pin numbers are J3 from Waveshare's schematic; pin 1 is the 5V end; the board prints GPIO numbers, not pin numbers)", 16, "#111", "bold")
rows = [
    ("W1", RED, "Nimue 5V → Merlin 5V (pin 1), the only 5 V wire"),
    ("W2", BLK, "Nimue GND → Merlin GND (pin 5)"),
    ("W3", GRN, "Nimue D7 (TX) → Merlin GPIO5 (pin 11), Merlin listens"),
    ("W4", PUR, "Merlin GPIO4 (pin 14) → Nimue D6 (RX), optional, dashed"),
    ("W5", BLK, "Iolaus GND → Merlin GND (pin 13)"),
    ("W6", GRN, "Iolaus GPIO13 (TX) → Merlin GPIO21 (pin 15), Merlin listens"),
    ("W7", PUR, "Merlin GPIO22 (pin 17) → Iolaus GPIO34 (RX)"),
    ("W8", ORG, "Iolaus 3V3 → Merlin 3V3 (pin 18), 3.3 V power"),
]
for k, (tag, col, t) in enumerate(rows):
    cx, cy = (80 if k < 4 else 760), LY + 58 + (k % 4) * 27
    add(f'<line x1="{cx}" y1="{cy-5}" x2="{cx+30}" y2="{cy-5}" stroke="{col}" stroke-width="4"'
        + (' stroke-dasharray="8 5"' if tag == "W4" else "") + "/>")
    text(cx + 40, cy, f"<tspan font-weight=\"bold\">{tag}</tspan>  {t}", 14, "#222")

text(60, LY + 196, "⚠ 3.3 V on every signal pin. Never put Merlin's 5V on any GPIO of Nimue, Iolaus or Merlin.", 13, RED, "bold")
text(60, LY + 218, "Feed Nimue from Merlin 5V only with Nimue's USB unplugged. Feed Iolaus from Merlin 3V3 only with its USB and battery unplugged.", 13, RED, "bold")
text(60, LY + 242, "METER CHECK 1 (Merlin on USB, nothing else wired, black probe on GND): pins 1 and 3 read 5.0 V; pins 2 and 18 read 3.3 V; GPIO5, 21 and 22 read 0 V. If not, stop.", 13, "#111", "bold")
text(60, LY + 264, "METER CHECK 2 (everything unplugged, meter on beep): each wire beeps end-to-end. Merlin 5V to GPIO5, GPIO21, GPIO22 and 3V3 must NOT beep.", 13, "#111", "bold")
text(60, LY + 286, "Camera (OV5647, Phase 3) uses Merlin's CAMERA ribbon socket, not header wires.", 13, "#333")
add("</svg>")
open(out, "w").write("\n".join(s))
