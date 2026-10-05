"""Board drawings (top views, not to scale) with pins in their real header order.

Pin order sources (see docs/manual/00-overview-and-safety.md verification log):
- Uno R4 WiFi: standard Uno R3 header layout (Arduino ABX00087 datasheet).
- XIAO ESP32-S3: Seeed pinout, USB-C at the top; left D0-D6, right 5V GND 3V3 D10 D9 D8 D7.
- Generic BSS138 4-channel shifter: HV1 HV2 HV GND HV3 HV4 / LV1 LV2 LV GND LV3 LV4 (SparkFun BOB-12009 layout,
  copied by most clones). Adafruit 757: LV A1 A2 A3 A4 GND / HV B1 B2 B3 B4 GND.
- Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3 40-pin header: Waveshare numbering, pin 1 = 5V.
"""
from svglib import C, MONO, SANS

R4_W, R4_H = 470, 330


def r4(s, x, y, title="Arduino Uno R4 WiFi  (top view, 5 V logic)"):
    s.rect(x, y, R4_W, R4_H, fill=C["board_r4"], stroke="#0a5154", sw=2, rx=14)
    # USB-C and barrel jack on the left edge
    s.rect(x - 18, y + 52, 40, 34, fill="#c0c0c0", stroke="#666", sw=1.5, rx=6)
    s.text(x + 30, y + 74, "USB-C", size=11, fill="white", weight="bold")
    s.rect(x - 22, y + 236, 58, 52, fill="#222", stroke="#000", sw=1.5, rx=3)
    s.text(x + 42, y + 266, "DC jack (VIN)", size=10, fill="#cde")
    s.pins["r4.USB"] = (x - 18, y + 69)
    # LED matrix 12 x 8
    mx, my = x + 150, y + 110
    s.rect(mx - 8, my - 8, 12 * 11 + 6, 8 * 11 + 6, fill="#0b5b5e", stroke="#0a4144", sw=1, rx=3)
    for r in range(8):
        for c in range(12):
            s.circle(mx + c * 11, my + r * 11, 2.6, fill="#f4d35e" if (r + c) % 5 == 0 else "#d8e9e9", stroke="none", sw=0)
    s.text(mx + 60, my + 98, "12 x 8 LED matrix", size=10, anchor="middle", fill="#d8f0f0")
    # ESP32-S3 Wi-Fi module (the R4's own radio)
    s.rect(x + 330, y + 110, 100, 70, fill="#9aa5a8", stroke="#555", sw=1.5, rx=3)
    s.text(x + 380, y + 140, "ESP32-S3", size=10, anchor="middle", weight="bold")
    s.text(x + 380, y + 154, "Wi-Fi module", size=10, anchor="middle")
    s.text(x + 380, y + 168, "(on-board)", size=9, anchor="middle", fill="#333")
    s.text(x + R4_W / 2, y + 226, title, size=12, anchor="middle", fill="white", weight="bold")
    # Top digital header: 10 + 8 pins (female sockets)
    p = 16
    top = [("SCL", "SCL"), ("SDA", "SDA"), ("AREF", "AREF"), ("GND_TOP", "GND"), ("D13", "13"), ("D12", "12"),
           ("D11", "11"), ("D10", "10"), ("D9", "9"), ("D8", "8")]
    s.rect(x + 142, y + 8, 10 * p + 4, 20, fill="#111", stroke="none", rx=2)
    s.header("r4", top, x + 152, y + 18, p, female=True, rotate=True, label_side="below")
    top2 = [("D7", "7"), ("D6", "~6"), ("D5", "~5"), ("D4", "4"), ("D3", "~3"), ("D2", "2"), ("D1", "TX>1"), ("D0", "RX<0")]
    s.rect(x + 318, y + 8, 8 * p + 4, 20, fill="#111", stroke="none", rx=2)
    s.header("r4", top2, x + 328, y + 18, p, female=True, rotate=True, label_side="below")
    s.text(x + 400, y + 82, "DIGITAL", size=9, anchor="middle", fill="#cde")
    # Bottom power + analog headers
    pw = [("NC", ""), ("IOREF", "IOREF"), ("RESET", "RESET"), ("3V3", "3.3V"), ("5V", "5V"), ("GND1", "GND"), ("GND2", "GND"), ("VIN", "VIN")]
    s.rect(x + 182, y + R4_H - 28, 8 * p + 4, 20, fill="#111", stroke="none", rx=2)
    s.header("r4", pw, x + 192, y + R4_H - 18, p, female=True, rotate=True, label_side="above")
    an = [("A0", "A0"), ("A1", "A1"), ("A2", "A2"), ("A3", "A3"), ("A4", "A4"), ("A5", "A5")]
    s.rect(x + 334, y + R4_H - 28, 6 * p + 4, 20, fill="#111", stroke="none", rx=2)
    s.header("r4", an, x + 344, y + R4_H - 18, p, female=True, rotate=True, label_side="above")
    s.text(x + 248, y + R4_H - 66, "POWER", size=9, anchor="middle", fill="#cde")
    s.text(x + 384, y + R4_H - 66, "ANALOG IN", size=9, anchor="middle", fill="#cde")


