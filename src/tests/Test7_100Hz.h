#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"

class Test7_100Hz : public Test
{
    static const int CH = 0;
    bool on_ = false;

public:
    const char *name() const override { return "Zoom depth: 100 Hz ~50%"; }
    void pinHints(Stream &s) const override { s.printf("PWM -> GPIO%d (100 Hz ~50%%)\n", PWM_PIN); }
    void begin() override
    {
        ledcAttachPin(PWM_PIN, CH);
        ledcSetup(CH, 100, 12);
        ledcWrite(CH, 2048);
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