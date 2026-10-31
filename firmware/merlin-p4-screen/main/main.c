// Hidden Device Hunter - Merlin: the finder's main screen.
// Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-4.3. Framework: ESP-IDF 5.5 + Waveshare BSP + LVGL 9.
// Build, flash and wiring: ../README.md and docs/merlin-build-guide.md.
//
// Merlin listens to two boards over 3.3 V serial wires (no 5 V on any of these pins):
//   Nimue (XIAO ESP32-C5 sniffer): Nimue D7 (TX) -> GPIO5,  GPIO4 -> Nimue D6 (RX, optional)
//   Iolaus (handheld TTGO LoRa32): Iolaus TX -> GPIO21,     GPIO22 -> Iolaus RX
// Sound comes from the onboard speaker (2-pin socket by MIC1), driven by the onboard codec.
//
// Lines from Nimue, 115200 baud, one per line:
//   L,<MAC>,<cat>,<maker>   locked (cat 0 = unknown, 1 = camera maker, 2 = IoT chip)
//   U                       unlocked
//   T,<now>,<avg>,<ch>      target signal in dBm, about 10 per second
//   S,<total>,<cams>,<iot>,<hidden>,<locked>   once a second
//   M,<mode>                0 = Wi-Fi, 1 = Bluetooth, 2 = both
//   B,<n> then n x D,<k>,<MAC>,<cat>,<avg>,<ch>,<maker>,<name>,<pkts>,<age s>,<flags>   device list
//   V,<0-3>                 sound volume
// Sent to Nimue by the mode buttons (needs the optional wire): M,<0|1|2>
//
// Lines from Iolaus, 115200 baud (Iolaus passes on what Argus sends by LoRa):
//   I,<uptime s>                                    Iolaus is alive, every 2 s
//   H,<seq>,<router ch>,<uptime min>,<lora dBm>     Argus is alive, every 30 s
//   A,<seq>,<deauth>,<disassoc>,<ch>,<from MAC>,<to MAC>,<dBm>,<lora dBm>   attack on the router

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#if defined(BSP_CAPS_AUDIO) && BSP_CAPS_AUDIO && __has_include("esp_codec_dev.h")
#include "esp_codec_dev.h"
#define HAVE_SPEAKER 1
#else
#define HAVE_SPEAKER 0
#endif

// ---------- settings you can change ----------
#define NIMUE_UART UART_NUM_1
#define NIMUE_RX_PIN 5
#define NIMUE_TX_PIN 4
#define IOLAUS_UART UART_NUM_2
#define IOLAUS_RX_PIN 21
#define IOLAUS_TX_PIN 22
#define LINK_BAUD 115200
#define RSSI_FAR (-90)   // same scale as the old R4 bars
#define RSSI_NEAR (-30)
#define LOST_AFTER_MS 3000
#define ARGUS_LATE_MS 75000   // no heartbeat for this long -> "Argus not heard"
#define ATTACK_SHOW_MS 600000 // attack banner stays 10 minutes after the last alert
#define MAX_ROWS 60
#define SEGMENTS 12
// ---------------------------------------------

static const char *TAG = "merlin";
static const char *CAT_NAMES[] = {"", "CAMERA maker", "IoT chip"};
static const char *MODE_NAMES[] = {"Wi-Fi", "Bluetooth", "Both"};
static const char *VOL_NAMES[] = {"Sound off", "Sound low", "Sound mid", "Sound high"};

// Scorched Earth palette
#define C_BG 0x0C0E0B
#define C_RED 0xD32F2F
#define C_OLIVE 0x9E9D24
#define C_BORDER 0x4A5D4E
#define C_TEXT 0xE0E2DC
#define C_DIM 0x8A8F86

typedef struct {
  char mac[18];
  int cat, avg, ch, flags;
  char maker[24], name[24];
} Row;

// Written by the serial tasks, read by the screen timer and the sound task; guarded by `lock`.
static portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
static struct {
  // Nimue
  bool locked, camera;
  char mac[18], maker[24];
  int cat, now, avg, ch, peak;
  uint32_t lastT, lastS;
  int total, cams, iot, hidden, mode, volume, nimueVolume;
  Row rows[MAX_ROWS];
  int rowCount, rowsExpected;
  bool rowsDirty;
  // Iolaus / Argus
  uint32_t lastI, lastH, lastA;
  int routerCh, loraDbm, argusUpMin;
  int deauth, disassoc, attackCh, attackDbm, attacks;
  char attackFrom[18], attackTo[18];
  bool alarm;  // set by a new attack line, cleared by the sound task once it has played
} st = {.mode = -1, .now = -100, .avg = -100, .volume = 2, .nimueVolume = -1};

