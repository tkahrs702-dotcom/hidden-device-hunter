// Hidden Device Hunter - Phase 2: main screen on the Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3
// Framework: ESP-IDF v5.5.5 + Waveshare BSP + LVGL (see ../README.md for build and flash).
//
// The P4 listens to the same serial lines the C5 sniffer sends the R4 and draws them on the
// 4.3 in touchscreen. The R4 can stay connected as the beeper: the P4 only listens, unless its
// TX wire is moved onto the C5 (then the mode buttons work too).
// Wiring (all 3.3 V, no level shifter on the P4 side, see ../README.md):
//   C5 TX (D7, LINK_CROSSED)  -> P4 header pin 11, GPIO5 (P4 RX)
//   C5 GND                    -> P4 header pin 5, GND
//   optional: P4 header pin 14, GPIO4 (P4 TX) -> C5 RX (D6), only with the R4's TX wire unplugged
//
// Lines from the C5 (one per line, 115200 baud):
//   L,<MAC>,<cat>,<maker>   locked (cat 0 = unknown, 1 = camera maker, 2 = IoT chip)
//   U                       unlocked
//   T,<now>,<avg>,<ch>      target signal in dBm, about 10 per second
//   S,<total>,<cams>,<iot>,<hidden>,<locked>   once a second
//   M,<mode>                0 = Wi-Fi, 1 = Bluetooth, 2 = both
//   B,<n> then n x D,<k>,<MAC>,<cat>,<avg>,<ch>,<maker>,<name>,<pkts>,<age s>,<flags>   device list
//   V,<0-3>                 buzzer volume, for the R4 only (ignored here)
// Sent to the C5 by the mode buttons: M,<0|1|2>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"

// ---------- settings you can change ----------
#define LINK_UART UART_NUM_1
#define LINK_RX_PIN 5  // header pin 11
#define LINK_TX_PIN 4  // header pin 14
#define LINK_BAUD 115200
// Same scale as the R4 bars (phase2-r4-display.ino).
#define RSSI_FAR (-90)
#define RSSI_NEAR (-30)
#define LOST_AFTER_MS 3000
#define MAX_ROWS 60
// ---------------------------------------------

static const char *CAT_NAMES[] = {"", "CAMERA maker", "IoT chip"};
static const char *MODE_NAMES[] = {"Wi-Fi", "Bluetooth", "Wi-Fi + Bluetooth"};

typedef struct {
  char mac[18];
  int cat, avg, ch, flags;
  char maker[24], name[24];
} Row;

// Written by the serial task, read by the screen timer; guarded by `lock`.
static portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
static struct {
  bool locked, camera;
  char mac[18], maker[24];
  int cat, now, avg, ch, peak;
  uint32_t lastT, lastS;
  int total, cams, iot, hidden, mode;
  Row rows[MAX_ROWS];
  int rowCount, rowsExpected;
  bool rowsDirty, lockDirty;
} st = {.mode = -1, .now = -100, .avg = -100};

static uint32_t ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }

static int strengthPct(int rssi) {
  int p = (rssi - RSSI_FAR) * 100 / (RSSI_NEAR - RSSI_FAR);
  return p < 0 ? 0 : (p > 100 ? 100 : p);
}

static void copyField(char *dst, const char *src, size_t n) {
  strncpy(dst, src ? src : "", n - 1);
  dst[n - 1] = 0;
}