XIAO_W, XIAO_H = 210, 214


def xiao(s, x, y, title="Seeed XIAO ESP32-S3", subtitle="top view, 3.3 V logic"):
    # USB-C sticks out of the top edge
    s.rect(x + XIAO_W / 2 - 26, y - 14, 52, 30, fill="#c0c0c0", stroke="#666", sw=1.5, rx=6)
    s.pins["x.USB"] = (x + XIAO_W / 2, y - 14)
    s.rect(x, y, XIAO_W, XIAO_H, fill=C["board_xiao"], stroke="#000", sw=2, rx=12)
    s.rect(x + 70, y + 30, 70, 58, fill="#b8b8b8", stroke="#666", sw=1.2, rx=3)
    s.text(x + 105, y + 56, "ESP32-S3", size=10, anchor="middle", weight="bold")
    s.text(x + 105, y + 70, "shield", size=9, anchor="middle")
    s.rect(x + 74, y + 98, 16, 11, fill="#555", stroke="#999", sw=1, rx=2)
    s.text(x + 82, y + 120, "BOOT", size=8, anchor="middle", fill="#ddd")
    s.text(x + 82, y + 129, "GPIO0", size=7, anchor="middle", fill="#aaa")
    s.rect(x + 120, y + 98, 16, 11, fill="#555", stroke="#999", sw=1, rx=2)
    s.text(x + 128, y + 120, "RESET", size=8, anchor="middle", fill="#ddd")
    # u.FL antenna socket
    s.circle(x + 105, y + 152, 9, fill=C["pin"], stroke="#7a6a20", sw=1.5)
    s.circle(x + 105, y + 152, 3, fill="#7a6a20", stroke="none", sw=0)
    s.pins["x.UFL"] = (x + 105, y + 152)
    s.text(x + 92, y + 150, "u.FL", size=9, anchor="end", fill="white", weight="bold")
    s.text(x + 92, y + 161, "antenna", size=9, anchor="end", fill="white", weight="bold")
    p = 24
    left = [("D0", "D0"), ("D1", "D1"), ("D2", "D2"), ("D3", "D3"), ("D4", "D4"), ("D5", "D5"), ("D6", "D6 TX")]
    right = [("5V", "5V"), ("GND", "GND"), ("3V3", "3V3"), ("D10", "D10"), ("D9", "D9"), ("D8", "D8"), ("D7", "D7 RX")]
    s.header("x", left, x + 12, y + 30 + 2, p, horizontal=False, label_side="right", size=10)
    s.header("x", right, x + XIAO_W - 12, y + 30 + 2, p, horizontal=False, label_side="left", size=10)
    s.text(x + 22, y + 32 + 6 * p + 16, "GPIO43", size=8, fill="#9fd", font=MONO)
    s.text(x + XIAO_W - 22, y + 32 + 6 * p + 16, "GPIO44", size=8, fill="#9fd", font=MONO, anchor="end")
    s.text(x + 22, y + 32 + 6 * p + 26, "UART TX", size=8, fill="#9fd", font=MONO)
    s.text(x + XIAO_W - 22, y + 32 + 6 * p + 26, "UART RX", size=8, fill="#9fd", font=MONO, anchor="end")
    s.text(x + 92, y + XIAO_H + 18, title, size=12, anchor="end", weight="bold")
    s.text(x + 92, y + XIAO_H + 33, subtitle, size=10, anchor="end", fill=C["muted"])


