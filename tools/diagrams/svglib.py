"""Tiny SVG drawing helpers for the Hidden Device Hunter manual diagrams.

No dependencies. Every diagram is plain SVG with a white background and system fonts,
so it renders the same on GitHub (light or dark theme), in a browser, or printed.
"""
from html import escape

SANS = "Helvetica, Arial, sans-serif"
MONO = "Menlo, Consolas, 'DejaVu Sans Mono', monospace"

# Wire colours. Same meaning in every diagram.
C = {
    "5v": "#d62728",      # red      5 V power
    "3v3": "#ff7f0e",     # orange   3.3 V power
    "gnd": "#1a1a1a",     # black    ground
    "bat": "#c51b7d",     # magenta  LiPo +
    "tone": "#9467bd",    # purple   buzzer tone signal
    "r4tx": "#e6b800",    # yellow   R4 TX -> XIAO RX (shifter ch 1)
    "xtx": "#2ca02c",     # green    XIAO TX -> R4 / P4 RX
    "p4tx": "#17becf",    # cyan     P4 TX -> XIAO RX
    "p4r4": "#8c564b",    # brown    P4 TX -> R4 RX (optional, shifter ch 3)
    "sda": "#1f77b4",     # blue     I2C SDA (STEMMA QT colour)
    "scl": "#e6b800",     # yellow   I2C SCL (STEMMA QT colour)
    "rf": "#6b6b6b",      # grey     coax
    "board_r4": "#0f7c80",
    "board_xiao": "#2b2b2b",
    "board_shift": "#1d4f91",
    "board_p4": "#1f3a5f",
    "board_bno": "#2d2d6b",
    "pin": "#d4af37",
    "ink": "#111111",
    "muted": "#555555",
    "note": "#fff7d6",
    "warn": "#fde2e1",
}

# Light wires get a dark outline so they show on white.
OUTLINED = {C["r4tx"], C["scl"], C["3v3"], C["p4tx"]}


