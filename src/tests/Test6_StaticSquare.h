#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"

class Test6_StaticSquare : public Test
{
  static const int CH = 0;
  bool on_ = false;

public:
  const char *name() const override { return "Static square ~50% @ PWM_FREQ"; }
  void pinHints(Stream &s) const override { s.printf("Square -> GPIO%d (~50%% @ %lu Hz)\n", PWM_PIN, (unsigned long)PWM_FREQ); }
  void begin() override
  {
    ledcAttachPin(PWM_PIN, CH);
    ledcSetup(CH, PWM_FREQ, PWM_BITS);
    uint32_t m = (1u << PWM_BITS) - 1u;
    ledcWrite(CH, m / 2);
    on_ = true;
  }
  void tick() override { (void)on_; }
  void end() override
  {
    if (!on_)
      return;
    ledcSafeDetach(PWM_PIN);
    on_ = false;
  }
};