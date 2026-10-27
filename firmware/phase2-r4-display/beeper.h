// beeper.h - Modulino Buzzer beeper for the Arduino Uno R4 WiFi.
//
// Buzzer: Arduino Modulino Buzzer (ABX00108) on the R4's Qwiic socket (Wire1, 3.3 V).
// The Modulino plays each tone by itself; the R4 only tells it when to start a beep.
//
// Why a FreeRTOS task: WiFi.scanNetworks() blocks for a few seconds, and I2C can't be
// driven from a timer interrupt. A separate, higher-priority task sends the beeps, so
// they keep their rhythm during scans. The sketch's loop() runs in its own task.
//
// Usage:  Beeper::begin();            // in setup(); plays a 3-beep startup chirp (4 kHz, 100 ms)
//         Beeper::set(500, 40);       // a 40 ms beep every 500 ms
//         Beeper::set(0, 0);          // silent
//         Beeper::continuous();       // steady tone (target is right in front of you)
//         Beeper::mute(true);         // mute / unmute
//         Beeper::runLoopAsTask();    // last line of setup(); starts the tasks
#pragma once
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Modulino.h>

namespace Beeper {

static ModulinoBuzzer buzzer;
static volatile uint32_t periodMs = 0;   // beep start to next beep start; 0 = silent
static volatile uint32_t onMs = 0;       // beep length
static volatile uint32_t gen = 0;        // changes on every new setting
static volatile bool muted = false;
static volatile bool found = false;
static volatile uint8_t volumeLevel = 3;  // 0 mute, 1 quiet, 2 medium, 3 loud
// The Modulino has no volume setting; its piezo is loudest near 4 kHz, so lower levels use lower pitches.
static uint32_t freqHz() { return volumeLevel >= 3 ? 4000 : (volumeLevel == 2 ? 2500 : 1500); }
static TaskHandle_t beepTask = nullptr;
static TaskHandle_t loopTask = nullptr;

static void play(uint32_t ms) { if (found && volumeLevel) buzzer.tone(freqHz(), ms); }
static void quiet() { if (found) buzzer.noTone(); }

// Try to find the buzzer on Wire1. Only begin() and the beeper task touch Wire1.
static bool probe(int tries) {
  for (int i = 0; i < tries; i++) {
    if (buzzer.begin()) return true;
    delay(100);
  }
  return false;
}

static void beepThread(void *) {
  while (!found) {  // keep looking if it was missed at power-up
    found = probe(1);
    if (!found) vTaskDelay(pdMS_TO_TICKS(900));
  }
  for (int i = 0; i < 3; i++) { play(100); vTaskDelay(pdMS_TO_TICKS(200)); }  // startup chirp
  bool sounding = false;
  for (;;) {
    uint32_t p = periodMs, on = onMs, g = gen;
    if (p == 0 || muted) {
      if (sounding) { quiet(); sounding = false; }
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }
    if (on >= p) {  // steady tone: keep refreshing it before it runs out
      play(400);
      sounding = true;
      for (int t = 0; t < 30 && g == gen && !muted; t++) vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    play(on);
    sounding = true;
    for (uint32_t t = 0; t < p && g == gen && !muted; t += 10) vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void loopThread(void *) { for (;;) loop(); }

inline bool begin(uint8_t /*unused*/ = 0) {
  Modulino.begin();
  found = probe(5);
  if (xTaskCreate(beepThread, "beep", 320, nullptr, 2, &beepTask) != pdPASS) {
    Serial.println(F("WARNING: not enough memory for the beeper task."));
    return false;
  }
  return found;
}

// intervalMs = time from one beep start to the next, lengthMs = how long each beep lasts.
inline void set(uint32_t intervalMs, uint32_t lengthMs) {
  if (intervalMs == periodMs && lengthMs == onMs) return;
  taskENTER_CRITICAL();
  onMs = lengthMs;
  periodMs = intervalMs;
  gen = gen + 1;
  taskEXIT_CRITICAL();
}

inline void continuous() { set(1000, 1000); }
inline void mute(bool m) { muted = m; }
inline void volume(uint8_t level) { volumeLevel = level > 3 ? 3 : level; }

// Runs loop() as a task and starts the scheduler. Never returns if it succeeds.
inline void runLoopAsTask() {
  if (xTaskCreate(loopThread, "loop", 1100, nullptr, 1, &loopTask) != pdPASS) {
    Serial.println(F("WARNING: not enough memory for the loop task; buzzer will stay silent."));
    return;
  }
  size_t left = xPortGetFreeHeapSize();  // the scheduler still needs about 1.5 KB
  Serial.print(F("RTOS heap left: "));
  Serial.println(left);
  if (left < 1600) Serial.println(F("WARNING: too little heap left; the scheduler may not start."));
  Serial.flush();
  vTaskStartScheduler();
}

}  // namespace Beeper