// ---------- serial link ----------
static void onLine(char *line) {
  char *f[12] = {0};
  int n = 0;
  for (char *save, *tok = strtok_r(line, ",", &save); tok && n < 12; tok = strtok_r(NULL, ",", &save)) f[n++] = tok;
  if (n == 0) return;
  uint32_t now = ms();
  taskENTER_CRITICAL(&lock);
  switch (f[0][0]) {
    case 'L':
      if (n < 3) break;
      st.locked = true;
      st.lastT = now;
      st.peak = 0;
      st.cat = atoi(f[2]);
      st.camera = st.cat == 1;
      copyField(st.mac, f[1], sizeof(st.mac));
      copyField(st.maker, n >= 4 ? f[3] : "", sizeof(st.maker));
      st.lockDirty = true;
      break;
    case 'U':
      st.locked = false;
      st.lockDirty = true;
      break;
    case 'T':
      if (n < 3) break;
      st.locked = true;
      st.lastT = now;
      st.now = atoi(f[1]);
      st.avg = atoi(f[2]);
      st.ch = n >= 4 ? atoi(f[3]) : 0;
      if (strengthPct(st.avg) > st.peak) st.peak = strengthPct(st.avg);
      break;
    case 'S':
      if (n < 6) break;
      st.lastS = now;
      st.total = atoi(f[1]);
      st.cams = atoi(f[2]);
      st.iot = atoi(f[3]);
      st.hidden = atoi(f[4]);
      if ((atoi(f[5]) == 1) != st.locked) {  // catches a missed L or U line
        st.locked = !st.locked;
        if (st.locked) st.lastT = now;
        st.lockDirty = true;
      }
      break;
    case 'M':
      if (n >= 2) st.mode = atoi(f[1]);
      break;
    case 'B':
      if (n < 2) break;
      st.rowsExpected = atoi(f[1]);
      st.rowCount = 0;
      if (st.rowsExpected == 0) st.rowsDirty = true;
      break;
    case 'D':
      if (n < 6 || st.rowCount >= MAX_ROWS) break;
      {
        Row *r = &st.rows[st.rowCount++];
        copyField(r->mac, f[2], sizeof(r->mac));
        r->cat = atoi(f[3]);
        r->avg = atoi(f[4]);
        r->ch = atoi(f[5]);
        copyField(r->maker, n >= 7 ? f[6] : "-", sizeof(r->maker));
        copyField(r->name, n >= 8 ? f[7] : "-", sizeof(r->name));
        r->flags = n >= 11 ? atoi(f[10]) : 0;
      }
      if (st.rowCount >= st.rowsExpected) st.rowsDirty = true;
      break;
  }
  taskEXIT_CRITICAL(&lock);
}

static void linkTask(void *arg) {
  static char buf[160];
  int len = 0;
  uint8_t chunk[64];
  for (;;) {
    int got = uart_read_bytes(LINK_UART, chunk, sizeof(chunk), pdMS_TO_TICKS(50));
    for (int i = 0; i < got; i++) {
      char c = (char)chunk[i];
      if (c == '\r') continue;
      if (c == '\n') {
        buf[len] = 0;
        if (len) onLine(buf);
        len = 0;
      } else if (len < (int)sizeof(buf) - 1) {
        buf[len++] = c;
      }
    }
  }
}

static void linkBegin(void) {
  uart_config_t cfg = {
      .baud_rate = LINK_BAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(LINK_UART, 4096, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(LINK_UART, &cfg));
  ESP_ERROR_CHECK(uart_set_pin(LINK_UART, LINK_TX_PIN, LINK_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  // Pull-up keeps the line idle (high) when the C5 is unplugged, so no garbage is read.
  gpio_pullup_en(LINK_RX_PIN);
  xTaskCreate(linkTask, "link", 4096, NULL, 5, NULL);
}

// ---------- screen ----------
static lv_obj_t *lblLink, *lblStats, *lblTitle, *lblMaker, *lblDbm, *lblInfo, *bar, *list;
static lv_obj_t *modeBtn[3];

static void modeClicked(lv_event_t *e) {
  int m = (int)(intptr_t)lv_event_get_user_data(e);
  char out[8];
  int n = snprintf(out, sizeof(out), "M,%d\n", m);
  uart_write_bytes(LINK_UART, out, n);  // reaches the C5 only if the optional TX wire is fitted
}

static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, lv_color_t color) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, color, 0);
  lv_label_set_text(l, "");
  return l;
}

