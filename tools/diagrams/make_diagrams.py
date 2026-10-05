#!/usr/bin/env python3
"""Generate the wiring diagrams and schematics in docs/manual/diagrams/.

Run from anywhere:  python3 tools/diagrams/make_diagrams.py
Pin numbers here must match the sketches in firmware/ (BUZZER_PIN 8, Serial1 on R4 D0/D1,
XIAO LINK_TX D6 / LINK_RX D7, BOOT = GPIO0) and the tables in docs/manual/.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from svglib import SVG, C, MONO, SANS  # noqa: E402
import boards  # noqa: E402

OUT = os.path.join(os.path.dirname(__file__), "..", "..", "docs", "manual", "diagrams")


def title(s, t, sub=None):
    s.text(24, 34, t, size=20, weight="bold")
    if sub:
        s.text(24, 56, sub, size=12, fill=C["muted"])


def table(s, x, y, head, rows, widths, size=11, title_text=None):
    if title_text:
        s.text(x, y - 10, title_text, size=13, weight="bold")
    rh = size * 1.75
    tw = sum(widths)
    s.rect(x, y, tw, rh, fill="#e9eef5", stroke="#9aa", sw=1)
    cx = x
    for h, w in zip(head, widths):
        s.text(cx + 6, y + rh * 0.68, h, size=size, weight="bold")
        cx += w
    for i, row in enumerate(rows):
        ry = y + rh * (i + 1)
        s.rect(x, ry, tw, rh, fill="white" if i % 2 == 0 else "#f7f7f7", stroke="#ccc", sw=0.8)
        cx = x
        for j, (cell, w) in enumerate(zip(row, widths)):
            color = None
            if isinstance(cell, tuple):
                cell, color = cell
            if color:
                s.rect(cx + 6, ry + rh * 0.3, 18, rh * 0.4, fill=color, stroke="#333", sw=0.8, rx=2)
                s.text(cx + 30, ry + rh * 0.68, cell, size=size, font=MONO if j else SANS)
            else:
                s.text(cx + 6, ry + rh * 0.68, cell, size=size, font=MONO if j in (1, 2) else SANS)
            cx += w
    return y + rh * (len(rows) + 1)


def note(s, x, y, w, rows, warn=False, size=11):
    h = len(rows) * size * 1.4 + 14
    s.rect(x, y, w, h, fill=C["warn"] if warn else C["note"], stroke="#c33" if warn else "#c9a400", sw=1.2, rx=6)
    s.lines(x + 10, y + size + 6, rows, size=size)
    return y + h


def usb_cable(s, pts, label=None):
    s.poly(pts, stroke="#555", sw=7)
    s.poly(pts, stroke="#9a9a9a", sw=4)
    if label:
        x, y = pts[len(pts) // 2]
        s.text(x + 8, y - 6, label, size=10, fill=C["muted"])


def power_bank(s, x, y):
    s.rect(x, y, 190, 80, fill="#4a4a4a", stroke="#222", sw=2, rx=10)
    s.text(x + 95, y + 36, "USB power bank", size=13, anchor="middle", fill="white", weight="bold")
    s.text(x + 95, y + 54, "5 V, no low-current auto-off", size=10, anchor="middle", fill="#ddd")
    s.rect(x + 150, y - 8, 26, 10, fill="#bbb", stroke="#666", sw=1)
    return (x + 163, y - 8)


# --------------------------------------------------------------------------------------------
# Phase 1
# --------------------------------------------------------------------------------------------
def phase1_wiring():
    s = SVG(1100, 760, "Phase 1 wiring: Uno R4 WiFi, 100 ohm resistor and passive piezo buzzer on a breadboard",
            "R4 D8 to breadboard a10; 100 ohm resistor c10 to c14; buzzer + in e14, - in e17; a17 to the top - rail; "
            "R4 GND (top header) to the top - rail; power bank to R4 USB-C.")
    title(s, "Phase 1 wiring: R4 + 100 ohm resistor + passive piezo buzzer",
          "Pin-to-pin. Every wire below is one jumper. Matches firmware/phase1-r4-finder (BUZZER_PIN = 8).")
    boards.r4(s, 70, 170)
    boards.breadboard(s, 600, 190)
    boards.resistor_on_board(s, "bb.c10", "bb.c14")
    boards.piezo_on_board(s, "bb.e14", "bb.e17")

    d8 = s.p("r4.D8")
    a10 = s.p("bb.a10")
    s.wire([d8, (d8[0], 128), (a10[0], 128), a10], C["tone"], label="W1  D8 -> a10", label_at=(560, 128))
    g = s.p("r4.GND_TOP")
    tm3 = s.p("bb.T-3")
    s.wire([g, (g[0], 100), (tm3[0], 100), tm3], C["gnd"], label="W2  GND -> top - rail", label_at=(470, 100))
    a17 = s.p("bb.a17")
    tm17 = s.p("bb.T-17")
    s.wire([a17, tm17], C["gnd"])
    s.tag(a17[0] + 60, (a17[1] + tm17[1]) / 2 - 4, "W3  a17 -> - rail", C["gnd"])

    pb = power_bank(s, 70, 600)
    usb = s.p("r4.USB")
    usb_cable(s, [pb, (pb[0], 575), (30, 575), (30, usb[1]), usb], "USB-C cable (data-capable)")

    y = table(s, 470, 520, ["#", "From", "To", "Wire"],
              [("W1", "R4 D8", "breadboard a10", ("tone signal", C["tone"])),
               ("W2", "R4 GND (top, by AREF)", "top - rail", ("ground", C["gnd"])),
               ("W3", "breadboard a17", "top - rail", ("ground", C["gnd"])),
               ("R1", "100 ohm leg 1: c10", "leg 2: c14", "brown-black-brown"),
               ("BZ1", "buzzer + : e14", "buzzer - : e17", "passive piezo"),
               ("USB", "power bank", "R4 USB-C", "programming: computer")],
              [44, 196, 150, 200], size=11, title_text="Wire list (breadboard columns 10, 14, 17 are examples; any free columns work)")
    note(s, 470, y + 12, 590, [
        "How it connects: holes a-e of one column are joined underneath, so W1 (a10) meets R1 (c10),",
        "R1 (c14) meets the buzzer + (e14), and the buzzer - (e17) meets W3 (a17). W2 and W3 share the - rail.",
        "W1 and W2 need a MALE end at the R4 (its headers are sockets) and a male end in the breadboard.",
    ], size=10)
    s.save(os.path.join(OUT, "phase1-wiring.svg"))


def phase1_schematic():
    s = SVG(1000, 560, "Phase 1 schematic", "D8 through R1 100 ohm to piezo BZ1 plus; BZ1 minus to GND. Power from USB-C.")
    title(s, "Phase 1 schematic", "Electrical view of the same circuit. Reference designators match the wire list.")
    # R4 block
    x, y = 170, 110
    s.rect(x, y, 230, 250, fill="#eef7f7", stroke=C["board_r4"], sw=2, rx=6)
    s.text(x + 115, y + 24, "U1  Arduino Uno R4 WiFi", size=13, anchor="middle", weight="bold")
    s.text(x + 115, y + 42, "RA4M1, 5 V logic", size=11, anchor="middle", fill=C["muted"])
    pins = [("D8", 120), ("GND", 200), ("USB-C VBUS", 90), ]
    s.line(x + 230, y + 120, x + 270, y + 120, C["ink"], 1.5)
    s.text(x + 222, y + 124, "D8", size=12, anchor="end", font=MONO)
    s.line(x + 230, y + 210, x + 270, y + 210, C["ink"], 1.5)
    s.text(x + 222, y + 214, "GND", size=12, anchor="end", font=MONO)
    s.line(x - 40, y + 90, x, y + 90, C["5v"], 1.5)
    s.text(x + 8, y + 94, "USB-C", size=12, font=MONO)
    s.text(x - 44, y + 80, "+5 V from", size=10, anchor="end", fill=C["muted"])
    s.text(x - 44, y + 93, "computer or", size=10, anchor="end", fill=C["muted"])
    s.text(x - 44, y + 106, "power bank", size=10, anchor="end", fill=C["muted"])
    s.text(x + 115, y + 160, "tone via FspTimer ISR", size=10, anchor="middle", fill=C["muted"])
    s.text(x + 115, y + 174, "(beeper.h, ~2.5 kHz)", size=10, anchor="middle", fill=C["muted"])
    # net D8 -> R1 -> BZ1
    ny = y + 120
    s.resistor_h(x + 270, x + 430, ny, "R1", "100 ohm 1/4 W")
    s.line(x + 430, ny, x + 520, ny, C["ink"], 1.5)
    # piezo symbol: two plates
    px = x + 520
    s.line(px, ny, px, ny + 30, C["ink"], 1.5)
    s.rect(px - 22, ny + 30, 44, 8, fill=C["ink"], stroke=C["ink"], sw=1)
    s.rect(px - 16, ny + 44, 32, 12, fill="white", stroke=C["ink"], sw=1.5)
    s.rect(px - 22, ny + 62, 44, 8, fill=C["ink"], stroke=C["ink"], sw=1)
    s.text(px - 34, ny + 42, "+", size=14, anchor="middle", weight="bold")
    s.text(px + 30, ny + 52, "BZ1  passive piezo", size=12, weight="bold")
    s.text(px + 30, ny + 67, "(not an 'active' buzzer)", size=10, fill=C["muted"])
    s.line(px, ny + 70, px, y + 210, C["ink"], 1.5)
    s.line(x + 270, y + 210, px, y + 210, C["ink"], 1.5)
    s.dot(px, y + 210)
    s.ground(px, y + 210)
    s.text(x + 350, ny - 30, "net TONE", size=10, fill=C["tone"], weight="bold")
    s.text(x + 350, y + 202, "net GND", size=10, weight="bold")
    note(s, 60, 400, 880, [
        "Why 100 ohm: a piezo is a small capacitor. Each time D8 flips, the resistor caps the charging spike at",
        "5 V / 100 ohm = 50 mA for a few microseconds; the average current is about 1 mA or less, inside the R4's 8 mA per-pin rating.",
        "Variant: a 3-pin passive buzzer module needs no resistor. Module S -> D8, module + -> 5V, module - -> GND.",
        "Do not drive a magnetic buzzer or speaker straight from D8; they need a transistor.",
    ], size=11)
    s.save(os.path.join(OUT, "phase1-schematic.svg"))


def rf_chain(s, ufl, x, y):
    """u.FL plug -> 1.13 mm coax -> RP-SMA female bulkhead -> Alfa APA-M04 (RP-SMA male). Bulkhead at (x, y)."""
    ux, uy = ufl
    s.poly([(ux, uy), (ux, y), (x, y)], stroke="#333", sw=6)
    s.poly([(ux, uy), (ux, y), (x, y)], stroke=C["rf"], sw=3.5)
    s.circle(ux, uy, 7, fill="#c9b24a", stroke="#333", sw=1.5)
    s.text(ux - 10, y - 30, "u.FL plug: press straight down", size=10, weight="bold", anchor="end")
    s.text(ux - 10, y - 17, "until it clicks (power OFF)", size=10, anchor="end")
    s.text((ux + x) / 2, y + 18, "pigtail, 1.13 mm coax", size=10, anchor="middle", fill=C["muted"])
    # RP-SMA female bulkhead (hex nut + threaded barrel)
    s.rect(x, y - 13, 18, 26, fill="#c0a24a", stroke="#6b5a20", sw=1.5, rx=2)
    s.rect(x + 18, y - 9, 30, 18, fill="#e0c870", stroke="#6b5a20", sw=1.5)
    for i in range(5):
        s.line(x + 22 + i * 6, y - 9, x + 22 + i * 6, y + 9, "#6b5a20", 1)
    s.text(x + 24, y + 30, "RP-SMA FEMALE", size=10, anchor="middle", weight="bold")
    s.text(x + 24, y + 43, "(pigtail end)", size=9, anchor="middle", fill=C["muted"])
    # antenna male plug + panel
    ax = x + 56
    s.rect(ax, y - 11, 26, 22, fill="#d7d7d7", stroke="#555", sw=1.5, rx=2)
    s.text(ax + 13, y - 18, "RP-SMA MALE", size=9, anchor="middle", weight="bold")
    s.line(ax + 26, y, ax + 50, y, "#222", 5)
    s.rect(ax + 50, y - 70, 26, 140, fill="#e8e8e8", stroke="#444", sw=2, rx=4)
    s.text(ax + 63, y + 88, "Alfa APA-M04", size=11, anchor="middle", weight="bold")
    s.text(ax + 63, y + 101, "2.4 GHz panel, 7 dBi", size=10, anchor="middle")
    s.text(ax + 63, y + 114, "(edge view)", size=9, anchor="middle", fill=C["muted"])
    for i in range(4):
        s.path(f"M {ax + 84 + i * 9} {y - 26 - i * 9} Q {ax + 98 + i * 11} {y} {ax + 84 + i * 9} {y + 26 + i * 9}", stroke="#888", sw=1.4)
    s.text(ax + 90, y - 78, "flat face = aiming side", size=10, anchor="start", fill=C["muted"])


# --------------------------------------------------------------------------------------------
# Phase 2
# --------------------------------------------------------------------------------------------
def phase2_wiring():
    s = SVG(1600, 1270, "Phase 2 wiring: R4, level shifter, XIAO ESP32-S3 and panel antenna",
            "Phase 1 buzzer wiring stays. R4 5V to breadboard bottom + rail; rail to shifter HV and XIAO 5V; R4 GND to shifter "
            "GND and XIAO GND; XIAO 3V3 to shifter LV; R4 D1 to HV1, LV1 to XIAO D7; XIAO D6 to LV2, HV2 to R4 D0; "
            "XIAO u.FL to pigtail to Alfa APA-M04.")
    title(s, "Phase 2 wiring: R4 + level shifter + XIAO ESP32-S3 + panel antenna",
          "Pin-to-pin. Phase 1 (W1-W3, R1, BZ1) stays exactly as built. Matches firmware/phase2-r4-display and firmware/phase2-xiao-sniffer.")
    boards.r4(s, 70, 170)
    boards.breadboard(s, 600, 190)
    boards.resistor_on_board(s, "bb.c10", "bb.c14")
    boards.piezo_on_board(s, "bb.e14", "bb.e17")
    boards.shifter(s, 760, 640)
    boards.xiao(s, 1100, 620)

    P = s.p
    # Phase 1 wires (unchanged)
    d8, a10 = P("r4.D8"), P("bb.a10")
    s.wire([d8, (d8[0], 128), (a10[0], 128), a10], C["tone"], label="W1", label_at=(600, 128))
    g = P("r4.GND_TOP")
    s.wire([g, (g[0], 100), (P("bb.T-3")[0], 100), P("bb.T-3")], C["gnd"], label="W2", label_at=(470, 100))
    s.wire([P("bb.a17"), P("bb.T-17")], C["gnd"])
    s.tag(P("bb.a17")[0] + 24, 228, "W3", C["gnd"])

    # Power
    v5 = P("r4.5V")
    bp1 = P("bb.B+1")
    s.wire([v5, (v5[0], 506), (566, 506), (566, bp1[1]), bp1], C["5v"], label="P1", label_at=(450, 506))
    hv = P("sh.HV")
    bp16 = P("bb.B+16")
    s.wire([bp16, (bp16[0], 600), (hv[0], 600), hv], C["5v"], label="P2", label_at=(bp16[0], 470))
    x5 = P("x.5V")
    bp28 = P("bb.B+28")
    s.wire([bp28, (bp28[0], 575), (1345, 575), (1345, x5[1]), x5], C["5v"], label="P3", label_at=(1180, 575))
    g1 = P("r4.GND1")
    shg = P("sh.GND_HV")
    s.wire([g1, (g1[0], 524), (shg[0], 524), shg], C["gnd"], label="P4", label_at=(700, 524))
    g2 = P("r4.GND2")
    xg = P("x.GND")
    s.wire([g2, (g2[0], 542), (1362, 542), (1362, xg[1]), xg], C["gnd"], label="P5", label_at=(1000, 542))
    x33 = P("x.3V3")
    lv = P("sh.LV")
    s.wire([x33, (1380, x33[1]), (1380, 930), (lv[0], 930), lv], C["3v3"], label="P6", label_at=(1000, 930))

    # Signals
    d1, d0 = P("r4.D1"), P("r4.D0")
    hv1, hv2 = P("sh.HV1"), P("sh.HV2")
    s.wire([d1, (d1[0], 150), (580, 150), (580, 612), (hv1[0], 612), hv1], C["r4tx"], label="S1", label_at=(580, 300), arrow=False)
    s.wire([d0, (d0[0], 164), (562, 164), (562, 590), (hv2[0], 590), hv2], C["xtx"], label="S4", label_at=(562, 380))
    lv1, lv2 = P("sh.LV1"), P("sh.LV2")
    x7, x6 = P("x.D7"), P("x.D6")
    s.wire([lv1, (lv1[0], 905), (1365, 905), (1365, x7[1]), x7], C["r4tx"], label="S2", label_at=(1000, 905))
    s.wire([x6, (1060, x6[1]), (1060, 880), (lv2[0], 880), lv2], C["xtx"], label="S3", label_at=(960, 880))

    rf_chain(s, P("x.UFL"), 1290, 990)

    pb = power_bank(s, 70, 640)
    usb = P("r4.USB")
    usb_cable(s, [pb, (pb[0], 610), (30, 610), (30, usb[1]), usb], "USB-C (computer or power bank)")
    note(s, 70, 760, 480, [
        "Never plug the XIAO's own USB-C in while P3 feeds",
        "its 5V pin. To reflash the XIAO in place: pull P3 first.",
        "Uploading to the R4 over USB is fine with everything",
        "connected (D0/D1 are not its USB port).",
    ], warn=True, size=11)

    rows = [
        ("W1-W3", "Phase 1 buzzer wiring", "unchanged", ("see Phase 1", C["tone"])),
        ("P1", "R4 5V (power header)", "breadboard bottom + rail", ("5 V", C["5v"])),
        ("P2", "bottom + rail", "shifter HV", ("5 V", C["5v"])),
        ("P3", "bottom + rail", "XIAO 5V", ("5 V", C["5v"])),
        ("P4", "R4 GND (power header)", "shifter GND, HV side", ("ground", C["gnd"])),
        ("P5", "R4 GND (power header)", "XIAO GND", ("ground", C["gnd"])),
        ("P6", "XIAO 3V3", "shifter LV", ("3.3 V", C["3v3"])),
        ("S1", "R4 D1 (TX)", "shifter HV1", ("R4 -> XIAO", C["r4tx"])),
        ("S2", "shifter LV1", "XIAO D7 (RX, GPIO44)", ("R4 -> XIAO", C["r4tx"])),
        ("S3", "XIAO D6 (TX, GPIO43)", "shifter LV2", ("XIAO -> R4", C["xtx"])),
        ("S4", "shifter HV2", "R4 D0 (RX)", ("XIAO -> R4", C["xtx"])),
        ("A1", "XIAO u.FL socket", "pigtail u.FL plug", ("coax", C["rf"])),
        ("A2", "pigtail RP-SMA female", "Alfa RP-SMA male", ("hand-tight", C["rf"])),
    ]
    yb = table(s, 70, 990, ["#", "From", "To", "Signal"], rows, [70, 220, 230, 150], size=11,
               title_text="Wire list (jumper type: M at the R4 or breadboard, F at the shifter and XIAO pins)")
    note(s, 790, 1110, 740, [
        "Shifter channels: 1 carries R4 TX to XIAO RX, 2 carries XIAO TX to R4 RX, 3 and 4 are spare",
        "(channel 3 is used later by the Phase 3 P4 -> R4 link).",
        "Adafruit 757 instead of the generic board: HV1 = B1, LV1 = A1, HV2 = B2, LV2 = A2, HV = HV, LV = LV.",
        "The shifter's two GND pins are joined on the board; one GND wire (P4) is enough.",
        "Labels on clones can sit in a different order. Always wire by the printed label, not by position.",
        "TX always goes to the other board's RX. Grounds of both boards are joined through P4 and P5.",
    ], size=11)
    s.save(os.path.join(OUT, "phase2-wiring.svg"))


# --------------------------------------------------------------------------------------------
# Schematic helpers
# --------------------------------------------------------------------------------------------
def block(s, key, x, y, w, h, name, sub, left=(), right=(), bottom=(), top=(), fill="#f4f7fb", stroke="#345"):
    """Schematic part box. left/right: [(pin, dy)], bottom/top: [(pin, dx)]. Pin end points saved as key.pin."""
    s.rect(x, y, w, h, fill=fill, stroke=stroke, sw=2, rx=6)
    s.text(x + w / 2, y + 22, name, size=13, anchor="middle", weight="bold")
    if sub:
        s.text(x + w / 2, y + 38, sub, size=10, anchor="middle", fill=C["muted"])
    for pin, dy in left:
        s.line(x - 30, y + dy, x, y + dy, C["ink"], 1.5)
        s.text(x + 6, y + dy + 4, pin, size=11, font=MONO)
        s.pins[f"{key}.{pin.split()[0]}"] = (x - 30, y + dy)
    for pin, dy in right:
        s.line(x + w, y + dy, x + w + 30, y + dy, C["ink"], 1.5)
        s.text(x + w - 6, y + dy + 4, pin, size=11, font=MONO, anchor="end")
        s.pins[f"{key}.{pin.split()[0]}"] = (x + w + 30, y + dy)
    for pin, dx in bottom:
        s.line(x + dx, y + h, x + dx, y + h + 30, C["ink"], 1.5)
        s.text(x + dx, y + h - 8, pin, size=11, font=MONO, anchor="middle")
        s.pins[f"{key}.{pin.split()[0]}"] = (x + dx, y + h + 30)
    for pin, dx in top:
        s.line(x + dx, y - 30, x + dx, y, C["ink"], 1.5)
        s.text(x + dx, y + 16 + (24 if sub else 0), pin, size=11, font=MONO, anchor="middle")
        s.pins[f"{key}.{pin.split()[0]}"] = (x + dx, y - 30)


def net(s, pts, color=C["ink"], name=None, at=None, sw=2):
    s.poly(pts, stroke=color, sw=sw)
    if name:
        x, y = at if at else pts[len(pts) // 2]
        s.text(x, y - 6, name, size=10, anchor="middle", fill=color if color not in (C["r4tx"], C["scl"]) else "#7a6100",
               weight="bold")


def piezo_sym(s, px, y_top, y_bot, label="BZ1", left=False):
    s.line(px, y_top, px, y_top + 20, C["ink"], 1.5)
    s.rect(px - 20, y_top + 20, 40, 7, fill=C["ink"], stroke=C["ink"], sw=1)
    s.rect(px - 14, y_top + 32, 28, 11, fill="white", stroke=C["ink"], sw=1.5)
    s.rect(px - 20, y_top + 48, 40, 7, fill=C["ink"], stroke=C["ink"], sw=1)
    s.line(px, y_top + 55, px, y_bot, C["ink"], 1.5)
    s.text(px - 30, y_top + 30, "+", size=13, anchor="middle", weight="bold")
    if left:
        s.text(px - 28, y_top + 76, label, size=11, weight="bold", anchor="end")
    else:
        s.text(px + 26, y_top + 42, label, size=11, weight="bold")


def antenna_sym(s, x, y, label):
    s.line(x, y, x, y - 40, C["ink"], 1.5)
    s.poly([(x - 16, y - 66), (x, y - 40), (x + 16, y - 66)], C["ink"], 1.5)
    s.line(x, y - 40, x, y - 66, C["ink"], 1.5)
    s.text(x + 22, y - 54, label, size=11, weight="bold")


def coax_sym(s, x1, y, x2, label):
    s.line(x1, y, x2, y, C["ink"], 1.5)
    s.path(f"M {x1 + 10} {y - 7} L {x2 - 10} {y - 7} M {x1 + 10} {y + 7} L {x2 - 10} {y + 7}", stroke=C["ink"], sw=1, dash="4 3")
    s.text((x1 + x2) / 2, y - 14, label, size=10, anchor="middle")


def mosfet_n(s, x, y, label="Q1 BSS138"):
    """N-channel enhancement MOSFET, gate on the left at (x, y). Drain at (x+40, y-40), source at (x+40, y+40)."""
    s.line(x, y, x + 22, y, C["ink"], 1.5)               # gate lead
    s.line(x + 22, y - 18, x + 22, y + 18, C["ink"], 2)  # gate plate
    for dy in (-16, 0, 16):                             # broken channel
        s.line(x + 30, y + dy - 6, x + 30, y + dy + 6, C["ink"], 2.5)
    s.line(x + 30, y - 16, x + 40, y - 16, C["ink"], 1.5)
    s.line(x + 40, y - 16, x + 40, y - 40, C["ink"], 1.5)   # drain
    s.line(x + 30, y + 16, x + 40, y + 16, C["ink"], 1.5)
    s.line(x + 40, y + 16, x + 40, y + 40, C["ink"], 1.5)   # source
    s.line(x + 30, y, x + 40, y, C["ink"], 1.5)
    s.line(x + 40, y, x + 40, y + 16, C["ink"], 1.5)        # body tied to source
    s.arrow_head(x + 31, y, -1, 0, C["ink"], 7)
    s.text(x + 50, y + 4, label, size=11, weight="bold")
    s.text(x + 50, y - 22, "D", size=10, fill=C["muted"])
    s.text(x + 50, y + 34, "S", size=10, fill=C["muted"])
    s.text(x - 4, y - 8, "G", size=10, fill=C["muted"], anchor="end")


def phase2_schematic():
    s = SVG(1460, 900, "Phase 2 schematic", "R4, BSS138 level shifter, XIAO ESP32-S3, buzzer and antenna as a schematic.")
    title(s, "Phase 2 schematic", "Electrical view of the Phase 2 wiring. Net colours match the wiring diagram. All grounds are one net.")
    block(s, "u1", 120, 110, 220, 440, "U1  Uno R4 WiFi", "5 V logic",
          right=[("5V", 60), ("D1 TX", 150), ("D0 RX", 220), ("D8", 320), ("GND", 410)])
    s.lines(140, 175, ["Serial1 = D0/D1", "115200 8N1", "", "USB-C: debug Serial", "and power in"], size=10, fill=C["muted"])
    block(s, "u2", 570, 150, 200, 300, "U2  Level shifter", "4-ch BSS138, ch 1-2 used",
          left=[("HV", 30), ("HV1", 110), ("HV2", 180), ("GND", 270)],
          right=[("LV", 30), ("LV1", 110), ("LV2", 180), ("GND", 270)])
    s.text(670, 245, "10k pull-ups on", size=10, anchor="middle", fill=C["muted"])
    s.text(670, 259, "every HV and LV pin", size=10, anchor="middle", fill=C["muted"])
    s.text(670, 280, "(one channel drawn in", size=10, anchor="middle", fill=C["muted"])
    s.text(670, 294, "level-shifter.svg)", size=10, anchor="middle", fill=C["muted"])
    block(s, "u3", 1000, 110, 230, 440, "U3  XIAO ESP32-S3", "3.3 V logic",
          left=[("5V", 60), ("3V3", 70 + 60), ("D7 RX", 190), ("D6 TX", 260), ("GND", 410)],
          bottom=[("ANT", 115)])
    s.lines(1100, 330, ["GPIO43 = D6 = TX", "GPIO44 = D7 = RX", "SW1 BOOT = GPIO0", "(on board: lock/", "next suspect)", "",
                        "On-board LDO makes", "3V3 from 5V"], size=10, fill=C["muted"])
    P = s.p
    # +5V
    a, hv, x5 = P("u1.5V"), P("u2.HV"), P("u3.5V")
    net(s, [a, (500, a[1]), (500, hv[1]), hv], C["5v"], "+5V", (430, a[1]))
    net(s, [(500, a[1]), (500, 85), (940, 85), (940, x5[1]), x5], C["5v"], "+5V (P1, P2, P3)", (720, 85))
    s.dot(500, a[1], C["5v"])
    # +3V3
    lv, x33 = P("u2.LV"), P("u3.3V3")
    net(s, [lv, (880, lv[1]), (880, x33[1]), x33], C["3v3"], "+3V3 (P6)", (945, x33[1]))
    # UART
    d1, hv1, lv1, x7 = P("u1.D1"), P("u2.HV1"), P("u2.LV1"), P("u3.D7")
    net(s, [d1, (470, d1[1]), (470, hv1[1]), hv1], C["r4tx"], "S1", (420, d1[1]))
    s.arrow_head(hv1[0] - 4, hv1[1], 1, 0, "#7a6100")
    net(s, [lv1, (900, lv1[1]), (900, x7[1]), x7], C["r4tx"], "S2", (860, lv1[1]))
    s.arrow_head(x7[0] - 4, x7[1], 1, 0, "#7a6100")
    d0, hv2, lv2, x6 = P("u1.D0"), P("u2.HV2"), P("u2.LV2"), P("u3.D6")
    net(s, [hv2, (450, hv2[1]), (450, d0[1]), d0], C["xtx"], "S4", (410, d0[1]))
    s.arrow_head(d0[0] + 4, d0[1], -1, 0, C["xtx"])
    net(s, [x6, (920, x6[1]), (920, lv2[1] + 0), lv2], C["xtx"], "S3", (960, x6[1]))
    s.arrow_head(lv2[0] + 4, lv2[1], -1, 0, C["xtx"])
    # GND bus
    gy = 640
    g1, g2l, g2r, g3 = P("u1.GND"), P("u2.GND"), P(f"u2.GND"), P("u3.GND")
    net(s, [g1, (400, g1[1]), (400, gy)], C["gnd"])
    gl = (570 - 30, 150 + 270)
    gr = (770 + 30, 150 + 270)
    net(s, [gl, (540, gy)], C["gnd"])
    net(s, [gr, (800, gy)], C["gnd"])
    net(s, [g3, (960, g3[1]), (960, gy)], C["gnd"])
    net(s, [(330, gy), (960, gy)], C["gnd"], "GND (W2, W3, P4, P5)", (700, gy))
    for xx in (400, 540, 800, 960):
        s.dot(xx, gy)
    s.ground(330, gy)
    s.line(330, gy - 0, 400, gy, C["gnd"], 2)
    # Buzzer
    d8 = P("u1.D8")
    s.resistor_h(d8[0], d8[0] + 110, d8[1], "R1", "100 ohm")
    piezo_sym(s, d8[0] + 130, d8[1], gy, "BZ1 passive piezo", left=True)
    s.line(d8[0] + 110, d8[1], d8[0] + 130, d8[1], C["ink"], 1.5)
    s.dot(d8[0] + 130, gy)
    net(s, [(d8[0] + 60, d8[1])], C["tone"], "TONE (W1)", (d8[0] + 60, d8[1] - 22))
    # Antenna
    ant = P("u3.ANT")
    coax_sym(s, ant[0], 700, ant[0] + 170, "J1: u.FL -> RP-SMA F pigtail")
    s.line(ant[0], ant[1], ant[0], 700, C["ink"], 1.5)
    antenna_sym(s, ant[0] + 170, 700, "ANT1 Alfa APA-M04")
    s.text(ant[0] + 192, 662 + 50, "2.4 GHz, 7 dBi panel", size=10, fill=C["muted"])
    note(s, 120, 760, 1120, [
        "Power: everything runs from the R4's USB-C. The R4 5V pin (about 4.7 V on USB) feeds the shifter's HV side and the XIAO's 5V input; the XIAO's",
        "own regulator makes 3.3 V, which also sets the shifter's LV side. Never feed the XIAO from its USB-C and the 5V pin at the same time.",
        "UART: 115200 8N1. The XIAO sends L / U / T / S lines (see firmware/phase2-r4-display). R4 -> XIAO (S1, S2) is wired but not used yet.",
    ], size=11)
    s.save(os.path.join(OUT, "phase2-schematic.svg"))


# --------------------------------------------------------------------------------------------
# Level shifter detail
# --------------------------------------------------------------------------------------------
def level_shifter():
    s = SVG(1300, 900, "Level shifter pinout and how one channel works",
            "Generic BSS138 4-channel board and Adafruit 757 pin names, the one-channel circuit, and which channel each phase uses.")
    title(s, "Level shifter: pin names, one channel, and channel use",
          "Generic BSS138 board (LV1-4 / HV1-4) and Adafruit 757 (A1-4 / B1-4) are the same circuit with different labels.")
    boards.shifter(s, 230, 110, title="Generic 4-ch|LV1-4 / HV1-4 labels")
    # Adafruit 757 drawn as two vertical columns
    x, y = 820, 100
    s.rect(x, y, 170, 200, fill="#1d1d4d", stroke="#000", sw=2, rx=8)
    s.header("ada", [("LV", "LV"), ("A1", "A1"), ("A2", "A2"), ("A3", "A3"), ("A4", "A4"), ("GND_L", "GND")], x + 18, y + 25, 28,
             horizontal=False, label_side="right", size=10)
    s.header("ada", [("HV", "HV"), ("B1", "B1"), ("B2", "B2"), ("B3", "B3"), ("B4", "B4"), ("GND_H", "GND")], x + 152, y + 25, 28,
             horizontal=False, label_side="left", size=10)
    s.text(x - 10, y + 100, "LOW side", size=10, anchor="end", fill="#c25f00", weight="bold")
    s.text(x + 180, y + 100, "HIGH side", size=10, fill=C["5v"], weight="bold")
    s.text(x + 85, y + 222, "Adafruit 757", size=12, anchor="middle", weight="bold")
    # Equivalence table
    yy = table(s, 70, 370, ["Generic", "Adafruit 757", "Side", "Connects to"],
               [("HV", "HV", "5 V", "R4 5V (via breadboard + rail)"),
                ("LV", "LV", "3.3 V", "XIAO 3V3 (Phase 2) / P4 pin 18 (Phase 3)"),
                ("GND (x2)", "GND (x2)", "both", "ground; the two GND pins are joined"),
                ("HV1 / LV1", "B1 / A1", "ch 1", "R4 D1 TX  ->  XIAO D7 RX"),
                ("HV2 / LV2", "B2 / A2", "ch 2", "R4 D0 RX  <-  XIAO D6 TX"),
                ("HV3 / LV3", "B3 / A3", "ch 3", "Phase 3 optional: R4 D0 RX  <-  P4 pin 8 GPIO2"),
                ("HV4 / LV4", "B4 / A4", "ch 4", "spare")],
               [110, 120, 70, 360], size=11, title_text="Same pin, two names")
    # One-channel schematic
    ox, oy = 790, 400
    s.text(ox, oy - 10, "Inside one channel (x4 on the board)", size=13, weight="bold")
    s.line(ox + 20, oy + 30, ox + 440, oy + 30, C["3v3"], 2)
    s.text(ox + 20, oy + 22, "LV (3.3 V)", size=11, weight="bold", fill="#c25f00")
    s.line(ox + 20, oy + 70, ox + 440, oy + 70, C["5v"], 2)
    s.text(ox + 330, oy + 62, "HV (5 V)", size=11, weight="bold", fill=C["5v"])
    # gate to LV
    gx, gy = ox + 160, oy + 230
    mosfet_n(s, gx, gy)
    s.line(gx, gy, gx - 40, gy, C["ink"], 1.5)
    s.line(gx - 40, gy, gx - 40, oy + 30, C["ink"], 1.5)
    s.dot(gx - 40, oy + 30, C["3v3"])
    # source -> LVn, with pull-up to LV
    sx, sy = gx + 40, gy + 40
    s.line(sx, sy, sx, sy + 30, C["ink"], 1.5)
    s.line(sx, sy + 30, ox + 60, sy + 30, C["ink"], 1.5)
    s.text(ox + 30, sy + 34, "LVn", size=12, font=MONO, weight="bold", anchor="middle")
    s.resistor_v(ox + 100, oy + 30, sy + 30, "R", "10k")
    s.dot(ox + 100, oy + 30, C["3v3"])
    s.dot(ox + 100, sy + 30)
    # drain -> HVn, with pull-up to HV
    dx, dy = gx + 40, gy - 40
    s.line(dx, dy, dx, oy + 160, C["ink"], 1.5)
    s.line(dx, oy + 160, ox + 420, oy + 160, C["ink"], 1.5)
    s.text(ox + 440, oy + 164, "HVn", size=12, font=MONO, weight="bold", anchor="start")
    s.resistor_v(ox + 360, oy + 70, oy + 160, "R", "10k")
    s.dot(ox + 360, oy + 70, C["5v"])
    s.dot(ox + 360, oy + 160)
    note(s, 70, 560, 680, [
        "How it works (both directions, no direction pin):",
        "LV side pulls low: the MOSFET's source drops, it turns on and pulls HVn low too.",
        "HV side pulls low: the body diode drags LVn low, the MOSFET turns on and finishes the job.",
        "Nobody pulls low: both sides float up to their own rail through the 10k resistors.",
        "So a 5 V high never reaches the 3.3 V side, and a 3.3 V high is lifted to 5 V.",
        "Speed: fine for UART at 115200 and I2C at 400 kHz; do not push UART much higher.",
        "Buying a clone: look for four 3-pin parts marked J1Y (BSS138). Boards built on a",
        "TXS0108 or TXB0104 chip are a different design and are not recommended here.",
    ], size=11)
    note(s, 790, 720, 470, [
        "Power both sides before sending data:",
        "HV pin to 5 V and LV pin to 3.3 V, GND to ground.",
        "LV must be the lower voltage (1.8 V min).",
        "With LV unpowered, nothing reaches the 3.3 V board.",
    ], warn=True, size=11)
    s.save(os.path.join(OUT, "level-shifter.svg"))


# --------------------------------------------------------------------------------------------
# Phase 3
# --------------------------------------------------------------------------------------------
def phase3_wiring():
    s = SVG(1460, 1360, "Phase 3 wiring: ESP32-P4 board, XIAO ESP32-S3, BNO085, OV5647 and LiPo",
            "P4 header pin 1 5V to XIAO 5V, pin 5 GND to XIAO GND, XIAO D6 to pin 11 GPIO5, pin 14 GPIO4 to XIAO D7; "
            "BNO085 VIN to pin 2, SDA to pin 4, SCL to pin 6, GND to pin 10; optional R4 beeper through shifter channel 3.")
    title(s, "Phase 3 wiring: ESP32-P4 4.3\" board + XIAO + BNO085 IMU + camera (planned, firmware not written)",
          "Pin-to-pin. P4, XIAO and BNO085 are all 3.3 V logic: direct wires, no shifter. Header numbering is Waveshare's (pin 1 = 5V), NOT Raspberry Pi's.")
    # P4 board (rear view)
    bx, by, bw, bh = 520, 110, 520, 640
    s.rect(bx, by, bw, bh, fill=C["board_p4"], stroke="#0d1c2e", sw=2, rx=14)
    s.text(bx + 20, by + 28, "Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3", size=13, fill="white", weight="bold")
    s.text(bx + 20, by + 45, "rear view (screen faces away), 3.3 V logic", size=10, fill="#cfe")
    used = (1, 2, 4, 5, 6, 8, 10, 11, 13, 14, 18)
    hx, hy = bx + 170, by + 100
    boards.p4_header(s, hx, hy, used=used)
    s.text(hx + 85, hy - 22, "40-pin header", size=11, anchor="middle", fill="white", weight="bold")
    s.text(bx + 20, by + 60, "header rows drawn spread apart for the labels (real pitch 2.54 mm); pin 1 = square pad", size=9, fill="#cfe")
    # connectors on the P4
    s.rect(bx + 30, by + 600, 120, 22, fill="#e8e0c8", stroke="#776", sw=1.5, rx=2)
    s.text(bx + 90, by + 615, "CSI (15-pin)", size=10, anchor="middle", weight="bold")
    s.rect(bx + 400, by + 600, 46, 22, fill="#fafafa", stroke="#776", sw=1.5, rx=2)
    s.text(bx + 423, by + 615, "J2", size=10, anchor="middle", weight="bold")
    s.text(bx + 392, by + 608, "MX1.25 battery", size=9, fill="#cfe", anchor="end")
    s.text(bx + 392, by + 620, "pin 1 BAT+, pin 2 GND", size=9, fill="#cfe", anchor="end")
    s.rect(bx + 210, by + bh - 16, 64, 30, fill="#c0c0c0", stroke="#666", sw=1.5, rx=6)
    s.text(bx + 282, by + bh - 4, "USB-C: flash, console, charge", size=9, fill="#cfe")
    s.rect(bx + bw - 14, by + 500, 22, 40, fill="#888", stroke="#444", sw=1, rx=3)
    s.text(bx + bw - 20, by + 516, "POWER", size=9, anchor="end", fill="white")
    s.text(bx + bw - 20, by + 528, "(short press)", size=9, anchor="end", fill="#cfe")

    boards.xiao(s, 150, 140)
    boards.bno085(s, 1170, 160)
    P = s.p
    # XIAO <-> P4 (odd pins leave to the left)
    p1, x5 = P("p4.1"), P("x.5V")
    s.wire([p1, (610, p1[1]), (610, x5[1]), x5], C["5v"], label="T1", label_at=(450, x5[1]))
    p5, xg = P("p4.5"), P("x.GND")
    s.wire([p5, (595, p5[1]), (595, xg[1]), xg], C["gnd"], label="T2", label_at=(450, xg[1]))
    p11, x6 = P("p4.11"), P("x.D6")
    s.wire([x6, (100, x6[1]), (100, 430), (580, 430), (580, p11[1]), p11], C["xtx"], label="T3", label_at=(300, 430))
    p14, x7 = P("p4.14"), P("x.D7")
    s.wire([p14, (1000, p14[1]), (1000, 692), (560, 692), (560, x7[1]), x7], C["p4tx"], label="T4", label_at=(1000, 560))
    # BNO085 (even pins leave to the right)
    for pin, key, color, lab, lane in ((2, "VIN", C["3v3"], "I1", 1120), (4, "SDA", C["sda"], "I2", 1105),
                                       (6, "SCL", C["scl"], "I3", 1090), (10, "GND", C["gnd"], "I4", 1075)):
        a, b = P(f"p4.{pin}"), P(f"bno.{key}")
        s.wire([a, (lane, a[1]), (lane, b[1]), b], color, label=lab, label_at=(1140 if lab != "I4" else 1140, b[1]))
    s.text(1245, 360, "Or use a STEMMA QT to female-socket", size=10, anchor="middle")
    s.text(1245, 374, "cable: red VIN, black GND,", size=10, anchor="middle")
    s.text(1245, 388, "blue SDA, yellow SCL", size=10, anchor="middle")
    # Camera + battery
    s.rect(60, 560, 150, 120, fill="#2e7d32", stroke="#1b5e20", sw=2, rx=6)
    s.circle(135, 605, 22, fill="#111", stroke="#444", sw=3)
    s.text(135, 662, "OV5647 camera", size=11, anchor="middle", fill="white", weight="bold")
    s.poly([(210, 620), (bx + 90, 620), (bx + 90, by + 600)], stroke="#c9a96e", sw=10)
    s.text(230, 610, "15-pin 0.5 mm ribbon (NOT the 1.0 mm RPi one)", size=10)
    s.text(230, 642, "contacts face the connector's contacts", size=10, fill=C["muted"])
    s.rect(60, 790, 150, 70, fill="#5b6f8a", stroke="#334", sw=2, rx=10)
    s.text(135, 820, "3.7 V LiPo", size=12, anchor="middle", fill="white", weight="bold")
    s.text(135, 838, "MX1.25 plug", size=10, anchor="middle", fill="#dde")
    jx, jy = bx + 412, by + 622
    s.wire([(210, 812), (jx, 812), (jx, jy)], C["bat"], sw=3)
    s.wire([(210, 830), (jx + 22, 830), (jx + 22, jy)], C["gnd"], sw=3)
    s.lines(60, 884, ["Red = + to J2 pin 1, black = GND to pin 2.", "CHECK with a meter before plugging in:",
                      "cheap cells come wired both ways."], size=10, fill=C["5v"], weight="bold")

    # Optional R4 beeper link (dashed box)
    oy = 930
    s.rect(500, oy, 920, 200, fill="none", stroke="#888", sw=1.5, rx=8, dash="8 5")
    s.text(515, oy + 22, "OPTIONAL: keep the R4 as a beeper (P4 -> R4 only, through channel 3 of the Phase 2 level shifter)", size=12, weight="bold")
    boards.shifter(s, 760, oy + 50, title="")
    s.rect(1150, oy + 40, 250, 140, fill=C["board_r4"], stroke="#0a5154", sw=2, rx=10)
    s.text(1275, oy + 64, "Uno R4 WiFi (5 V)", size=12, anchor="middle", fill="white", weight="bold")
    s.header("r4b", [("D0", "D0 RX"), ("5V", "5V"), ("GND", "GND")], 1170, oy + 90, 30, horizontal=False, label_side="right",
             female=True, size=10)
    s.text(1240, oy + 170, "runs phase2-r4-display", size=9, anchor="start", fill="#cde")
    p8, p13, p18 = P("p4.8"), P("p4.13"), P("p4.18")
    lv3, lvp, lvg = P("sh.LV3"), P("sh.LV"), P("sh.GND_LV")
    hv3, hvp, hvg = P("sh.HV3"), P("sh.HV"), P("sh.GND_HV")
    s.wire([p8, (1060, p8[1]), (1060, 905), (740, 905), (740, oy + 190), (lv3[0], oy + 190), lv3], C["p4r4"],
           label="O1", label_at=(900, 905), dash="7 4")
    s.wire([p18, (1045, p18[1]), (1045, 890), (725, 890), (725, oy + 178), (lvp[0], oy + 178), lvp], C["3v3"],
           label="O2", label_at=(900, 890), dash="7 4")
    s.wire([p13, (540, p13[1]), (540, oy + 166), (lvg[0], oy + 166), lvg], C["gnd"], label="O3", label_at=(540, 880), dash="7 4")
    r4d0, r45, r4g = P("r4b.D0"), P("r4b.5V"), P("r4b.GND")
    s.wire([hv3, (hv3[0], oy + 40), (1120, oy + 40), (1120, r4d0[1]), r4d0], C["p4r4"], label="O4", label_at=(1030, oy + 40), dash="7 4")
    s.wire([hvp, (hvp[0], oy + 30), (1105, oy + 30), (1105, r45[1]), r45], C["5v"], label="O5", label_at=(1105, oy + 90), dash="7 4")
    s.wire([hvg, (hvg[0], oy + 46), (1090, oy + 46), (1090, r4g[1]), r4g], C["gnd"], label="O6", label_at=(1090, oy + 160), dash="7 4")

    rows = [("T1", "P4 pin 1 (5V)", "XIAO 5V", ("5 V", C["5v"])),
            ("T2", "P4 pin 5 (GND)", "XIAO GND", ("ground", C["gnd"])),
            ("T3", "XIAO D6 (TX, GPIO43)", "P4 pin 11 (GPIO5, P4 RX)", ("XIAO -> P4", C["xtx"])),
            ("T4", "P4 pin 14 (GPIO4, P4 TX)", "XIAO D7 (RX, GPIO44)", ("P4 -> XIAO", C["p4tx"])),
            ("I1", "P4 pin 2 (3V3)", "BNO085 VIN", ("3.3 V", C["3v3"])),
            ("I2", "P4 pin 4 (GPIO7 SDA)", "BNO085 SDA", ("I2C data", C["sda"])),
            ("I3", "P4 pin 6 (GPIO8 SCL)", "BNO085 SCL", ("I2C clock", C["scl"])),
            ("I4", "P4 pin 10 (GND)", "BNO085 GND", ("ground", C["gnd"])),
            ("O1", "P4 pin 8 (GPIO2, TX)", "shifter LV3", ("optional", C["p4r4"])),
            ("O2", "P4 pin 18 (3V3)", "shifter LV", ("optional", C["3v3"])),
            ("O3", "P4 pin 13 (GND)", "shifter GND (LV side)", ("optional", C["gnd"])),
            ("O4", "shifter HV3", "R4 D0 (RX)", ("optional", C["p4r4"])),
            ("O5", "R4 5V", "shifter HV", ("optional", C["5v"])),
            ("O6", "R4 GND", "shifter GND (HV side)", ("optional", C["gnd"]))]
    table(s, 40, 1165, ["#", "From", "To", "Signal"], rows[:8], [44, 180, 190, 115], size=10,
          title_text="Wire list (F-F jumpers; O4-O6 need M ends at the R4)")
    table(s, 600, 1165, ["#", "From", "To", "Signal"], rows[8:], [44, 180, 190, 115], size=10)
    note(s, 1150, 1155, 300, [
        "Remove S1-S4 and P1-P6 from",
        "Phase 2 first: in Phase 3 the",
        "XIAO talks to the P4, not the R4.",
        "Leave pins 7, 9 (P4 console UART)",
        "and 25, 27 (USB, no GPIO) free.",
        "XIAO USB-C stays unplugged",
        "while T1 powers it.",
    ], warn=True, size=10)
    s.save(os.path.join(OUT, "phase3-wiring.svg"))


def phase3_schematic():
    s = SVG(1300, 860, "Phase 3 schematic", "P4, XIAO, BNO085, camera, battery and the optional R4 link as a schematic.")
    title(s, "Phase 3 schematic (planned)", "All logic on this page is 3.3 V except the optional R4 link, which goes through shifter channel 3.")
    block(s, "p4", 470, 120, 280, 520, "U4  ESP32-P4 board", "Waveshare 4.3\", header pins shown",
          left=[("1 5V", 70), ("5 GND", 110), ("11 GPIO5 RX", 170), ("14 GPIO4 TX", 220)],
          right=[("2 3V3", 70), ("4 GPIO7 SDA", 130), ("6 GPIO8 SCL", 180), ("10 GND", 230)],
          bottom=[("8 GPIO2", 60), ("18 3V3", 140), ("13 GND", 220)])
    s.lines(490, 420, ["CSI: OV5647 (MIPI 2-lane)", "J2: LiPo 3.7 V (pin 1 BAT+)", "USB-C: flash, console, charge",
                       "I2C bus GPIO7/8 is shared with", "touch (GT911), audio codec,", "camera control"], size=10, fill=C["muted"])
    block(s, "u3", 80, 120, 200, 300, "U3  XIAO ESP32-S3", "3.3 V, monitor mode",
          right=[("5V", 70), ("GND", 110), ("D7 RX", 170), ("D6 TX", 220)], bottom=[("ANT", 100)])
    block(s, "u5", 980, 120, 200, 220, "U5  BNO085", "I2C 0x4A",
          left=[("VIN", 70), ("SDA", 130), ("SCL", 180)], bottom=[("GND", 100)])
    P = s.p
    net(s, [P("u3.5V"), P("p4.1")], C["5v"], "+5V (T1)")
    net(s, [P("u3.GND"), P("p4.5")], C["gnd"], "GND (T2)")
    # TX -> RX crossover: XIAO D6 (TX) to pin 11 (P4 RX), pin 14 (P4 TX) to XIAO D7 (RX)
    a, b = P("u3.D7"), P("u3.D6")
    c, d = P("p4.11"), P("p4.14")
    net(s, [a, (340, a[1]), (410, d[1]), d], C["p4tx"])
    net(s, [b, (340, b[1]), (410, c[1]), c], C["xtx"])
    s.text(375, 268, "TX -> RX crossover", size=10, anchor="middle", fill=C["muted"])
    s.text(375, 380, "T3 (green): XIAO D6 TX -> P4 GPIO5 RX", size=10, anchor="middle", fill=C["xtx"])
    s.text(375, 394, "T4 (cyan): P4 GPIO4 TX -> XIAO D7 RX", size=10, anchor="middle", fill="#0f7f8a")
    net(s, [P("p4.2"), (900, P("p4.2")[1]), (900, P("u5.VIN")[1]), P("u5.VIN")], C["3v3"], "+3V3 (I1)", (860, P("p4.2")[1]))
    net(s, [P("p4.4"), P("u5.SDA")], C["sda"], "SDA (I2)")
    net(s, [P("p4.6"), P("u5.SCL")], C["scl"], "SCL (I3)")
    g = P("u5.GND")
    net(s, [P("p4.10"), (860, P("p4.10")[1]), (860, 410), (g[0], 410), g], C["gnd"], "GND (I4)", (960, 410))
    s.lines(1000, 460, ["Board pull-ups: the BNO085 breakout", "has 10k on SDA/SCL; the P4 bus has", "its own. No extra resistors."], size=10, fill=C["muted"])
    ant = P("u3.ANT")
    coax_sym(s, ant[0], 500, ant[0] + 150, "pigtail")
    s.line(ant[0], ant[1], ant[0], 500, C["ink"], 1.5)
    antenna_sym(s, ant[0] + 150, 500, "ANT1 APA-M04")
    # optional R4 link
    oy = 720
    s.rect(60, oy - 20, 1180, 130, fill="none", stroke="#888", sw=1.5, rx=8, dash="8 5")
    s.text(75, oy, "Optional R4 beeper link", size=12, weight="bold")
    block(s, "u2", 560, oy + 10, 160, 80, "", "",
          top=[("LV3", 30), ("LV", 80), ("GND", 130)])
    s.text(640, oy + 72, "U2 shifter, ch 3", size=12, anchor="middle", weight="bold")
    p8, p18, p13 = P("p4.8"), P("p4.18"), P("p4.13")
    lv, lv3, lg = P("u2.LV"), P("u2.LV3"), P("u2.GND")
    net(s, [p18, (p18[0], 680), (lv[0], 680), lv], C["3v3"], sw=1.6)
    net(s, [p8, (p8[0], 690), (lv3[0], 690), lv3], C["p4r4"], sw=1.6)
    net(s, [p13, (p13[0], 700), (lg[0], 700), lg], C["gnd"], sw=1.6)
    s.line(720, oy + 50, 900, oy + 50, C["p4r4"], 2)
    s.text(810, oy + 44, "HV3 -> R4 D0 RX", size=10, anchor="middle", fill=C["p4r4"], weight="bold")
    s.rect(900, oy + 20, 200, 70, fill="#eef7f7", stroke=C["board_r4"], sw=2, rx=6)
    s.text(1000, oy + 46, "U1 Uno R4 WiFi", size=12, anchor="middle", weight="bold")
    s.text(1000, oy + 64, "HV = R4 5V, GND = R4 GND", size=10, anchor="middle", fill=C["muted"])
    s.save(os.path.join(OUT, "phase3-schematic.svg"))


# --------------------------------------------------------------------------------------------
# System overview + power
# --------------------------------------------------------------------------------------------
def overview():
    s = SVG(1300, 760, "System overview and power", "How the boards connect in each phase, signal levels, and where each board gets power.")
    title(s, "System overview: what talks to what, at which voltage", "Solid = built in that phase. Dashed = optional. Red text = 5 V side, orange = 3.3 V side.")

    def box(x, y, w, h, t, lines, fill, phase):
        s.rect(x, y, w, h, fill=fill, stroke="#223", sw=2, rx=10)
        s.text(x + 12, y + 22, t, size=13, weight="bold", fill="white")
        s.lines(x + 12, y + 42, lines, size=10, fill="#e8eef5")
        s.rect(x + w - 66, y - 12, 66, 22, fill="white", stroke="#223", sw=1.5, rx=11)
        s.text(x + w - 33, y + 4, phase, size=10, anchor="middle", weight="bold")

    box(60, 110, 280, 150, "Uno R4 WiFi  (5 V)", ["Phase 1: scans Wi-Fi itself, beeps", "Phase 2: beeper + LED bars for the XIAO",
                                                  "Serial1 on D0/D1, buzzer on D8", "Power: USB-C (computer or power bank)"], C["board_r4"], "Phase 1")
    box(60, 330, 280, 90, "Buzzer + 100 ohm", ["passive piezo on D8", "beeps faster as signal rises"], "#5a3d7a", "Phase 1")
    box(500, 110, 260, 150, "Level shifter (BSS138)", ["5 V <-> 3.3 V, both directions", "ch 1: R4 TX -> XIAO RX", "ch 2: XIAO TX -> R4 RX",
                                                       "ch 3: P4 TX -> R4 RX (opt.)"], C["board_shift"], "Phase 2")
    box(920, 110, 300, 150, "XIAO ESP32-S3  (3.3 V)", ["monitor mode on ch 1-11, receive only", "maker table tags CAM / IOT",
                                                       "UART TX D6, RX D7", "Power: 5V pin from R4 (Ph 2) or P4 (Ph 3)"], C["board_xiao"], "Phase 2")
    box(920, 330, 300, 90, "Alfa APA-M04 panel", ["7 dBi, aim the flat face", "u.FL -> RP-SMA F pigtail"], "#666", "Phase 2")
    box(500, 470, 300, 170, "ESP32-P4 4.3\" board  (3.3 V)", ["camera + LVGL heat map + crosshair", "UART to XIAO: GPIO5 RX / GPIO4 TX",
                                                              "I2C GPIO7/8 to BNO085", "Power: USB-C or 3.7 V LiPo (J2)",
                                                              "header 5V feeds the XIAO"], C["board_p4"], "Phase 3")
    box(920, 520, 300, 70, "BNO085 IMU", ["I2C 0x4A on pins 2, 4, 6, 10"], C["board_bno"], "Phase 3")
    box(60, 520, 280, 70, "OV5647 camera", ["CSI, 15-pin 0.5 mm ribbon"], "#2e7d32", "Phase 3")

    def link(pts, color, text, at, dash=None, both=False):
        s.poly(pts, stroke=color if color not in (C["r4tx"],) else "#c9a000", sw=3, dash=dash)
        (x1, y1), (x2, y2) = pts[-2], pts[-1]
        L = ((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5
        s.arrow_head(x2, y2, (x2 - x1) / L, (y2 - y1) / L, color, 10)
        if both:
            (x1, y1), (x2, y2) = pts[1], pts[0]
            L = ((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5
            s.arrow_head(x2, y2, (x2 - x1) / L, (y2 - y1) / L, color, 10)
        s.text(at[0], at[1], text, size=10, anchor="middle", weight="bold")

    link([(340, 185), (500, 185)], C["5v"], "UART 5 V", (420, 177), both=True)
    link([(760, 185), (920, 185)], "#c25f00", "UART 3.3 V", (840, 177), both=True)
    link([(200, 260), (200, 330)], C["tone"], "D8 tone", (240, 300))
    link([(1070, 260), (1070, 330)], C["rf"], "coax", (1100, 300))
    link([(1000, 260), (1000, 290), (760, 290), (760, 470)], "#c25f00", "UART 3.3 V direct (Phase 3)", (880, 284), both=True)
    link([(800, 555), (920, 555)], C["sda"], "I2C", (860, 547), both=True)
    link([(340, 555), (500, 555)], "#2e7d32", "CSI", (420, 547))
    link([(560, 470), (560, 260)], C["p4r4"], "opt. P4 -> R4", (500, 380), dash="8 5")

    note(s, 60, 670, 1160, [
        "Voltage rule: the R4 is the only 5 V logic board. Every R4 <-> ESP32 signal goes through the shifter; XIAO <-> P4 and BNO085 <-> P4 are 3.3 V on both ends.",
        "Grounds: every wire bundle between two boards carries a ground. One power source per board: never the XIAO's USB-C and its 5V pin together.",
    ], warn=True, size=11)
    s.save(os.path.join(OUT, "system-overview.svg"))


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    which = sys.argv[1:] or None
    for fn in [overview, phase1_wiring, phase1_schematic, phase2_wiring, phase2_schematic, level_shifter,
               phase3_wiring, phase3_schematic]:
        if which is None or fn.__name__ in which:
            fn()
            print("wrote", fn.__name__)