static uint32_t ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }

static int strengthPct(int rssi) {
  int p = (rssi - RSSI_FAR) * 100 / (RSSI_NEAR - RSSI_FAR);
  return p < 0 ? 0 : (p > 100 ? 100 : p);
}

static void copyField(char *dst, const char *src, size_t n) {
  strncpy(dst, src ? src : "", n - 1);
  dst[n - 1] = 0;
}

// Splits on commas, keeping empty fields (strtok would drop them and shift every later field).
static int split(char *line, char **f, int max) {
  int n = 0;
  f[n++] = line;
  for (char *p = line; *p && n < max; p++)
    if (*p == ',') {
      *p = 0;
      f[n++] = p + 1;
    }
  return n;
}

// ---------- Nimue link ----------
static void onNimueLine(char *line) {
  char *f[12];
  int n = split(line, f, 12);
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
      break;
    case 'U':
      st.locked = false;
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
      }
      break;
    case 'M':
      if (n >= 2) st.mode = atoi(f[1]);
      break;
    case 'V':  // Nimue repeats its volume; only a change counts, so it never undoes a tap on the Sound button
      if (n >= 2 && atoi(f[1]) >= 0 && atoi(f[1]) <= 3 && atoi(f[1]) != st.nimueVolume)
        st.volume = st.nimueVolume = atoi(f[1]);
      break;
    case 'B':
      if (n < 2) break;
      st.rowsExpected = atoi(f[1]) > MAX_ROWS ? MAX_ROWS : atoi(f[1]);
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

// ---------- Iolaus link ----------
static void onIolausLine(char *line) {
  char *f[10];
  int n = split(line, f, 10);
  uint32_t now = ms();
  taskENTER_CRITICAL(&lock);
  switch (f[0][0]) {
    case 'I':
      st.lastI = now;
      break;
    case 'H':
      if (n < 5) break;
      st.lastI = st.lastH = now;
      st.routerCh = atoi(f[2]);
      st.argusUpMin = atoi(f[3]);
      st.loraDbm = atoi(f[4]);
      break;
    case 'A':
      if (n < 9) break;
      st.lastI = st.lastH = st.lastA = now;
      st.deauth = atoi(f[2]);
      st.disassoc = atoi(f[3]);
      st.attackCh = atoi(f[4]);
      copyField(st.attackFrom, f[5], sizeof(st.attackFrom));
      copyField(st.attackTo, f[6], sizeof(st.attackTo));
      st.attackDbm = atoi(f[7]);
      st.loraDbm = atoi(f[8]);
      st.attacks++;
      st.alarm = true;
      break;
  }
  taskEXIT_CRITICAL(&lock);
}

typedef struct {
  uart_port_t port;
  void (*onLine)(char *);
} Link;

static void linkTask(void *arg) {
  Link *link = arg;
  char buf[160];
  int len = 0;
  uint8_t chunk[64];
  for (;;) {
    int got = uart_read_bytes(link->port, chunk, sizeof(chunk), pdMS_TO_TICKS(50));
    for (int i = 0; i < got; i++) {
      char c = (char)chunk[i];
      if (c == '\r') continue;
      if (c == '\n') {
        buf[len] = 0;
        if (len) link->onLine(buf);
        len = 0;
      } else if (len < (int)sizeof(buf) - 1) {
        buf[len++] = c;
      }
    }
  }
}