SH_W, SH_H = 176, 120


def shifter(s, x, y, title="Level shifter|generic BSS138, 4-ch", show_mosfets=True):
    s.rect(x, y, SH_W, SH_H, fill=C["board_shift"], stroke="#0e2d55", sw=2, rx=8)
    p = 28
    hv = [("HV1", "HV1"), ("HV2", "HV2"), ("HV", "HV"), ("GND_HV", "GND"), ("HV3", "HV3"), ("HV4", "HV4")]
    lv = [("LV1", "LV1"), ("LV2", "LV2"), ("LV", "LV"), ("GND_LV", "GND"), ("LV3", "LV3"), ("LV4", "LV4")]
    s.header("sh", hv, x + 18, y + 14, p, label_side="below", size=10)
    s.header("sh", lv, x + 18, y + SH_H - 14, p, label_side="above", size=10)
    if show_mosfets:
        for i in range(4):
            s.rect(x + 34 + i * 30, y + 50, 14, 12, fill="#222", stroke="#000", sw=1)
        s.text(x + SH_W / 2, y + 74, "4 x BSS138 \"J1Y\"", size=8, anchor="middle", fill="#cfe")
    s.text(x + SH_W + 8, y + 18, "HIGH side (5 V)", size=10, fill=C["5v"], weight="bold")
    s.text(x + SH_W + 8, y + SH_H - 10, "LOW side (3.3 V)", size=10, fill="#c25f00", weight="bold")
    if title:
        t1, _, t2 = title.partition("|")
        s.text(x - 12, y + SH_H / 2 - 2, t1, size=11, anchor="end", weight="bold")
        s.text(x - 12, y + SH_H / 2 + 12, t2, size=10, anchor="end", fill=C["muted"])