class SVG:
    def __init__(self, w, h, title, desc=""):
        self.w, self.h = w, h
        self.title, self.desc = title, desc
        self.body = []
        self.pins = {}

    # ---------- primitives ----------
    def add(self, s):
        self.body.append(s)

    def rect(self, x, y, w, h, fill="none", stroke=C["ink"], sw=1.5, rx=0, dash=None, opacity=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        o = f' fill-opacity="{opacity}"' if opacity is not None else ""
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}"{o} stroke="{stroke}" stroke-width="{sw}"{d}/>')

    def circle(self, x, y, r, fill="none", stroke=C["ink"], sw=1.5):
        self.add(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')

    def line(self, x1, y1, x2, y2, stroke=C["ink"], sw=1.5, dash=None, cap="round"):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{stroke}" stroke-width="{sw}" stroke-linecap="{cap}"{d}/>')

    def path(self, d, stroke=C["ink"], sw=1.5, fill="none", dash=None, join="round"):
        ds = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<path d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" stroke-linejoin="{join}" stroke-linecap="round"{ds}/>')

    def poly(self, pts, stroke=C["ink"], sw=1.5, fill="none", dash=None):
        d = "M " + " L ".join(f"{x} {y}" for x, y in pts)
        self.path(d, stroke, sw, fill, dash)

    def text(self, x, y, s, size=12, anchor="start", weight="normal", fill=C["ink"], font=SANS, rotate=None, italic=False):
        t = f' transform="rotate({rotate} {x} {y})"' if rotate is not None else ""
        st = ' font-style="italic"' if italic else ""
        self.add(f'<text x="{x}" y="{y}" font-family="{font}" font-size="{size}" text-anchor="{anchor}" '
                 f'font-weight="{weight}" fill="{fill}"{st}{t}>{escape(str(s))}</text>')

    def lines(self, x, y, rows, size=12, gap=None, **kw):
        gap = gap or size * 1.35
        for i, r in enumerate(rows):
            self.text(x, y + i * gap, r, size=size, **kw)

    def arrow_head(self, x, y, dx, dy, color, size=8):
        # dx, dy = unit direction the arrow points
        px, py = -dy, dx
        a = (x, y)
        b = (x - dx * size + px * size * 0.5, y - dy * size + py * size * 0.5)
        c = (x - dx * size - px * size * 0.5, y - dy * size - py * size * 0.5)
        self.add(f'<path d="M {a[0]} {a[1]} L {b[0]} {b[1]} L {c[0]} {c[1]} Z" fill="{color}" stroke="none"/>')

    # ---------- wires ----------
    def wire(self, pts, color, sw=3.2, label=None, label_at=None, arrow=False, dash=None):
        """Orthogonal (or any) polyline wire with round end dots. pts = [(x,y), ...]."""
        if color in OUTLINED:
            self.poly(pts, stroke="#333", sw=sw + 1.8, dash=dash)
        self.poly(pts, stroke=color, sw=sw, dash=dash)
        for (x, y) in (pts[0], pts[-1]):
            self.circle(x, y, sw * 0.9, fill=color, stroke="#333" if color in OUTLINED else color, sw=1)
        if arrow:
            (x1, y1), (x2, y2) = pts[-2], pts[-1]
            L = ((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5 or 1
            dx, dy = (x2 - x1) / L, (y2 - y1) / L
            mx, my = (x1 + x2) / 2, (y1 + y2) / 2
            self.arrow_head(mx + dx * 5, my + dy * 5, dx, dy, color if color not in OUTLINED else "#333", 9)
        if label:
            lx, ly = label_at if label_at else pts[len(pts) // 2]
            self.tag(lx, ly, label, color)

    def tag(self, x, y, s, color, size=10):
        w = len(s) * size * 0.62 + 8
        self.rect(x - w / 2, y - size * 0.8, w, size * 1.5, fill="white", stroke=color, sw=1.2, rx=3)
        self.text(x, y + size * 0.35, s, size=size, anchor="middle", font=MONO, weight="bold")

    def dot(self, x, y, color=C["ink"], r=3.5):
        self.circle(x, y, r, fill=color, stroke=color, sw=1)

    # ---------- pins and headers ----------
    def pin(self, name, x, y, female=False, color=None):
        """Register a pin location and draw it. female = socket (R4 headers), else male pin."""
        self.pins[name] = (x, y)
        if female:
            self.rect(x - 5, y - 5, 10, 10, fill="#111", stroke="#444", sw=1, rx=1)
            self.rect(x - 2.2, y - 2.2, 4.4, 4.4, fill="#555", stroke="none", sw=0)
        else:
            self.rect(x - 5, y - 5, 10, 10, fill="#222", stroke="#000", sw=1, rx=1)
            self.rect(x - 2.5, y - 2.5, 5, 5, fill=color or C["pin"], stroke="none", sw=0)

    def header(self, prefix, labels, x, y, pitch, horizontal=True, label_side="below", female=False,
               size=9, rotate=False, label_fill="white", skip_blank=True):
        """Draw a straight row of pins. labels: list of names ('' = unused position). Returns list of coords."""
        out = []
        for i, lab in enumerate(labels):
            px = x + i * pitch if horizontal else x
            py = y if horizontal else y + i * pitch
            if isinstance(lab, tuple):
                kname, lab = lab
            else:
                kname = lab
            key = f"{prefix}.{kname}" if kname else f"{prefix}.#{i}"
            self.pin(key, px, py, female=female)
            out.append((px, py))
            if not lab:
                continue
            if horizontal:
                if rotate:
                    ty = py + 10 if label_side == "below" else py - 10
                    anchor = "end" if label_side == "below" else "start"
                    self.text(px + 3, ty, lab, size=size, anchor=anchor, fill=label_fill, font=MONO, rotate=-90)
                else:
                    ty = py + 17 if label_side == "below" else py - 10
                    self.text(px, ty, lab, size=size, anchor="middle", fill=label_fill, font=MONO)
            else:
                if label_side == "right":
                    self.text(px + 10, py + 3.5, lab, size=size, anchor="start", fill=label_fill, font=MONO)
                else:
                    self.text(px - 10, py + 3.5, lab, size=size, anchor="end", fill=label_fill, font=MONO)
        return out

    def p(self, name):
        return self.pins[name]

    # ---------- schematic symbols ----------
    def ground(self, x, y, color=C["ink"]):
        self.line(x, y, x, y + 8, color)
        self.line(x - 9, y + 8, x + 9, y + 8, color, 2)
        self.line(x - 6, y + 12, x + 6, y + 12, color, 2)
        self.line(x - 3, y + 16, x + 3, y + 16, color, 2)

    def resistor_h(self, x1, x2, y, label=None, value=None, color=C["ink"]):
        """Zig-zag resistor between x1 and x2 on a horizontal line."""
        n = 6
        body = 44
        mid = (x1 + x2) / 2
        a, b = mid - body / 2, mid + body / 2
        pts = [(x1, y), (a, y)]
        step = body / n
        for i in range(n):
            pts.append((a + step * (i + 0.5), y + (-7 if i % 2 == 0 else 7)))
        pts += [(b, y), (x2, y)]
        self.poly(pts, color, 1.8)
        if label:
            self.text(mid, y - 13, label, size=11, anchor="middle", weight="bold")
        if value:
            self.text(mid, y + 22, value, size=11, anchor="middle")

    def resistor_v(self, x, y1, y2, label=None, value=None, color=C["ink"]):
        n = 6
        body = 40
        mid = (y1 + y2) / 2
        a = mid - body / 2
        b = mid + body / 2
        pts = [(x, y1), (x, a)]
        step = body / n
        for i in range(n):
            pts.append((x + (-6 if i % 2 == 0 else 6), a + step * (i + 0.5)))
        pts += [(x, b), (x, y2)]
        self.poly(pts, color, 1.8)
        if label:
            self.text(x + 11, mid - 2, label, size=11, weight="bold")
        if value:
            self.text(x + 11, mid + 12, value, size=11)

    def save(self, path):
        head = (f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" '
                f'viewBox="0 0 {self.w} {self.h}" role="img" aria-labelledby="t d">\n'
                f'<title id="t">{escape(self.title)}</title>\n<desc id="d">{escape(self.desc or self.title)}</desc>\n'
                f'<rect x="0" y="0" width="{self.w}" height="{self.h}" fill="#ffffff"/>\n')
        with open(path, "w") as f:
            f.write(head + "\n".join(self.body) + "\n</svg>\n")
