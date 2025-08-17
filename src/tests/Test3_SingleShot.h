#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"
#include "UiBridge.h"

namespace T3_internal
{
  static volatile bool fire = false;
  static uint32_t last_ms = 0;

  static void IRAM_ATTR onBtn() { fire = true; }
}

class Test3_SingleShot : public Test
{
public:
  const char *name() const override
  {
    return "Single-shot pulse: press BOOT -> 1 ms pulse on PIN_PULSE";
  }
  void pinHints(Stream &s) const override
  {
    s.printf("Pulse -> GPIO%d ; BOOT -> GPIO%d\n", PIN_PULSE, PIN_BTN);
  }

  void begin() override
  {
    T3_internal::fire = false;
    T3_internal::last_ms = 0;

    pinMode(PIN_PULSE, OUTPUT);
    digitalWrite(PIN_PULSE, LOW);
    pinMode(PIN_BTN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_BTN), T3_internal::onBtn, FALLING);

    UiBridge_logf("[Test3] Interrupt on GPIO%d, pulse on GPIO%d", PIN_BTN, PIN_PULSE);
  }

  void tick() override
  {
    if (!T3_internal::fire)
      return;

    noInterrupts();
    bool fired = T3_internal::fire;
    T3_internal::fire = false;
    interrupts();
    if (!fired)
      return;

    uint32_t now = millis();
    if (now - T3_internal::last_ms > 150)
    {
      digitalWrite(PIN_PULSE, HIGH);
      delayMicroseconds(1000);
      digitalWrite(PIN_PULSE, LOW);
      T3_internal::last_ms = now;
      UiBridge_logf("[Test3] Pulse fired (~1 ms) at %lu ms", (unsigned long)now);
    }
  }

  void end() override
  {
    detachInterrupt(digitalPinToInterrupt(PIN_BTN));
    digitalWrite(PIN_PULSE, LOW);
  }
};