def breadboard(s, x, y, cols=30, pitch=14, title="Half-size breadboard"):
    """Rows a-e (top half) and f-j (bottom half), columns 1..cols; one +/- rail pair at the top and bottom.
    Pin keys: bb.<row><col> e.g. bb.a10 ; rails bb.T+<col>, bb.T-<col>, bb.B+<col>, bb.B-<col>."""
    W = cols * pitch + 40
    H = 18 * pitch
    s.rect(x, y, W, H, fill="#f2efe6", stroke="#b8b2a0", sw=1.5, rx=6)

    def hole(key, hx, hy):
        s.pins[key] = (hx, hy)
        s.rect(hx - 3, hy - 3, 6, 6, fill="#3b3b3b", stroke="none", sw=0, rx=1)

    x0 = x + 26
    ys = {}
    yy = y + pitch
    ys["T+"] = yy; ys["T-"] = yy + pitch
    rows_top = "edcba"   # e nearest the centre gap
    base = yy + pitch * 2.5
    for i, r in enumerate("abcde"):
        ys[r] = base + i * pitch
    gap_y = ys["e"] + pitch
    for i, r in enumerate("fghij"):
        ys[r] = gap_y + pitch + i * pitch
    ys["B-"] = ys["j"] + pitch * 1.5
    ys["B+"] = ys["B-"] + pitch
    # rail lines
    s.line(x0 - 8, ys["T+"] - pitch / 2 + 1, x0 + (cols - 1) * pitch + 8, ys["T+"] - pitch / 2 + 1, C["5v"], 1.5)
    s.line(x0 - 8, ys["T-"] + pitch / 2 - 1, x0 + (cols - 1) * pitch + 8, ys["T-"] + pitch / 2 - 1, "#1f5fbf", 1.5)
    s.line(x0 - 8, ys["B-"] - pitch / 2 + 1, x0 + (cols - 1) * pitch + 8, ys["B-"] - pitch / 2 + 1, "#1f5fbf", 1.5)
    s.line(x0 - 8, ys["B+"] + pitch / 2 - 1, x0 + (cols - 1) * pitch + 8, ys["B+"] + pitch / 2 - 1, C["5v"], 1.5)
    s.text(x + 10, ys["T+"] + 4, "+", size=12, anchor="middle", fill=C["5v"], weight="bold")
    s.text(x + 10, ys["T-"] + 4, "-", size=14, anchor="middle", fill="#1f5fbf", weight="bold")
    s.text(x + 10, ys["B-"] + 4, "-", size=14, anchor="middle", fill="#1f5fbf", weight="bold")
    s.text(x + 10, ys["B+"] + 4, "+", size=12, anchor="middle", fill=C["5v"], weight="bold")
    s.rect(x + 6, gap_y - 3, W - 12, 6, fill="#d9d4c4", stroke="none", sw=0)
    for c in range(1, cols + 1):
        hx = x0 + (c - 1) * pitch
        for r in "abcdefghij":
            hole(f"bb.{r}{c}", hx, ys[r])
        for rail in ("T+", "T-", "B-", "B+"):
            hole(f"bb.{rail}{c}", hx, ys[rail])
        if c in (1, 5, 10, 15, 20, 25, 30):
            s.text(hx, ys["a"] - pitch * 0.62, str(c), size=8, anchor="middle", fill="#777", font=MONO)
            s.text(hx, ys["j"] + pitch * 0.85, str(c), size=8, anchor="middle", fill="#777", font=MONO)
    for r in "abcdefghij":
        s.text(x0 - 14, ys[r] + 3, r, size=8, anchor="middle", fill="#777", font=MONO)
    s.text(x, y + H + 16, title, size=11, anchor="start", weight="bold")
    return W, H


def piezo_on_board(s, plus_key, minus_key, below=True, label="Passive piezo buzzer"):
    (x1, y1), (x2, y2) = s.p(plus_key), s.p(minus_key)
    cx = (x1 + x2) / 2
    cy = y1 + (34 if below else -34)
    s.line(x1, y1, x1, cy, "#888", 2)
    s.line(x2, y2, x2, cy, "#888", 2)
    s.circle(cx, cy, 24, fill="#111", stroke="#000", sw=2)
    s.circle(cx, cy, 4, fill="#444", stroke="none", sw=0)
    s.text(x1 - 4, cy - 10, "+", size=13, anchor="middle", fill="white", weight="bold")
    s.rect(cx + 30, cy - 9, 215, 32, fill="white", stroke="#999", sw=0.8, rx=3, opacity=0.92)
    s.text(cx + 34, cy + 4, label, size=10, anchor="start", weight="bold")
    s.text(cx + 34, cy + 17, "(+ leg is the longer one / marked +)", size=9, anchor="start", fill=C["muted"])


def resistor_on_board(s, k1, k2, label="100 ohm"):
    (x1, y1), (x2, y2) = s.p(k1), s.p(k2)
    s.line(x1, y1, x2, y2, "#888", 2)
    mx = (x1 + x2) / 2
    s.rect(mx - 15, y1 - 6, 30, 12, fill="#e8c99b", stroke="#9b7a4a", sw=1.2, rx=5)
    for i, col in enumerate(["#8b4513", "#000", "#8b4513", "#d4af37"]):  # brown black brown gold = 100 ohm 5%
        s.rect(mx - 10 + i * 6, y1 - 6, 3, 12, fill=col, stroke="none", sw=0)
    s.rect(mx - 24, y1 - 22, 48, 14, fill="white", stroke="#999", sw=0.8, rx=3)
    s.text(mx, y1 - 11, label, size=10, anchor="middle", weight="bold")


