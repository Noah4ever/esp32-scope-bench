#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"

class Test1_LEDCSweep : public Test
{
    static const int CH = 0;
    static constexpr uint32_t MS_PER_STEP = 10000;
    struct Step
    {
        uint32_t freq;
        uint8_t bits;
        uint32_t ms;
    };
    const Step steps_[4] = {{10000, 8, MS_PER_STEP}, {100000, 6, MS_PER_STEP}, {500000, 3, MS_PER_STEP}, {1000000, 2, MS_PER_STEP}};
    size_t i_ = 0;
    uint32_t next_ = 0;
    void apply_(const Step &s)
    {
        ledcSetup(CH, s.freq, s.bits);
        ledcAttachPin(LEDC_TEST_PIN, CH);
        uint32_t m = (1u << s.bits) - 1u;
        ledcWrite(CH, (m * 50u) / 100u);
    }

public:
    const char *name() const override { return "LEDC sweep 10k→1M @ ~50% (GPIO LEDC_TEST_PIN)"; }
    void pinHints(Stream &s) const override { s.printf("CH1 -> GPIO%d (LEDC sweep)\n", LEDC_TEST_PIN); }
    void begin() override
    {
        i_ = 0;
        apply_(steps_[i_]);
        next_ = millis() + steps_[i_].ms;
        UiBridge_logf("[Test1] LEDC: %lu kHz @ %u-bit (~50%%)", (unsigned long)steps_[i_].freq / 1000, steps_[i_].bits);
    }
    void tick() override
    {
        uint32_t now = millis();
        if ((int32_t)(now - next_) >= 0)
        {
            i_ = (i_ + 1) % 4;
            apply_(steps_[i_]);
            next_ = now + steps_[i_].ms;
            UiBridge_logf("[Test1] LEDC: %lu kHz @ %u-bit (~50%%)", (unsigned long)steps_[i_].freq / 1000, steps_[i_].bits);
        }
    }
    void end() override { ledcSafeDetach(LEDC_TEST_PIN); }
};