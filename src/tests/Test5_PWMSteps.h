#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"

class Test5_PWMSteps : public Test
{
  static const int CH = 0;
  uint8_t j_ = 0;
  uint32_t next_ = 0;
  const uint8_t duties_[3] = {25, 50, 75};
  void apply_(uint8_t d)
  {
    ledcSetup(CH, 1000, 10);
    ledcAttachPin(PWM_PIN, CH);
    uint32_t m = (1u << 10) - 1u;
    ledcWrite(CH, (m * d) / 100u);
  }

public:
  const char *name() const override { return "PWM 1 kHz: duty steps 25/50/75%"; }
  void pinHints(Stream &s) const override { s.printf("PWM -> GPIO%d (1 kHz)\n", PWM_PIN); }
  void begin() override
  {
    j_ = 0;
    apply_(duties_[j_]);
    next_ = millis() + 6000;
  }
  void tick() override
  {
    uint32_t now = millis();
    if ((int32_t)(now - next_) >= 0)
    {
      j_ = (j_ + 1) % 3;
      apply_(duties_[j_]);
      UiBridge_logf("[T5] PWM duty = %u%% @ 1 kHz", duties_[j_]);
      next_ = now + 6000;
    }
  }
  void end() override { ledcSafeDetach(PWM_PIN); }
};