def jumper_legend(s, x, y):
    s.text(x, y, "Jumper ends:", size=10, weight="bold")
    s.text(x, y + 14, "R4 headers are SOCKETS: wire end must be a male pin.", size=10)
    s.text(x, y + 28, "XIAO, shifter and P4 headers are PINS: wire end must be female.", size=10)


# Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3 40-pin header, Waveshare numbering (pin 1 = 5V, odd pins left column).
P4_HEADER = {
    1: "5V", 2: "3V3", 3: "5V", 4: "GPIO7 SDA", 5: "GND", 6: "GPIO8 SCL", 7: "GPIO37", 8: "GPIO2",
    9: "GPIO38", 10: "GND", 11: "GPIO5", 12: "GPIO3", 13: "GND", 14: "GPIO4", 15: "GPIO21", 16: "GPIO28",
    17: "GPIO22", 18: "3V3", 19: "GND", 20: "GPIO29", 21: "GPIO24", 22: "GPIO30", 23: "GPIO25", 24: "GPIO31",
    25: "USBD_N", 26: "GND", 27: "USBD_P", 28: "GPIO34", 29: "GND", 30: "GPIO35", 31: "GPIO32", 32: "GPIO49",
    33: "GND", 34: "GPIO50", 35: "GPIO46", 36: "GPIO51", 37: "GPIO47", 38: "GPIO52", 39: "GPIO48", 40: "GND",
}


def p4_header(s, x, y, pitch=24, used=(), gap=170):
    """2 x 20 header, pin 1 top-left, drawn with the two rows spread `gap` px apart so the labels fit between
    them and wires can leave outwards (odd pins to the left, even pins to the right). Keys p4.<n>."""
    s.rect(x - 12, y - 12, 24, pitch * 19 + 24, fill="#111", stroke="#000", sw=1, rx=2)
    s.rect(x + gap - 12, y - 12, 24, pitch * 19 + 24, fill="#111", stroke="#000", sw=1, rx=2)
    for n in range(1, 41):
        col = (n - 1) % 2
        row = (n - 1) // 2
        px = x + col * gap
        py = y + row * pitch
        s.pin(f"p4.{n}", px, py)
        name = P4_HEADER[n]
        hot = n in used
        fill = "#ffffff" if hot else "#8ea3b8"
        weight = "bold" if hot else "normal"
        if col == 0:
            s.text(px + 14, py + 4, f"{n:>2} {name}", size=10, anchor="start", fill=fill, font=MONO, weight=weight)
        else:
            s.text(px - 14, py + 4, f"{name} {n:<2}", size=10, anchor="end", fill=fill, font=MONO, weight=weight)


def bno085(s, x, y):
    """Adafruit 4754 BNO085, drawn with the four I2C pins only, in the order they land on the P4 header
    (pins 2, 4, 6, 10). The real breakout has more pins in a different order: wire by label. Keys bno.*."""
    s.rect(x, y, 150, 130, fill=C["board_bno"], stroke="#111", sw=2, rx=8)
    s.rect(x + 50, y + 40, 40, 40, fill="#222", stroke="#000", sw=1)
    s.text(x + 70, y + 64, "BNO085", size=9, anchor="middle", fill="#ddd")
    s.header("bno", [("VIN", "VIN"), ("SDA", "SDA"), ("SCL", "SCL"), ("GND", "GND")], x + 14, y + 26, 26,
             horizontal=False, label_side="right", size=10)
    s.rect(x + 118, y + 50, 20, 30, fill="#ddd", stroke="#666", sw=1, rx=2)
    s.text(x + 128, y + 96, "STEMMA", size=7, anchor="middle", fill="#ddd")
    s.text(x + 128, y + 105, "QT", size=7, anchor="middle", fill="#ddd")
    s.text(x + 75, y + 150, "Adafruit 4754 BNO085 IMU", size=12, anchor="middle", weight="bold")
    s.text(x + 75, y + 165, "I2C address 0x4A, mount flat", size=10, anchor="middle", fill=C["muted"])