static void linkBegin(Link *link, int rx, int tx, const char *name) {
  uart_config_t cfg = {
      .baud_rate = LINK_BAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(link->port, 4096, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(link->port, &cfg));
  ESP_ERROR_CHECK(uart_set_pin(link->port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  gpio_pullup_en(rx);  // idle high when the other board is unplugged, so no garbage is read
  xTaskCreate(linkTask, name, 4096, link, 5, NULL);
}

static Link nimue = {NIMUE_UART, onNimueLine};
static Link iolaus = {IOLAUS_UART, onIolausLine};

// ---------- sound (onboard speaker) ----------
#if HAVE_SPEAKER
#define RATE 16000
static esp_codec_dev_handle_t spk;
static int16_t tone[RATE / 1000 * 80];  // up to 80 ms

static void play(int hz, int durMs, int volume) {
  static const int VOL[] = {0, 45, 65, 85};
  int n = RATE / 1000 * durMs;
  if (n > (int)(sizeof(tone) / sizeof(tone[0]))) n = sizeof(tone) / sizeof(tone[0]);
  for (int i = 0; i < n; i++) {
    float env = i < 80 ? i / 80.0f : (i > n - 80 ? (n - i) / 80.0f : 1.0f);  // no clicks
    tone[i] = (int16_t)(12000 * env * sinf(2 * (float)M_PI * hz * i / RATE));
  }
  esp_codec_dev_set_out_vol(spk, VOL[volume]);
  esp_codec_dev_write(spk, tone, n * sizeof(int16_t));
}

// Beeps faster as the locked device gets closer, like the old R4 beeper; a siren on a router attack.
static void soundTask(void *arg) {
  spk = bsp_audio_codec_speaker_init();
  if (!spk) {
    ESP_LOGE(TAG, "speaker not found, running silent");
    vTaskDelete(NULL);
  }
  esp_codec_dev_sample_info_t fs = {.sample_rate = RATE, .channel = 1, .bits_per_sample = 16};
  esp_codec_dev_open(spk, &fs);
  for (;;) {
    taskENTER_CRITICAL(&lock);
    bool alarm = st.alarm, heard = st.locked && ms() - st.lastT < LOST_AFTER_MS;
    int vol = st.volume, pct = strengthPct(st.avg);
    st.alarm = false;
    taskEXIT_CRITICAL(&lock);
    if (vol == 0) {
      vTaskDelay(pdMS_TO_TICKS(200));
    } else if (alarm) {
      for (int i = 0; i < 6; i++) play(i % 2 ? 1400 : 2200, 80, vol);
    } else if (heard) {
      play(1800 + pct * 12, 40, vol);
      vTaskDelay(pdMS_TO_TICKS(1000 - pct * 9));  // 1 s far away, 0.1 s right next to it
    } else {
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}
#endif

// ---------- screen ----------
static lv_obj_t *lblNimue, *lblIolaus, *lblGuard, *guardBox, *lblTitle, *lblMaker, *lblDbm, *lblInfo, *lblStats, *list;
static lv_obj_t *seg[SEGMENTS], *modeBtn[3], *volLbl;

static void modeClicked(lv_event_t *e) {
  int m = (int)(intptr_t)lv_event_get_user_data(e);
  char out[8];
  int n = snprintf(out, sizeof(out), "M,%d\n", m);
  uart_write_bytes(NIMUE_UART, out, n);  // reaches Nimue only if the optional wire is fitted
}

static void volClicked(lv_event_t *e) {
  taskENTER_CRITICAL(&lock);
  st.volume = (st.volume + 1) % 4;
  taskEXIT_CRITICAL(&lock);
}

static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, uint32_t color) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
  lv_obj_set_width(l, LV_PCT(100));
  lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
  lv_label_set_text(l, "");
  return l;
}

static lv_obj_t *box(lv_obj_t *parent) {
  lv_obj_t *b = lv_obj_create(parent);
  lv_obj_set_size(b, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(b, lv_color_hex(C_BG), 0);
  lv_obj_set_style_border_color(b, lv_color_hex(C_BORDER), 0);
  lv_obj_set_style_border_width(b, 2, 0);
  lv_obj_set_style_radius(b, 8, 0);
  lv_obj_set_style_pad_all(b, 10, 0);
  lv_obj_set_style_pad_row(b, 4, 0);
  lv_obj_set_flex_flow(b, LV_FLEX_FLOW_COLUMN);
  lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
  return b;
}

static lv_obj_t *button(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *data, lv_obj_t **lbl) {
  lv_obj_t *b = lv_button_create(parent);
  lv_obj_set_flex_grow(b, 1);
  lv_obj_set_height(b, 56);
  lv_obj_set_style_bg_color(b, lv_color_hex(C_BORDER), 0);
  lv_obj_set_style_bg_color(b, lv_color_hex(C_OLIVE), LV_STATE_CHECKED);
  lv_obj_t *t = lv_label_create(b);
  lv_obj_set_style_text_font(t, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(t, lv_color_hex(C_TEXT), 0);
  lv_label_set_text(t, text);
  lv_obj_center(t);
  lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, data);
  if (lbl) *lbl = t;
  return b;
}

static void buildScreen(void) {
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_hex(C_BG), 0);
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, 10, 0);
  lv_obj_set_style_pad_row(scr, 8, 0);

  lv_obj_t *top = box(scr);
  lblNimue = label(top, &lv_font_montserrat_18, C_DIM);
  lblIolaus = label(top, &lv_font_montserrat_18, C_DIM);

  guardBox = box(scr);
  lblGuard = label(guardBox, &lv_font_montserrat_20, C_TEXT);
  lv_label_set_long_mode(lblGuard, LV_LABEL_LONG_WRAP);

  lv_obj_t *target = box(scr);
  lblTitle = label(target, &lv_font_montserrat_20, C_TEXT);
  lblMaker = label(target, &lv_font_montserrat_20, C_OLIVE);
  lblDbm = label(target, &lv_font_montserrat_48, C_TEXT);
  lv_obj_t *bar = lv_obj_create(target);  // segmented bar: SEGMENTS blocks that light up
  lv_obj_remove_style_all(bar);
  lv_obj_set_size(bar, LV_PCT(100), 40);
  lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(bar, 4, 0);
  for (int i = 0; i < SEGMENTS; i++) {
    seg[i] = lv_obj_create(bar);
    lv_obj_remove_style_all(seg[i]);
    lv_obj_set_flex_grow(seg[i], 1);
    lv_obj_set_height(seg[i], LV_PCT(100));
    lv_obj_set_style_radius(seg[i], 3, 0);
    lv_obj_set_style_bg_opa(seg[i], LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(seg[i], lv_color_hex(C_BORDER), 0);
  }
  lblInfo = label(target, &lv_font_montserrat_18, C_DIM);

  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, 6, 0);
  for (int i = 0; i < 3; i++) modeBtn[i] = button(row, MODE_NAMES[i], modeClicked, (void *)(intptr_t)i, NULL);
  button(row, VOL_NAMES[2], volClicked, NULL, &volLbl);

  lblStats = label(scr, &lv_font_montserrat_14, C_DIM);
  list = lv_list_create(scr);
  lv_obj_set_width(list, LV_PCT(100));
  lv_obj_set_flex_grow(list, 1);
  lv_obj_set_style_bg_color(list, lv_color_hex(C_BG), 0);
  lv_obj_set_style_border_color(list, lv_color_hex(C_BORDER), 0);
  lv_obj_set_style_text_color(list, lv_color_hex(C_TEXT), 0);
  lv_list_add_text(list, "Device list: waiting for Nimue");
}

static void refresh(lv_timer_t *t) {
  static typeof(st) s;  // snapshot, so the serial tasks are never held up by drawing
  taskENTER_CRITICAL(&lock);
  s = st;
  st.rowsDirty = false;
  taskEXIT_CRITICAL(&lock);
  uint32_t now = ms();

  bool nimueOk = s.lastS && now - s.lastS < 2500;
  lv_label_set_text_fmt(lblNimue, "Nimue sniffer: %s   %s", nimueOk ? "connected" : "NOT HEARD",
                        s.mode >= 0 && s.mode <= 2 ? MODE_NAMES[s.mode] : "");
  lv_obj_set_style_text_color(lblNimue, lv_color_hex(nimueOk ? C_OLIVE : C_RED), 0);
  bool iolausOk = s.lastI && now - s.lastI < 5000;
  lv_label_set_text_fmt(lblIolaus, "Iolaus LoRa: %s", iolausOk ? "connected" : "NOT HEARD");
  lv_obj_set_style_text_color(lblIolaus, lv_color_hex(iolausOk ? C_OLIVE : C_RED), 0);

  bool attack = s.lastA && now - s.lastA < ATTACK_SHOW_MS;
  bool argusOk = s.lastH && now - s.lastH < ARGUS_LATE_MS;
  if (attack) {
    lv_label_set_text_fmt(lblGuard, "ROUTER ATTACK %lus ago: %d deauth, %d disassoc on ch %d\nfrom %s  to %s  (%d dBm at Argus)",
                          (unsigned long)((now - s.lastA) / 1000), s.deauth, s.disassoc, s.attackCh, s.attackFrom,
                          s.attackTo, s.attackDbm);
    lv_obj_set_style_border_color(guardBox, lv_color_hex(C_RED), 0);
    lv_obj_set_style_text_color(lblGuard, lv_color_hex(C_RED), 0);
  } else if (argusOk) {
    lv_label_set_text_fmt(lblGuard, "Router guard OK. Argus on ch %d, up %d min, LoRa %d dBm. Alerts so far: %d",
                          s.routerCh, s.argusUpMin, s.loraDbm, s.attacks);
    lv_obj_set_style_border_color(guardBox, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_text_color(lblGuard, lv_color_hex(C_TEXT), 0);
  } else {
    lv_label_set_text(lblGuard, "Router guard: Argus not heard yet");
    lv_obj_set_style_border_color(guardBox, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_text_color(lblGuard, lv_color_hex(C_DIM), 0);
  }

  for (int i = 0; i < 3; i++) {
    if (i == s.mode) lv_obj_add_state(modeBtn[i], LV_STATE_CHECKED);
    else lv_obj_remove_state(modeBtn[i], LV_STATE_CHECKED);
  }
  lv_label_set_text(volLbl, VOL_NAMES[s.volume]);

  int pct = 0;
  if (!s.locked) {
    lv_label_set_text(lblTitle, "Scanning, nothing locked");
    lv_label_set_text(lblMaker, "");
    lv_label_set_text(lblDbm, "--");
    lv_label_set_text(lblInfo, "Press BOOT on Nimue to lock the next suspect");
  } else {
    bool lost = now - s.lastT > LOST_AFTER_MS;
    lv_label_set_text_fmt(lblTitle, "Locked %s", s.mac);
    lv_label_set_text_fmt(lblMaker, "%s %s", s.cat >= 0 && s.cat <= 2 ? CAT_NAMES[s.cat] : "", s.maker);
    lv_obj_set_style_text_color(lblMaker, lv_color_hex(s.camera ? C_RED : C_OLIVE), 0);
    if (lost) {
      lv_label_set_text(lblDbm, "Not heard");
    } else {
      pct = strengthPct(s.avg);
      lv_label_set_text_fmt(lblDbm, "%d dBm", s.avg);
    }
    lv_label_set_text_fmt(lblInfo, "now %d dBm   ch %d   closest %d%%", s.now, s.ch, s.peak);
  }
  int lit = (pct * SEGMENTS + 50) / 100;
  for (int i = 0; i < SEGMENTS; i++) {
    uint32_t c = i >= lit ? C_BORDER : (i >= SEGMENTS * 3 / 4 ? C_RED : C_OLIVE);
    lv_obj_set_style_bg_color(seg[i], lv_color_hex(c), 0);
  }

  lv_label_set_text_fmt(lblStats, "Heard %d   cameras %d   IoT %d   hidden %d", s.total, s.cams, s.iot, s.hidden);
  if (s.rowsDirty) {
    lv_obj_clean(list);
    if (s.rowCount == 0) lv_list_add_text(list, "No devices heard yet");
    for (int i = 0; i < s.rowCount; i++) {
      Row *r = &s.rows[i];
      const char *tag = r->cat == 1 ? "CAM" : r->cat == 2 ? "IOT" : (r->flags & 4) ? "HID" : (r->flags & 16) ? "BLE" : "";
      char line[96];
      snprintf(line, sizeof(line), "%-3s %4d dBm  ch%-3d %s  %s  %s", tag, r->avg, r->ch, r->mac, r->maker, r->name);
      lv_obj_t *b = lv_list_add_button(list, NULL, line);
      lv_obj_set_style_bg_color(b, lv_color_hex(C_BG), 0);
      lv_obj_set_style_text_color(b, lv_color_hex(r->cat == 1 ? C_RED : C_TEXT), 0);
    }
  }
}

void app_main(void) {
  linkBegin(&nimue, NIMUE_RX_PIN, NIMUE_TX_PIN, "nimue");
  linkBegin(&iolaus, IOLAUS_RX_PIN, IOLAUS_TX_PIN, "iolaus");

  // Same start-up as the board's verified firmware: the panel is natively 480 x 800 portrait, so no rotation.
  bsp_display_cfg_t cfg = {
      .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
      .rotation = (esp_lv_adapter_rotation_t)0,
      .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL,
  };
  bsp_display_start_with_config(&cfg);
  bsp_display_backlight_on();
  bsp_display_lock(-1);  // -1 = wait for the LVGL lock; every LVGL call outside an LVGL timer needs it
  buildScreen();
  lv_timer_create(refresh, 100, NULL);
  bsp_display_unlock();

#if HAVE_SPEAKER
  xTaskCreate(soundTask, "sound", 4096, NULL, 4, NULL);
#else
  ESP_LOGW(TAG, "this board package has no speaker support; running silent");
#endif
}
