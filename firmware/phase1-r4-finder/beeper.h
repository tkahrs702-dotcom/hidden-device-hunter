// beeper.h - background beeper for the Arduino Uno R4 WiFi.
//
// Why a timer: WiFi.scanNetworks() blocks loop() for a few seconds. If loop() made the
// beeps, they would stop during every scan. Here a hardware timer ticks 5000 times a
// second and toggles the buzzer pin itself, so the beeping never pauses.
//
// Tone: the pin flips every tick, so a passive piezo plays about 2.5 kHz.
// Usage:  Beeper::begin(8);          // buzzer on D8 (through 100 ohm)
//         Beeper::set(500, 40);      // a 40 ms beep every 500 ms
//         Beeper::set(0, 0);         // silent
//         Beeper::continuous();      // steady tone (target is right in front of you)
#pragma once
#include <Arduino.h>
#include "FspTimer.h"

namespace Beeper {

static FspTimer timer;
static volatile uint8_t pin = 8;
static volatile uint32_t periodTicks = 0;  // ticks from one beep start to the next; 0 = silent
static volatile uint32_t onTicks = 0;      // beep length in ticks
static volatile uint32_t tick = 0;
static volatile bool level = false;
static const uint32_t TICKS_PER_MS = 5;    // 5000 Hz tick
static volatile bool muted = false;

static void isr(timer_callback_args_t *) {
  uint32_t p = periodTicks;
  if (p == 0 || muted) {
    if (level) { level = false; digitalWrite(pin, LOW); }
    tick = 0;
    return;
  }
  if (tick >= p) tick = 0;
  if (tick < onTicks) {
    level = !level;
    digitalWrite(pin, level ? HIGH : LOW);
  } else if (level) {
    level = false;
    digitalWrite(pin, LOW);
  }
  tick++;
}

inline bool begin(uint8_t buzzerPin) {
  pin = buzzerPin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  uint8_t type = 0;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    FspTimer::force_use_of_pwm_reserved_timer();
    ch = FspTimer::get_available_timer(type, true);
  }
  if (ch < 0) return false;
  if (!timer.begin(TIMER_MODE_PERIODIC, type, ch, 5000.0f, 0.0f, isr)) return false;
  timer.setup_overflow_irq();
  timer.open();
  timer.start();
  return true;
}

// intervalMs = time from one beep start to the next, lengthMs = how long each beep lasts.
inline void set(uint32_t intervalMs, uint32_t lengthMs) {
  noInterrupts();
  periodTicks = intervalMs * TICKS_PER_MS;
  onTicks = lengthMs * TICKS_PER_MS;
  interrupts();
}

inline void continuous() { set(1000, 1000); }
inline void mute(bool m) { muted = m; }

}  // namespace Beeper
