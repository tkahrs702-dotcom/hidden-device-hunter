// meter.h - pictures for the Uno R4 WiFi's 12 x 8 LED matrix.
#pragma once
#include <Arduino.h>
#include "Arduino_LED_Matrix.h"

namespace Meter {

static ArduinoLEDMatrix matrix;
static uint8_t frame[8][12];

inline void begin() { matrix.begin(); }

inline void clear() { memset(frame, 0, sizeof(frame)); }

inline void show() { matrix.renderBitmap(frame, 8, 12); }

// Signal bars: 0..12 columns lit, each column taller than the one before (like phone bars).
inline void bars(int lit) {
  clear();
  if (lit < 0) lit = 0;
  if (lit > 12) lit = 12;
  for (int c = 0; c < lit; c++) {
    int h = 1 + (c * 7) / 11;  // 1 .. 8 rows
    for (int r = 0; r < h; r++) frame[7 - r][c] = 1;
  }
  show();
}

inline void drawRows(const char *rows[8]) {
  clear();
  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 12 && rows[r][c]; c++) frame[r][c] = (rows[r][c] == 'X') ? 1 : 0;
  show();
}

// "?" = no target chosen yet
inline void question() {
  static const char *q[8] = {"....XXXX....", "...XX..XX...", ".......XX...", "......XX....",
                             ".....XX.....", ".....XX.....", "............", ".....XX....."};
  drawRows(q);
}

// "X" = target not heard in the last few scans
inline void lost() {
  static const char *x[8] = {"............", "...X....X...", "....X..X....", ".....XX.....",
                             ".....XX.....", "....X..X....", "...X....X...", "............"};
  drawRows(x);
}

// Camera picture = the target's maker is on the camera list (Phase 2)
inline void camera() {
  static const char *cam[8] = {"............", "..XXX.......", "XXXXXXXXXX..", "X........X.X",
                               "X...XX...XXX", "X...XX...X.X", "XXXXXXXXXX..", "............"};
  drawRows(cam);
}

// A moving dot along the bottom row = waiting for a target.
// linkUp adds a steady dot in the top-right corner: the sniffer's status lines are arriving.
inline void waiting(uint8_t step, bool linkUp) {
  clear();
  frame[7][step % 12] = 1;
  if (linkUp) frame[0][11] = 1;
  show();
}

}  // namespace Meter