static void buildScreen(void) {
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x101418), 0);
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, 12, 0);
  lv_obj_set_style_pad_row(scr, 8, 0);
  lv_color_t white = lv_color_hex(0xFFFFFF), grey = lv_color_hex(0x9AA4AE);

  lblLink = label(scr, &lv_font_montserrat_20, grey);
  lblStats = label(scr, &lv_font_montserrat_14, grey);
  lblTitle = label(scr, &lv_font_montserrat_28, white);
  lblMaker = label(scr, &lv_font_montserrat_20, lv_color_hex(0xFFB020));
  lblDbm = label(scr, &lv_font_montserrat_48, white);
  bar = lv_bar_create(scr);
  lv_obj_set_size(bar, LV_PCT(100), 36);
  lv_bar_set_range(bar, 0, 100);
  lblInfo = label(scr, &lv_font_montserrat_20, grey);

  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, 8, 0);
  for (int i = 0; i < 3; i++) {
    modeBtn[i] = lv_button_create(row);
    lv_obj_set_flex_grow(modeBtn[i], 1);
    lv_obj_t *t = lv_label_create(modeBtn[i]);
    lv_label_set_text(t, MODE_NAMES[i]);
    lv_obj_center(t);
    lv_obj_add_event_cb(modeBtn[i], modeClicked, LV_EVENT_CLICKED, (void *)(intptr_t)i);
  }

  list = lv_list_create(scr);
  lv_obj_set_width(list, LV_PCT(100));
  lv_obj_set_flex_grow(list, 1);
  lv_list_add_text(list, "Device list: waiting for the sniffer");
}

static void refresh(lv_timer_t *t) {
  static typeof(st) s;  // snapshot, so the serial task is never held up by drawing
  taskENTER_CRITICAL(&lock);
  s = st;
  st.rowsDirty = st.lockDirty = false;
  taskEXIT_CRITICAL(&lock);
  uint32_t now = ms();

  bool linkOk = s.lastS && now - s.lastS < 2500;
  lv_label_set_text_fmt(lblLink, "Sniffer: %s    Mode: %s", linkOk ? "connected" : "not heard",
                        s.mode >= 0 && s.mode <= 2 ? MODE_NAMES[s.mode] : "?");
  lv_label_set_text_fmt(lblStats, "Heard %d   cameras %d   IoT %d   hidden %d", s.total, s.cams, s.iot, s.hidden);
  for (int i = 0; i < 3; i++) {
    if (i == s.mode) lv_obj_add_state(modeBtn[i], LV_STATE_CHECKED);
    else lv_obj_remove_state(modeBtn[i], LV_STATE_CHECKED);
  }

  if (!s.locked) {
    lv_label_set_text(lblTitle, "Scanning, nothing locked");
    lv_label_set_text(lblMaker, "");
    lv_label_set_text(lblDbm, "--");
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_label_set_text(lblInfo, "Press BOOT on the sniffer to lock the next suspect");
  } else {
    bool lost = now - s.lastT > LOST_AFTER_MS;
    int pct = strengthPct(s.avg);
    lv_label_set_text_fmt(lblTitle, "Locked %s", s.mac);
    lv_label_set_text_fmt(lblMaker, "%s %s", s.cat >= 0 && s.cat <= 2 ? CAT_NAMES[s.cat] : "", s.maker);
    if (lost) {
      lv_label_set_text(lblDbm, "Not heard");
      lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    } else {
      lv_label_set_text_fmt(lblDbm, "%d dBm", s.avg);
      lv_bar_set_value(bar, pct, LV_ANIM_ON);
    }
    lv_obj_set_style_bg_color(bar, pct >= 97 ? lv_color_hex(0xE53935) : lv_color_hex(0x2E9BFF), LV_PART_INDICATOR);
    lv_label_set_text_fmt(lblInfo, "now %d dBm   ch %d   closest %d%%", s.now, s.ch, s.peak);
  }

  if (s.rowsDirty) {
    lv_obj_clean(list);
    if (s.rowCount == 0) lv_list_add_text(list, "No devices heard yet");
    for (int i = 0; i < s.rowCount; i++) {
      Row *r = &s.rows[i];
      const char *tag = r->cat == 1 ? "CAM" : r->cat == 2 ? "IOT" : (r->flags & 4) ? "HID" : (r->flags & 16) ? "BLE" : "";
      char line[96];
      snprintf(line, sizeof(line), "%-3s %4d dBm  ch%-3d %s  %s  %s", tag, r->avg, r->ch, r->mac, r->maker, r->name);
      lv_list_add_button(list, NULL, line);
    }
  }
}

void app_main(void) {
  linkBegin();
  bsp_display_start();
  bsp_display_backlight_on();
  bsp_display_lock(0);
  buildScreen();
  lv_timer_create(refresh, 100, NULL);
  bsp_display_unlock();
}
