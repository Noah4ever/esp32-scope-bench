#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "BenchPins.h"
#include "TestBase.h"

static DMA_ATTR uint8_t T2_BUF[2048];

class Test2_SPIZoom : public Test
{
    SPIClass vspi{VSPI};
    bool on_ = false;

public:
    const char *name() const override { return "SPI edge zoom: SCK 10 MHz (MOSI 0x55, CS LOW)"; }
    void pinHints(Stream &s) const override
    {
        s.printf("SCK -> GPIO%d (10 MHz)\n", SPI_SCK_PIN);
        s.printf("MOSI -> GPIO%d (~5 MHz from 0x55)\n", SPI_MOSI_PIN);
        s.printf("CS   -> GPIO%d (held LOW)\n", SPI_CS_PIN);
    }
    void begin() override
    {
        vspi.begin(SPI_SCK_PIN, 19 /*MISO*/, SPI_MOSI_PIN, SPI_CS_PIN);
        vspi.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
        pinMode(SPI_CS_PIN, OUTPUT);
        digitalWrite(SPI_CS_PIN, LOW);
        memset(T2_BUF, 0x55, sizeof(T2_BUF));
        on_ = true;
    }
    void tick() override
    {
        if (!on_)
            return;
        static uint32_t bursts = 0;
        vspi.transferBytes(T2_BUF, nullptr, sizeof(T2_BUF));
        if ((++bursts % 400) == 0)
            vTaskDelay(1);
    }
    void end() override
    {
        if (!on_)
            return;
        on_ = false;
        digitalWrite(SPI_CS_PIN, HIGH);
        vspi.endTransaction();
        vspi.end();
    }
};
